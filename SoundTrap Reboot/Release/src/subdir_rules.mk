################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --symdebug:dwarf --diag_warning=225 --memory_model=large --ptrdiff_size=16 --silicon_version=5515 --opt_level=2 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/gpio.obj: C:/Users/kho024/CCSv8/SoundTrapv0/SoundTrap\ Main\ App/src/gpio.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --symdebug:dwarf --diag_warning=225 --memory_model=large --ptrdiff_size=16 --silicon_version=5515 --opt_level=2 --preproc_with_compile --preproc_dependency="src/gpio.d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/protect.obj: C:/Users/kho024/CCSv8/SoundTrapv0/SoundTrap\ Main\ App/src/protect.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --symdebug:dwarf --diag_warning=225 --memory_model=large --ptrdiff_size=16 --silicon_version=5515 --opt_level=2 --preproc_with_compile --preproc_dependency="src/protect.d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/sysControl.obj: C:/Users/kho024/CCSv8/SoundTrapv0/SoundTrap\ Main\ App/src/sysControl.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --symdebug:dwarf --diag_warning=225 --memory_model=large --ptrdiff_size=16 --silicon_version=5515 --opt_level=2 --preproc_with_compile --preproc_dependency="src/sysControl.d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


