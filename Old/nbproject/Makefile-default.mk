#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=led.c main.c digPot.c uart.c ledInterface.c timer.c communication.c nrf24l01.c format.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/led.o ${OBJECTDIR}/main.o ${OBJECTDIR}/digPot.o ${OBJECTDIR}/uart.o ${OBJECTDIR}/ledInterface.o ${OBJECTDIR}/timer.o ${OBJECTDIR}/communication.o ${OBJECTDIR}/nrf24l01.o ${OBJECTDIR}/format.o
POSSIBLE_DEPFILES=${OBJECTDIR}/led.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/digPot.o.d ${OBJECTDIR}/uart.o.d ${OBJECTDIR}/ledInterface.o.d ${OBJECTDIR}/timer.o.d ${OBJECTDIR}/communication.o.d ${OBJECTDIR}/nrf24l01.o.d ${OBJECTDIR}/format.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/led.o ${OBJECTDIR}/main.o ${OBJECTDIR}/digPot.o ${OBJECTDIR}/uart.o ${OBJECTDIR}/ledInterface.o ${OBJECTDIR}/timer.o ${OBJECTDIR}/communication.o ${OBJECTDIR}/nrf24l01.o ${OBJECTDIR}/format.o

# Source Files
SOURCEFILES=led.c main.c digPot.c uart.c ledInterface.c timer.c communication.c nrf24l01.c format.c

# Pack Options 
PACK_COMPILER_OPTIONS=-I "${DFP_DIR}/include"
PACK_COMMON_OPTIONS=-B "${DFP_DIR}/gcc/dev/atmega328p"



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=ATmega328P
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/led.o: led.c  .generated_files/flags/default/3f5752edb51c5dce8247811ba255d5a45cffcd2f .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/led.o.d 
	@${RM} ${OBJECTDIR}/led.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/led.o.d" -MT "${OBJECTDIR}/led.o.d" -MT ${OBJECTDIR}/led.o  -o ${OBJECTDIR}/led.o led.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/3d967d111a3610e077b336278be6ccd61ab3fcb8 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/main.o.d" -MT "${OBJECTDIR}/main.o.d" -MT ${OBJECTDIR}/main.o  -o ${OBJECTDIR}/main.o main.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/digPot.o: digPot.c  .generated_files/flags/default/632abae8830d45f92df6a9a30c5bafc7f270e45c .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/digPot.o.d 
	@${RM} ${OBJECTDIR}/digPot.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/digPot.o.d" -MT "${OBJECTDIR}/digPot.o.d" -MT ${OBJECTDIR}/digPot.o  -o ${OBJECTDIR}/digPot.o digPot.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/uart.o: uart.c  .generated_files/flags/default/3435df2ded086fd82238db57eea3f029e163b4d5 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart.o.d 
	@${RM} ${OBJECTDIR}/uart.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/uart.o.d" -MT "${OBJECTDIR}/uart.o.d" -MT ${OBJECTDIR}/uart.o  -o ${OBJECTDIR}/uart.o uart.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/ledInterface.o: ledInterface.c  .generated_files/flags/default/782099287ff10cbf67e7dc0206a26535926c2f4b .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ledInterface.o.d 
	@${RM} ${OBJECTDIR}/ledInterface.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/ledInterface.o.d" -MT "${OBJECTDIR}/ledInterface.o.d" -MT ${OBJECTDIR}/ledInterface.o  -o ${OBJECTDIR}/ledInterface.o ledInterface.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/timer.o: timer.c  .generated_files/flags/default/1678596d8558349fb6af17ce4db96ff5986c6f24 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/timer.o.d 
	@${RM} ${OBJECTDIR}/timer.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/timer.o.d" -MT "${OBJECTDIR}/timer.o.d" -MT ${OBJECTDIR}/timer.o  -o ${OBJECTDIR}/timer.o timer.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/communication.o: communication.c  .generated_files/flags/default/6d694e59e9b4231206e1bfc4287d228859a7c02a .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/communication.o.d 
	@${RM} ${OBJECTDIR}/communication.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/communication.o.d" -MT "${OBJECTDIR}/communication.o.d" -MT ${OBJECTDIR}/communication.o  -o ${OBJECTDIR}/communication.o communication.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/nrf24l01.o: nrf24l01.c  .generated_files/flags/default/4e019b8175d9249c115d3a487c68076d0f2f80ab .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/nrf24l01.o.d 
	@${RM} ${OBJECTDIR}/nrf24l01.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/nrf24l01.o.d" -MT "${OBJECTDIR}/nrf24l01.o.d" -MT ${OBJECTDIR}/nrf24l01.o  -o ${OBJECTDIR}/nrf24l01.o nrf24l01.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/format.o: format.c  .generated_files/flags/default/77c56a4dd8419c52a9b9ad0e5071cc0a7dffb67e .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/format.o.d 
	@${RM} ${OBJECTDIR}/format.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS} -g -DDEBUG  -gdwarf-2  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/format.o.d" -MT "${OBJECTDIR}/format.o.d" -MT ${OBJECTDIR}/format.o  -o ${OBJECTDIR}/format.o format.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/led.o: led.c  .generated_files/flags/default/1e800920ed4660990aa20206eba63715253c6c0c .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/led.o.d 
	@${RM} ${OBJECTDIR}/led.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/led.o.d" -MT "${OBJECTDIR}/led.o.d" -MT ${OBJECTDIR}/led.o  -o ${OBJECTDIR}/led.o led.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/5e9958591b6e4c947ae29c8975a9075bc0a0ac3d .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/main.o.d" -MT "${OBJECTDIR}/main.o.d" -MT ${OBJECTDIR}/main.o  -o ${OBJECTDIR}/main.o main.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/digPot.o: digPot.c  .generated_files/flags/default/e0eac6a5cb585f8ee55b430d4916e1ea94bd7bb5 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/digPot.o.d 
	@${RM} ${OBJECTDIR}/digPot.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/digPot.o.d" -MT "${OBJECTDIR}/digPot.o.d" -MT ${OBJECTDIR}/digPot.o  -o ${OBJECTDIR}/digPot.o digPot.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/uart.o: uart.c  .generated_files/flags/default/11f4f5118edd92f692c582479bb347f39cd51bf4 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart.o.d 
	@${RM} ${OBJECTDIR}/uart.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/uart.o.d" -MT "${OBJECTDIR}/uart.o.d" -MT ${OBJECTDIR}/uart.o  -o ${OBJECTDIR}/uart.o uart.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/ledInterface.o: ledInterface.c  .generated_files/flags/default/d66460fd6a46c7b5620ea7c72fc64b879fa6fdfa .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ledInterface.o.d 
	@${RM} ${OBJECTDIR}/ledInterface.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/ledInterface.o.d" -MT "${OBJECTDIR}/ledInterface.o.d" -MT ${OBJECTDIR}/ledInterface.o  -o ${OBJECTDIR}/ledInterface.o ledInterface.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/timer.o: timer.c  .generated_files/flags/default/8c7b888feb59b69b40d4cdcd7e999d8affc71d98 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/timer.o.d 
	@${RM} ${OBJECTDIR}/timer.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/timer.o.d" -MT "${OBJECTDIR}/timer.o.d" -MT ${OBJECTDIR}/timer.o  -o ${OBJECTDIR}/timer.o timer.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/communication.o: communication.c  .generated_files/flags/default/5916fd97bb2f0c8df14d63405c57db5ba4081c5e .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/communication.o.d 
	@${RM} ${OBJECTDIR}/communication.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/communication.o.d" -MT "${OBJECTDIR}/communication.o.d" -MT ${OBJECTDIR}/communication.o  -o ${OBJECTDIR}/communication.o communication.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/nrf24l01.o: nrf24l01.c  .generated_files/flags/default/2d13625bb6279e43f7a234e197d5f61821471d44 .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/nrf24l01.o.d 
	@${RM} ${OBJECTDIR}/nrf24l01.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/nrf24l01.o.d" -MT "${OBJECTDIR}/nrf24l01.o.d" -MT ${OBJECTDIR}/nrf24l01.o  -o ${OBJECTDIR}/nrf24l01.o nrf24l01.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/format.o: format.c  .generated_files/flags/default/558fcf8f087179d47559181edfe0c8bd5141f96d .generated_files/flags/default/8eeb60b897092df86979565fb11a5ba3c2a185bd
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/format.o.d 
	@${RM} ${OBJECTDIR}/format.o 
	 ${MP_CC}  $(MP_EXTRA_CC_PRE) -mmcu=atmega328p ${PACK_COMPILER_OPTIONS} ${PACK_COMMON_OPTIONS}  -x c -c -D__$(MP_PROCESSOR_OPTION)__  -funsigned-char -funsigned-bitfields -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -MD -MP -MF "${OBJECTDIR}/format.o.d" -MT "${OBJECTDIR}/format.o.d" -MT ${OBJECTDIR}/format.o  -o ${OBJECTDIR}/format.o format.c  -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mmcu=atmega328p ${PACK_COMMON_OPTIONS}   -gdwarf-2 -D__$(MP_PROCESSOR_OPTION)__  -Wl,-Map="dist\${CND_CONF}\${IMAGE_TYPE}\vlp-firmware.${IMAGE_TYPE}.map"    -o dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1 -Wl,--gc-sections -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	
	
	
	
	
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -mmcu=atmega328p ${PACK_COMMON_OPTIONS}  -D__$(MP_PROCESSOR_OPTION)__  -Wl,-Map="dist\${CND_CONF}\${IMAGE_TYPE}\vlp-firmware.${IMAGE_TYPE}.map"    -o dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -DXPRJ_default=$(CND_CONF)  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION) -Wl,--gc-sections -Wl,--start-group  -Wl,-lm -Wl,--end-group 
	${MP_CC_DIR}\\avr-objcopy -O ihex "dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}" "dist/${CND_CONF}/${IMAGE_TYPE}/vlp-firmware.${IMAGE_TYPE}.hex"
	
	
	
	
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
