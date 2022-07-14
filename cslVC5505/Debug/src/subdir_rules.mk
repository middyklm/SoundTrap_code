################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"" --include_path="/include" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/Debug" --include_path="/packages/ti/xdais" --include_path="C:/Users/kho024/CCSv8/src" --include_path="C:/Users/kho024/CCSv8/inc" --symdebug:dwarf --diag_warning=225 --algebraic --asm_source=algebraic --memory_model=large --ptrdiff_size=16 --silicon_version=5515 --opt_level=3 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../src/%.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"" --include_path="/include" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/Debug" --include_path="/packages/ti/xdais" --include_path="C:/Users/kho024/CCSv8/src" --include_path="C:/Users/kho024/CCSv8/inc" --symdebug:dwarf --diag_warning=225 --algebraic --asm_source=algebraic --memory_model=large --ptrdiff_size=16 --silicon_version=5515 --opt_level=3 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


