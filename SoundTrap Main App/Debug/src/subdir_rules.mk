################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/audio.obj: ../src/audio.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/%.obj: ../src/%.s55 $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/decm.obj: ../src/decm.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/decmc_asm.obj: ../src/decmc_asm.s55 $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/fft.obj: ../src/fft.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/hwafft.obj: ../src/hwafft.s55 $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --asm_source=algebraic --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/%.obj: ../src/%.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C5500 Compiler'
	"C:/ti/ccsv8/tools/compiler/c5500_4.4.1/bin/cl55" -v5515 --memory_model=large -O3 -g --include_path="C:/Users/Middy/workspace_v8/cslVC5505" --include_path="C:/Users/Middy/workspace_v8/cslVC5505/inc" --include_path="C:/Users/Middy/workspace_v8/SoundTrap Main App - v0.1.0/SoundTrap Main App - v0.1/inc" --include_path="C:/ti/ccsv8/tools/compiler/c5500_4.4.1/include" --define=c5535 --diag_warning=225 --ptrdiff_size=32 --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


