################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/audioTest.obj: ../src/audioTest.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/flash.obj: ../src/flash.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/main.obj: ../src/main.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/tick.obj: ../src/tick.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/usb.obj: ../src/usb.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/src" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Offloader/Debug" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c5500" --define=c5535 --define=OFFLOADER --diag_warning=225 --ptrdiff_size=32 --asm_source=mnemonic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


