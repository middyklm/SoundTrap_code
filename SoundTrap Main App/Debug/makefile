################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv8/tools/compiler/c5500_4.4.1

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./src/accelerometer.obj" \
"./src/adcLogger.obj" \
"./src/audio.obj" \
"./src/audioOut.obj" \
"./src/audio_if.obj" \
"./src/blkpwr.obj" \
"./src/blkpwr_asm.obj" \
"./src/board.obj" \
"./src/bpwr.obj" \
"./src/bwdet.obj" \
"./src/cdet_v2.obj" \
"./src/cfg.obj" \
"./src/config.obj" \
"./src/crc.obj" \
"./src/crc_asm.obj" \
"./src/data.obj" \
"./src/decfilt.obj" \
"./src/decm.obj" \
"./src/decmc_asm.obj" \
"./src/devdep.obj" \
"./src/dma.obj" \
"./src/dmem.obj" \
"./src/fft.obj" \
"./src/filt.obj" \
"./src/filtmc_asm.obj" \
"./src/flsh.obj" \
"./src/fmem.obj" \
"./src/fs.obj" \
"./src/gpio.obj" \
"./src/gpsLogger.obj" \
"./src/hid.obj" \
"./src/hwafft.obj" \
"./src/i2cID.obj" \
"./src/idle.obj" \
"./src/info.obj" \
"./src/ioExpander.obj" \
"./src/irq.obj" \
"./src/job.obj" \
"./src/logg.obj" \
"./src/main.obj" \
"./src/memCardSelect.obj" \
"./src/misc.obj" \
"./src/misc_asm.obj" \
"./src/mk_library.obj" \
"./src/mspInterruptHandler.obj" \
"./src/mspif.obj" \
"./src/mux.obj" \
"./src/protect.obj" \
"./src/pstr.obj" \
"./src/record_app.obj" \
"./src/sar.obj" \
"./src/schedule.obj" \
"./src/sd.obj" \
"./src/sensor.obj" \
"./src/serialEepromV2.obj" \
"./src/serialInit.obj" \
"./src/sine.obj" \
"./src/stats_asm.obj" \
"./src/swVer.obj" \
"./src/sysControl.obj" \
"./src/temperatureLogger.obj" \
"./src/timr.obj" \
"./src/uart.obj" \
"./src/x3cmpv2.obj" \
"./src/x3cmpv2a.obj" \
"../Sudar.cmd" \
$(GEN_CMDS__FLAG) \
-l"C:/Users/Middy/workspace_v8/cslVC5505/Release/cslVC5505.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include src/subdir_vars.mk
-include subdir_rules.mk
-include src/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
SoundTrapMainApp.out \

EXE_OUTPUTS__QUOTED += \
"SoundTrapMainApp.out" \

BIN_OUTPUTS += \
SoundTrapMainApp.hex \

BIN_OUTPUTS__QUOTED += \
"SoundTrapMainApp.hex" \


# All Target
all:
	@$(MAKE) --no-print-directory pre-build
	@$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: SoundTrapMainApp.out

# Tool invocations
SoundTrapMainApp.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C5500 Linker'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --define=c5535 --diag_warning=225 --ptrdiff_size=32 -z -m"SoundTrapMainApp.map" --stack_size=0x200 --heap_size=0x400 -i"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/lib" -i"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --reread_libs --warn_sections --xml_link_info="SoundTrapMainApp_linkInfo.xml" --rom_model --sys_stacksize=0x200 -o "SoundTrapMainApp.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

SoundTrapMainApp.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: C5500 Hex Utility'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/hex55"  -o "SoundTrapMainApp.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "src\accelerometer.obj" "src\adcLogger.obj" "src\audio.obj" "src\audioOut.obj" "src\audio_if.obj" "src\blkpwr.obj" "src\blkpwr_asm.obj" "src\board.obj" "src\bpwr.obj" "src\bwdet.obj" "src\cdet_v2.obj" "src\cfg.obj" "src\config.obj" "src\crc.obj" "src\crc_asm.obj" "src\data.obj" "src\decfilt.obj" "src\decm.obj" "src\decmc_asm.obj" "src\devdep.obj" "src\dma.obj" "src\dmem.obj" "src\fft.obj" "src\filt.obj" "src\filtmc_asm.obj" "src\flsh.obj" "src\fmem.obj" "src\fs.obj" "src\gpio.obj" "src\gpsLogger.obj" "src\hid.obj" "src\hwafft.obj" "src\i2cID.obj" "src\idle.obj" "src\info.obj" "src\ioExpander.obj" "src\irq.obj" "src\job.obj" "src\logg.obj" "src\main.obj" "src\memCardSelect.obj" "src\misc.obj" "src\misc_asm.obj" "src\mk_library.obj" "src\mspInterruptHandler.obj" "src\mspif.obj" "src\mux.obj" "src\protect.obj" "src\pstr.obj" "src\record_app.obj" "src\sar.obj" "src\schedule.obj" "src\sd.obj" "src\sensor.obj" "src\serialEepromV2.obj" "src\serialInit.obj" "src\sine.obj" "src\stats_asm.obj" "src\swVer.obj" 
	-$(RM) "src\sysControl.obj" "src\temperatureLogger.obj" "src\timr.obj" "src\uart.obj" "src\x3cmpv2.obj" "src\x3cmpv2a.obj" 
	-$(RM) "src\blkpwr_asm.d" "src\crc_asm.d" "src\decmc_asm.d" "src\filtmc_asm.d" "src\hwafft.d" "src\misc_asm.d" "src\stats_asm.d" 
	-$(RM) "src\accelerometer.d" "src\adcLogger.d" "src\audio.d" "src\audioOut.d" "src\audio_if.d" "src\blkpwr.d" "src\board.d" "src\bpwr.d" "src\bwdet.d" "src\cdet_v2.d" "src\cfg.d" "src\config.d" "src\crc.d" "src\data.d" "src\decfilt.d" "src\decm.d" "src\devdep.d" "src\dma.d" "src\dmem.d" "src\fft.d" "src\filt.d" "src\flsh.d" "src\fmem.d" "src\fs.d" "src\gpio.d" "src\gpsLogger.d" "src\hid.d" "src\i2cID.d" "src\idle.d" "src\info.d" "src\ioExpander.d" "src\irq.d" "src\job.d" "src\logg.d" "src\main.d" "src\memCardSelect.d" "src\misc.d" "src\mk_library.d" "src\mspInterruptHandler.d" "src\mspif.d" "src\mux.d" "src\protect.d" "src\pstr.d" "src\record_app.d" "src\sar.d" "src\schedule.d" "src\sd.d" "src\sensor.d" "src\serialEepromV2.d" "src\serialInit.d" "src\swVer.d" "src\sysControl.d" "src\temperatureLogger.d" "src\timr.d" "src\uart.d" "src\x3cmpv2.d" 
	-$(RM) "src\sine.d" "src\x3cmpv2a.d" 
	-@echo 'Finished clean'
	-@echo ' '

pre-build:
	-@echo 'Update Build Date and Time'
	-del .\src\swVer.obj
	-@echo ' '

post-build:
	-@echo 'Create flash image: Intel-HEX'
	-"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/hex55.exe" -i "SoundTrapMainApp.out" -o "SoundTrapMainApp.bin" -boot -v5505 -b -serial8
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

