################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Reboot/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --define=BOOTLOADER --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/gpio.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/gpio.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Reboot/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --define=BOOTLOADER --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="src/gpio.d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/protect.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/protect.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Reboot/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --define=BOOTLOADER --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="src/protect.d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/sysControl.obj: C:/Users/Middy/workspace_v8/SoundTrap\ Main\ App\ -\ Middy/src/sysControl.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/src" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - Middy/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Reboot/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --define=BOOTLOADER --diag_warning=225 --ptrdiff_size=16 --preproc_with_compile --preproc_dependency="src/sysControl.d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


