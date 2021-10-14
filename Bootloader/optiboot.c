#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "boot.h"
#include "pin_defs.h"
#include "stk500.h"

#define OPTIBOOT_MAJVER 5
#define OPTIBOOT_MINVER 0

#define MAKESTR(a) #a
#define MAKEVER(a, b) MAKESTR(a*256+b)

asm("  .section .version\n"
    "optiboot_version:  .word " MAKEVER(OPTIBOOT_MAJVER, OPTIBOOT_MINVER) "\n"
    "  .section .text\n");


/* set the UART baud rate defaults */
#ifndef BAUD_RATE
#if F_CPU >= 8000000L
#define BAUD_RATE   115200L // Highest rate Avrdude win32 will support
#elsif F_CPU >= 1000000L
#define BAUD_RATE   9600L   // 19200 also supported, but with significant error
#elsif F_CPU >= 128000L
#define BAUD_RATE   4800L   // Good for 128kHz internal RC
#else
#define BAUD_RATE 1200L     // Good even at 32768Hz
#endif
#endif

#define BAUD_SETTING (( (F_CPU + BAUD_RATE * 4L) / ((BAUD_RATE * 8L))) - 1 )
#define BAUD_ACTUAL (F_CPU/(8 * ((BAUD_SETTING)+1)))
#define BAUD_ERROR (( 100*(BAUD_RATE - BAUD_ACTUAL) ) / BAUD_RATE)

#if BAUD_ERROR >= 5
#error BAUD_RATE error greater than 5%
#elif BAUD_ERROR <= -5
#error BAUD_RATE error greater than -5%
#elif BAUD_ERROR >= 2
#warning BAUD_RATE error greater than 2%
#elif BAUD_ERROR <= -2
#warning BAUD_RATE error greater than -2%
#endif

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

#define WATCHDOG_SELECTION WATCHDOG_1S /* Select what watchdog setting to use */

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

/* C zero initialises all global variables. However, that requires */
/* These definitions are NOT zero initialised, but that doesn't matter */
/* This allows us to drop the zero init code, saving us memory */
#define buff    ((uint8_t*)(RAMSTART+BSS_SIZE))

#define my_delay(msec) delay8((int) (F_CPU / 8000L * (msec)))

/* Function Prototypes */
/* The main function is in init9, which removes the interrupt vector table */
/* we don't need. It is also 'naked', which means the compiler does not    */
/* generate any entry or exit code itself. */
int main(void) __attribute__ ((OS_main)) __attribute__ ((section (".init9"))) __attribute__ ((__noreturn__));
static void radio_init(void);
void putch(char);
uint8_t getch(void);
static inline void getNch(uint8_t);
void verifySpace();
uint8_t getLen();
static inline void watchdogReset();
void watchdogConfig(uint8_t x);
void wait_timeout(void) __attribute__ ((__noreturn__));
void appStart(uint8_t rstFlags) __attribute__ ((naked))  __attribute__ ((__noreturn__));
static void delay8(uint16_t count);

/***************************************************************************/
/*						 		Main Function							   */
/***************************************************************************/
/* Handles the communcation with AVRDUDE and the memory operations when    */
/* flashing the device. 												   */

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

	/* UART setup */
	UCSR0A = _BV(U2X0); //Double speed mode USART0
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
	UBRR0L = (uint8_t)( (F_CPU + BAUD_RATE * 4L) / (BAUD_RATE * 8L) - 1 );

	/* Setup radio module */
	radio_init();

	// Set up watchdog to trigger after 500ms
	watchdogConfig(WATCHDOG_SELECTION);

	/* Forever loop */
	for (;;) {
		/* get character from UART */
		ch = getch();

		if (ch == STK_GET_PARAMETER) {
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
		else if (ch == STK_SET_DEVICE) {
			// SET DEVICE is ignored
			getNch(20);
		}
		else if (ch == STK_SET_DEVICE_EXT) {
			// SET DEVICE EXT is ignored
			getNch(5);
		}
		else if (ch == STK_LOAD_ADDRESS) {
			// LOAD ADDRESS
			uint16_t newAddress;
			newAddress = getch();
			newAddress |= getch() << 8;
			newAddress <<= 1; // Convert from word address to byte address
			address = newAddress;
			verifySpace();
		}
		else if (ch == STK_UNIVERSAL) {
			// UNIVERSAL command is ignored
			getNch(4);
			putch(0x00);
		}
		/* Write memory, length is big endian and is in bytes */
		else if (ch == STK_PROG_PAGE) {
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

			// If we are in NRWW section, page erase has to be delayed until now.
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
				__boot_page_fill_short((uint16_t)(void*)addrPtr, a);
				addrPtr += 2;
			} while (--ch);

			// Write from programming buffer
			__boot_page_write_short((uint16_t)(void*)address);
			boot_spm_busy_wait();
		}
		/* Read memory block mode, length is big endian.  */
		else if (ch == STK_READ_PAGE) {
			// READ PAGE - we only read flash and EEPROM
			uint8_t type;

			getch();			/* getlen() */
			length = getch();
			type = getch();

			verifySpace();
			/* TODO: putNch */
			do {
				// read a Flash byte and increment the address
				__asm__ ("lpm %0,Z+\n" : "=r" (ch), "=z" (address): "1" (address));
				putch(ch);
			} while (--length);
		}

		/* Get device signature bytes  */
		else if (ch == STK_READ_SIGN) {
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

/***************************************************************************/
/*					    Radio and UART functions						   */
/***************************************************************************/

/*
 * Radio mode gets set the moment we receive any command over the radio chip.
 * From that point our responses will also be sent through the radio instead
 * of through the UART.  Otherwise all communication goes through the UART
 * as normal.
 */
#define RADIO_ON 1
#define RADIO_OFF 0
static uint8_t radio_mode = RADIO_OFF;

#include "spi.h"
#include "spi.c"
/* The Makefile doesn't compile and link all .c files automatically, so I'm
   adding them manually */
#include "nrf24l01.h"
#include "nrf24l01.c"

/* Used to grab addresses from eeprom */
static uint8_t eeprom_read(uint16_t addr) {
	while (EECR & _BV(EEPE));
	EEAR = addr;
	EECR |= 1 << EERE;  /* Start eeprom read by writing EERE */

	return EEDR;
}

static void radio_init(void) {
	uint8_t RX_addr[3] = {'L', 'M', '1'};
	uint8_t TX_addr[3] = {'M', 'T', 'R'};

	spi_init();
	
	nrf24_config(TX_addr, RX_addr);
}

void putch(char ch) {

	static uint8_t tx_pkt_len = 0; /* Number of bytes in the local buffer */
	static uint8_t tx_pkt_buf[32]; /* Local buffer to store bytes before sending */

	if(radio_mode == RADIO_ON){

		tx_pkt_buf[tx_pkt_len++] = ch; /* Fills the local buffer */

		if (ch == STK_OK || tx_pkt_len == NRF24_MAX_PAYLOAD-1) { /* When last message or buffer full */
			while (1) { /* Send buffer until received */
				
				nrf24_sendData(tx_pkt_buf, tx_pkt_len);
				if (nrf24_wait_tx_result()==NRF24_MESSAGE_SENT)
				break; /* Payload sent and acknowledged*/
				
				/* Wait 4ms to allow the remote end to switch to Rx mode */
				my_delay(4);
			}

			/* Reset the local buffer */
			tx_pkt_len = 1; /* Only the identifier is stored */
			tx_pkt_buf[0] ++; /* Packet Identifier */
		}
	}
	else{
		while (!(UCSR0A & _BV(UDRE0)));
		UDR0 = ch;
	}
}

uint8_t getch(void) {
	uint8_t ch='\0';
	static uint8_t pkt_id = 0; /* Number of the packet we are currently receiving */
	static uint8_t rx_pkt_len = 0; /* Number of bytes in the local buffer */
	static uint8_t rx_pkt_ptr = 1; /* Start of data in the buffer */
	static uint8_t rx_pkt_buf[32]; /* Local buffer to store bytes before sending */

	while (1) {
		if (UCSR0A & _BV(RXC0)) {
			if (!(UCSR0A & _BV(FE0))) {
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
			ch = UDR0;
			break;
		}

		/* If there is data in the local buffer or new data in RF24 fifo */
		if (rx_pkt_len || nrf24_dataReady()==NRF24_DATA_AVAILABLE){
			watchdogReset();
			radio_mode=RADIO_ON; /* From now on, we're in radio mode */

			/* If our local buffer is empty, get more data */
			if (rx_pkt_len==0) { 
		
				nrf24_getData(rx_pkt_buf, &rx_pkt_len);
		
				if (rx_pkt_buf[0] == pkt_id) { /* We have already received this packet before */
					rx_pkt_len = 0;
				}
				else
				{
					pkt_id=rx_pkt_buf[0]; /* It's a new packet, update the current ID */
				}
			}

			/* If there is data in the local buffer */
			if (rx_pkt_len) 
			{
				ch=rx_pkt_buf[rx_pkt_ptr]; /* Grab next byte in the buffer */
				rx_pkt_ptr++;
				rx_pkt_len--;
				if(rx_pkt_len==0) /* We have read all the bytes in the buffer */
				{
					rx_pkt_ptr=1; /* Reset the data pointer */
				}
				break;
			}
		}
	}
	return ch;
}


/***************************************************************************/
/*							Other functions								   */
/***************************************************************************/

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

	watchdogConfig(WATCHDOG_SELECTION); /* Re-enable watchdog for user app */

	//  save the reset flags in the designated register
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

static void delay8(uint16_t count) {
	while (count --)
		__asm__ __volatile__ (
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\tnop\n"
			"\twdr\n"
		);
}
