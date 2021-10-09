#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "boot.h"
#include "pin_defs.h"
#include "stk500.h"
#include "../../Common/spi.h"
#include "../../Common/nrf24l01.h"

#define OPTIBOOT_MAJVER 5
#define OPTIBOOT_MINVER 0

#define MAKESTR(a) #a
#define MAKEVER(a, b) MAKESTR(a*256+b)

asm("  .section .version\n"
"optiboot_version:  .word " MAKEVER(OPTIBOOT_MAJVER, OPTIBOOT_MINVER) "\n"
"  .section .text\n");

#ifndef F_CPU
#define F_CPU 16000000
#endif
/* set the UART baud rate defaults */
#define BAUD_RATE 115200

#define BAUD_SETTING (( (F_CPU + BAUD_RATE * 4L) / ((BAUD_RATE * 8L))) - 1 )
#define BAUD_ACTUAL (F_CPU/(8 * ((BAUD_SETTING)+1)))
#define BAUD_ERROR (( 100*(BAUD_RATE - BAUD_ACTUAL) ) / BAUD_RATE)

/* Watchdog settings */
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))
#define WATCHDOG_32MS   (_BV(WDP0) | _BV(WDE))
#define WATCHDOG_64MS   (_BV(WDP1) | _BV(WDE))
#define WATCHDOG_125MS  (_BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_250MS  (_BV(WDP2) | _BV(WDE))
#define WATCHDOG_500MS  (_BV(WDP2) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_2S     (_BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_4S     (_BV(WDP3) | _BV(WDE))
#define WATCHDOG_8S     (_BV(WDP3) | _BV(WDP0) | _BV(WDE))

/* Function Prototypes */
/* The main function is in init9, which removes the interrupt vector table */
/* we don't need. It is also 'naked', which means the compiler does not    */
/* generate any entry or exit code itself. */
int main(void) __attribute__ ((OS_main)) __attribute__ ((section (".init9"))) __attribute__ ((__noreturn__));
void putch(char);
uint8_t getch(void);
static inline void getNch(uint8_t); /* "static inline" is a compiler hint to reduce code size */
void verifySpace();
static inline void watchdogReset();
void watchdogConfig(uint8_t x);
void wait_timeout(void) __attribute__ ((__noreturn__));
void appStart(uint8_t rstFlags) __attribute__ ((naked))  __attribute__ ((__noreturn__));
static void radio_init(void);


/*
* NRWW memory
* Addresses below NRWW (Non-Read-While-Write) can be programmed while
* continuing to run code from flash, slightly speeding up programming
* time.  Beware that Atmel data sheets specify this as a WORD address,
* while optiboot will be comparing against a 16-bit byte address.  This
* means that on a part with 128kB of memory, the upper part of the lower
* 64k will get NRWW processing as well, even though it doesn't need it.
* That's OK.  In fact, you can disable the overlapping processing for
* a part entirely by setting NRWWSTART to zero.  This reduces code
* space a bit, at the expense of being slightly slower, overall.
*
* RAMSTART should be self-explanatory.  It's bigger on parts with a
* lot of peripheral registers.
*/
#define RAMSTART (0x100)
#define NRWWSTART (0x7000)

// TODO: get actual .bss+.data size from GCC
#define BSS_SIZE	0x80

/* C zero initializes all global variables. However, that requires */
/* These definitions are NOT zero initialized, but that doesn't matter */
/* This allows us to drop the zero init code, saving us memory */
#define buff  ((uint8_t*)(RAMSTART+BSS_SIZE))

/*
* Handle devices with up to 4 uarts (eg m1280.)  Rather inelegantly.
* Note that mega8/m32 still needs special handling, because ubrr is handled
* differently.
*/
# define UART_SRA UCSR0A
# define UART_SRB UCSR0B
# define UART_SRC UCSR0C
# define UART_SRL UBRR0L
# define UART_UDR UDR0

/* Function to read EEPROM (Used to read addresses) */
static uint8_t eeprom_read(uint16_t addr) {
	while (!eeprom_is_ready());
	EEAR = addr;
	EECR |= 1 << EERE;	/* Start eeprom read by writing EERE */
	return EEDR;
}

/* main program starts here */
int main(void) {
	uint8_t ch;

	/*
	* Making these local and in registers prevents the need for initializing
	* them, and also saves space because code no longer stores to memory.
	* (initializing address keeps the compiler happy, but isn't really
	*  necessary, and uses 4 bytes of flash.)
	*/
	register uint16_t address = 0;
	register uint8_t  length;

	// After the zero init loop, this is the first code to run.
	//
	// This code makes the following assumptions:
	//  No interrupts will execute
	//  SP points to RAMEND
	//  r1 contains zero
	//
	// If not, uncomment the following instructions:
	// cli();
	asm volatile ("cli");
	asm volatile ("clr __zero_reg__");

	/*
	* With wireless flashing it's possible that this is a remote
	* board that's hard to reset manually.  In this case optiboot can
	* force the watchdog to run before jumping to userspace, so that if
	* a buggy program is uploaded, the board resets automatically.  We
	* still use the watchdog to reset the bootloader too.
	*/
	SP = RAMEND - 32;
	#define reset_cause (*(uint8_t *) (RAMEND - 16 - 4))
	#define marker (*(uint32_t *) (RAMEND - 16 - 3))

	/* GCC does loads Y with SP at the beginning, repeat it with the new SP */
	asm volatile ("in r28, 0x3d");
	asm volatile ("in r29, 0x3e");

	ch = MCUSR;
	MCUSR = 0;
	if ((ch & _BV(WDRF)) && marker == 0xdeadbeef) {
		marker = 0;
		appStart(reset_cause);
	}
	/* Save the original reset reason to pass on to the applicatoin */
	reset_cause = ch;
	marker = 0xdeadbeef;

	// Prepare .data
	asm volatile (
	"	ldi	r17, hi8(__data_end)\n"
	"	ldi	r26, lo8(__data_start)\n"
	"	ldi	r27, hi8(__data_start)\n"
	"	ldi	r30, lo8(__data_load_start)\n"
	"	ldi	r31, hi8(__data_load_start)\n"
	"	rjmp	cpchk\n"
	"copy:	lpm	__tmp_reg__, Z+\n"
	"	st	X+, __tmp_reg__\n"
	"cpchk:	cpi	r26, lo8(__data_end)\n"
	"	cpc	r27, r17\n"
	"	brne	copy\n");
	// Prepare .bss
	asm volatile (
	"	ldi	r17, hi8(__bss_end)\n"
	"	ldi	r26, lo8(__bss_start)\n"
	"	ldi	r27, hi8(__bss_start)\n"
	"	rjmp	clchk\n"
	"clear:	st	X+, __zero_reg__\n"
	"clchk:	cpi	r26, lo8(__bss_end)\n"
	"	cpc	r27, r17\n"
	"	brne	clear\n");

	/*
	* Disable pullups that may have been enabled by a user program.
	* Somehow a pullup on RXD screws up everything unless RXD is externally
	* driven high.
	*/
	DDRD |= 3;
	PORTD &= ~3;
	
	radio_init();

	// Set up watchdog to trigger after 500ms
	watchdogConfig(WATCHDOG_1S);

	/* Forever loop */
	for (;;) {
		/* get character from UART */
		ch = getch();

		if(ch == STK_GET_PARAMETER) {
			unsigned char which = getch();
			verifySpace();
			if (which == 0x82) {
				/*
				* Send optiboot version as "minor SW version"
				*/
				putch(OPTIBOOT_MINVER);
				} else if (which == 0x81) {
				putch(OPTIBOOT_MAJVER);
				} else {
				/*
				* GET PARAMETER returns a generic 0x03 reply for
				* other parameters - enough to keep Avrdude happy
				*/
				putch(0x03);
			}
		}
		else if(ch == STK_SET_DEVICE) {
			// SET DEVICE is ignored
			getNch(20);
		}
		else if(ch == STK_SET_DEVICE_EXT) {
			// SET DEVICE EXT is ignored
			getNch(5);
		}
		else if(ch == STK_LOAD_ADDRESS) {
			// LOAD ADDRESS
			uint16_t newAddress;
			newAddress = getch();
			newAddress |= getch() << 8;
			#ifdef RAMPZ
			// Transfer top bit to RAMPZ
			RAMPZ = (newAddress & 0x8000) ? 1 : 0;
			#endif
			newAddress <<= 1; // Convert from word address to byte address
			address = newAddress;
			verifySpace();
		}
		else if(ch == STK_UNIVERSAL) {
			// UNIVERSAL command is ignored
			getNch(4);
			putch(0x00);
		}
		/* Write memory, length is big endian and is in bytes */
		else if(ch == STK_PROG_PAGE) {
			// PROGRAM PAGE - we support flash and EEPROM programming
			uint8_t *bufPtr;
			uint16_t addrPtr;
			uint8_t type;

			getch();			/* getlen() */
			length = getch();
			type = getch();

			// If we are in RWW section, immediately start page erase
			if (address < NRWWSTART) __boot_page_erase_short((uint16_t)(void*)address);

			// While that is going on, read in page contents
			bufPtr = buff;
			do *bufPtr++ = getch();
			while (--length);

			if (type == 'F') {	/* Flash */
				// If we are in NRWW section, page erase has to be delayed until now.
				// Todo: Take RAMPZ into account (not doing so just means that we will
				//  treat the top of both "pages" of flash as NRWW, for a slight speed
				//  decrease, so fixing this is not urgent.)
				if (address >= NRWWSTART) __boot_page_erase_short((uint16_t)(void*)address);

				// Read command terminator, start reply
				verifySpace();

				// If only a partial page is to be programmed, the erase might not be complete.
				// So check that here
				boot_spm_busy_wait();

				// Copy buffer into programming buffer
				bufPtr = buff;
				addrPtr = (uint16_t)(void*)address;
				ch = SPM_PAGESIZE / 2;
				do {
					uint16_t a;
					a = *bufPtr++;
					a |= (*bufPtr++) << 8;
					__boot_page_fill_short((uint16_t)(void*)addrPtr,a);
					addrPtr += 2;
				} while (--ch);

				// Write from programming buffer
				__boot_page_write_short((uint16_t)(void*)address);
				boot_spm_busy_wait();

				#if defined(RWWSRE)
				// Reenable read access to flash
				boot_rww_enable();
				#endif
		}
		/* Read memory block mode, length is big endian.  */
		else if(ch == STK_READ_PAGE) {
			// READ PAGE - we only read flash and EEPROM
			uint8_t type;

			getch();			/* getlen() */
			length = getch();
			type = getch();

			verifySpace();
			/* TODO: putNch */
			do {
				#if defined(RAMPZ)
				// Since RAMPZ should already be set, we need to use EPLM directly.
				// Also, we can use the autoincrement version of lpm to update "address"
				//      do putch(pgm_read_byte_near(address++));
				//      while (--length);
				// read a Flash and increment the address (may increment RAMPZ)
				__asm__ ("elpm %0,Z+\n" : "=r" (ch), "=z" (address): "1" (address));
				#else
				// read a Flash byte and increment the address
				__asm__ ("lpm %0,Z+\n" : "=r" (ch), "=z" (address): "1" (address));
				#endif
				putch(ch);
			} while (--length);
		}

		/* Get device signature bytes  */
		else if(ch == STK_READ_SIGN) {
			// READ SIGN - return what Avrdude wants to hear
			verifySpace();
			putch(SIGNATURE_0);
			putch(SIGNATURE_1);
			putch(SIGNATURE_2);
		}
		else if (ch == STK_LEAVE_PROGMODE) { /* 'Q' */
			// Adaboot no-wait mod
			watchdogConfig(WATCHDOG_16MS);
			verifySpace();
		}
		else {
			// This covers the response to commands like STK_ENTER_PROGMODE
			verifySpace();
		}
		putch(STK_OK);
	}
}

static uint8_t radio_mode = 0;
static uint8_t radio_present = 0;
static uint8_t pkt_max_len = 32;

#define CE_DDR		DDRB
#define CE_PORT		PORTB
#define CSN_DDR		DDRB
#define CSN_PORT	PORTB
#define CE_PIN		(1 << 0)
#define CSN_PIN		(1 << 2)



#define SEQN

static void radio_init(void) {
	uint8_t addr[3] = {'L', 'M', '1'};
		

	spi_init();

	if (nrf24_init())
	return;

	radio_present = 1;
	/*
	* Set our own address.
	*
	* The remote end's address will be set according to the contents
	* of the first packet we receive from the master.
	*/

	nrf24_set_rx_addr(addr);

	nrf24_rx_mode();
}


void putch(char ch) {
	if (radio_mode) {
		static uint8_t pkt_len = 0;
		static uint8_t pkt_buf[32];

		pkt_buf[pkt_len++] = ch;

		if (ch == STK_OK || pkt_len == pkt_max_len) {
			#ifdef SEQN
			uint8_t cnt = 128;

			while (--cnt) {
				/* Wait 4ms to allow the remote end to switch to Rx mode */
				my_delay(4);

				nrf24_tx(pkt_buf, pkt_len);
				if (!nrf24_tx_result_wait())
				break;

				/*
				* TODO: also check if there's anything in the Rx FIFO - that
				* would indicate that the other side has actually received our
				* packet but the ACK may have been lost instead.  In any case
				* the other side is not listening for what we're re-sending,
				* maybe has given up and is resending the full command which
				* is ok.
				*/
			}

			pkt_len = 1;
			pkt_buf[0] ++;
			#else
			/* Wait 4ms to allow the remote end to switch to Rx mode */
			my_delay(4);

			nrf24_tx(pkt_buf, pkt_len);
			nrf24_tx_result_wait();

			pkt_len = 0;
			#endif
		}

		return;
	}
	while (!(UART_SRA & _BV(UDRE0)));
	UART_UDR = ch;
}

uint8_t getch(void) {
	uint8_t ch;

	static uint8_t pkt_len = 0, pkt_start = 0;
	static uint8_t pkt_buf[32];

	LED_PIN |= _BV(LED);


	while(1) {
		if (UART_SRA & _BV(RXC0)) {
			if (!(UART_SRA & _BV(FE0))) {
				/*
				* A Framing Error indicates (probably) that something is talking
				* to us at the wrong bit rate.  Assume that this is because it
				* expects to be talking to the application, and DON'T reset the
				* watchdog.  This should cause the bootloader to abort and run
				* the application "soon", if it keeps happening.  (Note that we
				* don't care that an invalid char is returned...)
				*/
				watchdogReset();
			}

			ch = UART_UDR;
			break;
		}

		if (radio_present && (pkt_len || nrf24_rx_fifo_data())) {
			watchdogReset();

			if (!pkt_len) {
				#ifdef SEQN
				static uint8_t seqn = 0xff;
				#define START 1
				#else
				#define START 0
				#endif
				nrf24_rx_read(pkt_buf, &pkt_len);
				pkt_start = START;

				if (!radio_mode && pkt_len >= 4) {
					/*
					* If this is the first packet we receive, the first three bytes
					* should contain the sender's address.
					*/
					nrf24_set_tx_addr(pkt_buf);
					pkt_max_len = pkt_buf[3];
					pkt_len -= 4;
					pkt_start += 4;

					radio_mode = 1;
				} else if (!radio_mode)
				pkt_len = 0;

				if (!pkt_len)
				continue;

				#ifdef SEQN
				if (pkt_buf[0] == seqn) {
					pkt_len = 0;
					continue;
				}

				seqn = pkt_buf[0];
				pkt_len--;
				#endif
			}

			ch = pkt_buf[pkt_start ++];
			pkt_len --;
			break;
		}
}

	return ch;
}

void getNch(uint8_t count) {
	do getch(); while (--count);
	verifySpace();
}

void wait_timeout(void) {
	watchdogConfig(WATCHDOG_16MS);      // shorten WD timeout
	while (1)			      // and busy-loop so that WD causes
	;				      //  a reset and app start.
}

void verifySpace(void) {
	if (getch() != CRC_EOP)
	wait_timeout();
	putch(STK_INSYNC);
}

// Watchdog functions. These are only safe with interrupts turned off.
void watchdogReset() {
	__asm__ __volatile__ (
	"wdr\n"
	);
}

void watchdogConfig(uint8_t x) {
	WDTCSR = _BV(WDCE) | _BV(WDE);
	WDTCSR = x;
}

void appStart(uint8_t rstFlags) {
	#ifdef FORCE_WATCHDOG
	watchdogConfig(WATCHDOG_4S);
	#else
	watchdogConfig(WATCHDOG_OFF);
	#endif

	// save the reset flags in the designated register
	//  This can be saved in a main program by putting code in .init0 (which
	//  executes before normal c init code) to save R2 to a global variable.
	__asm__ __volatile__ ("mov r2, %0\n" :: "r" (rstFlags));

	__asm__ __volatile__ (
	// Jump to RST vector
	"clr r30\n"
	"clr r31\n"
	"ijmp\n"
	);
}
