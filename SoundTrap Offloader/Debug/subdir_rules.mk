################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
config.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/config.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="config.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

crc.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/crc.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="crc.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

crc_asm.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/crc_asm.s55 $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="crc_asm.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

dma.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/dma.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="dma.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

gpio.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/gpio.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="gpio.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

hid.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/hid.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="hid.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

ioExpander.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/ioExpander.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="ioExpander.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

irq.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/irq.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="irq.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

memCardSelect.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/memCardSelect.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="memCardSelect.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

mspif.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/mspif.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="mspif.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

sar.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/sar.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="sar.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

sd.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/sd.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="sd.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

serialEepromV2.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/serialEepromV2.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="serialEepromV2.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

sine.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/sine.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="sine.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

uart.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/uart.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="uart.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


