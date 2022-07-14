################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./csl_audioClass.obj" \
"./csl_cdc.obj" \
"./csl_dat.obj" \
"./csl_dma.obj" \
"./csl_gpio.obj" \
"./csl_gpt.obj" \
"./csl_i2c.obj" \
"./csl_i2s.obj" \
"./csl_intc.obj" \
"./csl_irqplug.obj" \
"./csl_lcdc.obj" \
"./csl_mem.obj" \
"./csl_mmcsd.obj" \
"./csl_mmcsd_ataIf.obj" \
"./csl_msc.obj" \
"./csl_nand.obj" \
"./csl_pll.obj" \
"./csl_rtc.obj" \
"./csl_sar.obj" \
"./csl_sdio.obj" \
"./csl_spi.obj" \
"./csl_uart.obj" \
"./csl_usb.obj" \
"./csl_wdt.obj" \
"./vectors.obj" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir_vars.mk
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
LIB_OUTPUTS += \
cslVC5505.lib \

LIB_OUTPUTS__QUOTED += \
"cslVC5505.lib" \


# All Target
all: cslVC5505.lib

# Tool invocations
cslVC5505.lib: $(OBJS) $(OBJ_SRCS) $(LIB_SRCS)
ifneq ($(strip $(ORDERED_OBJS)),)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C5500 Archiver'
	"" r "cslVC5505.lib" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '
endif

# Other Targets
clean:
	-$(RM) $(LIB_OUTPUTS__QUOTED)
	-$(RM) "csl_audioClass.obj" "csl_cdc.obj" "csl_dat.obj" "csl_dma.obj" "csl_gpio.obj" "csl_gpt.obj" "csl_i2c.obj" "csl_i2s.obj" "csl_intc.obj" "csl_irqplug.obj" "csl_lcdc.obj" "csl_mem.obj" "csl_mmcsd.obj" "csl_mmcsd_ataIf.obj" "csl_msc.obj" "csl_nand.obj" "csl_pll.obj" "csl_rtc.obj" "csl_sar.obj" "csl_sdio.obj" "csl_spi.obj" "csl_uart.obj" "csl_usb.obj" "csl_wdt.obj" "vectors.obj" 
	-$(RM) "src\csl_audioClass.d" "src\csl_cdc.d" "src\csl_dat.d" "src\csl_dma.d" "src\csl_gpio.d" "src\csl_gpt.d" "src\csl_i2c.d" "src\csl_i2s.d" "src\csl_intc.d" "src\csl_lcdc.d" "src\csl_mem.d" "src\csl_mmcsd.d" "src\csl_mmcsd_ataIf.d" "src\csl_msc.d" "src\csl_nand.d" "src\csl_pll.d" "src\csl_rtc.d" "src\csl_sar.d" "src\csl_sdio.d" "src\csl_spi.d" "src\csl_uart.d" "src\csl_usb.d" "src\csl_wdt.d" 
	-$(RM) "src\csl_irqplug.d" "src\vectors.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

