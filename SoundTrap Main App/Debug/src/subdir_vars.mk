################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
S55_SRCS += \
../src/blkpwr_asm.s55 \
../src/crc_asm.s55 \
../src/decmc_asm.s55 \
../src/filtmc_asm.s55 \
../src/hwafft.s55 \
../src/misc_asm.s55 \
../src/stats_asm.s55 

ASM_SRCS += \
../src/sine.asm \
../src/x3cmpv2a.asm 

C_SRCS += \
../src/accelerometer.c \
../src/adcLogger.c \
../src/audio.c \
../src/audioOut.c \
../src/audio_if.c \
../src/blkpwr.c \
../src/board.c \
../src/bpwr.c \
../src/bwdet.c \
../src/cdet_v2.c \
../src/cfg.c \
../src/config.c \
../src/crc.c \
../src/data.c \
../src/decfilt.c \
../src/decm.c \
../src/devdep.c \
../src/dma.c \
../src/dmem.c \
../src/fft.c \
../src/filt.c \
../src/flsh.c \
../src/fmem.c \
../src/fs.c \
../src/gpio.c \
../src/gpsLogger.c \
../src/hid.c \
../src/i2cID.c \
../src/idle.c \
../src/info.c \
../src/ioExpander.c \
../src/irq.c \
../src/job.c \
../src/logg.c \
../src/main.c \
../src/memCardSelect.c \
../src/misc.c \
../src/mk_library.c \
../src/mspInterruptHandler.c \
../src/mspif.c \
../src/mux.c \
../src/protect.c \
../src/pstr.c \
../src/record_app.c \
../src/sar.c \
../src/schedule.c \
../src/sd.c \
../src/sensor.c \
../src/serialEepromV2.c \
../src/serialInit.c \
../src/swVer.c \
../src/sysControl.c \
../src/temperatureLogger.c \
../src/timr.c \
../src/uart.c \
../src/x3cmpv2.c 

S55_DEPS += \
./src/blkpwr_asm.d \
./src/crc_asm.d \
./src/decmc_asm.d \
./src/filtmc_asm.d \
./src/hwafft.d \
./src/misc_asm.d \
./src/stats_asm.d 

C_DEPS += \
./src/accelerometer.d \
./src/adcLogger.d \
./src/audio.d \
./src/audioOut.d \
./src/audio_if.d \
./src/blkpwr.d \
./src/board.d \
./src/bpwr.d \
./src/bwdet.d \
./src/cdet_v2.d \
./src/cfg.d \
./src/config.d \
./src/crc.d \
./src/data.d \
./src/decfilt.d \
./src/decm.d \
./src/devdep.d \
./src/dma.d \
./src/dmem.d \
./src/fft.d \
./src/filt.d \
./src/flsh.d \
./src/fmem.d \
./src/fs.d \
./src/gpio.d \
./src/gpsLogger.d \
./src/hid.d \
./src/i2cID.d \
./src/idle.d \
./src/info.d \
./src/ioExpander.d \
./src/irq.d \
./src/job.d \
./src/logg.d \
./src/main.d \
./src/memCardSelect.d \
./src/misc.d \
./src/mk_library.d \
./src/mspInterruptHandler.d \
./src/mspif.d \
./src/mux.d \
./src/protect.d \
./src/pstr.d \
./src/record_app.d \
./src/sar.d \
./src/schedule.d \
./src/sd.d \
./src/sensor.d \
./src/serialEepromV2.d \
./src/serialInit.d \
./src/swVer.d \
./src/sysControl.d \
./src/temperatureLogger.d \
./src/timr.d \
./src/uart.d \
./src/x3cmpv2.d 

OBJS += \
./src/accelerometer.obj \
./src/adcLogger.obj \
./src/audio.obj \
./src/audioOut.obj \
./src/audio_if.obj \
./src/blkpwr.obj \
./src/blkpwr_asm.obj \
./src/board.obj \
./src/bpwr.obj \
./src/bwdet.obj \
./src/cdet_v2.obj \
./src/cfg.obj \
./src/config.obj \
./src/crc.obj \
./src/crc_asm.obj \
./src/data.obj \
./src/decfilt.obj \
./src/decm.obj \
./src/decmc_asm.obj \
./src/devdep.obj \
./src/dma.obj \
./src/dmem.obj \
./src/fft.obj \
./src/filt.obj \
./src/filtmc_asm.obj \
./src/flsh.obj \
./src/fmem.obj \
./src/fs.obj \
./src/gpio.obj \
./src/gpsLogger.obj \
./src/hid.obj \
./src/hwafft.obj \
./src/i2cID.obj \
./src/idle.obj \
./src/info.obj \
./src/ioExpander.obj \
./src/irq.obj \
./src/job.obj \
./src/logg.obj \
./src/main.obj \
./src/memCardSelect.obj \
./src/misc.obj \
./src/misc_asm.obj \
./src/mk_library.obj \
./src/mspInterruptHandler.obj \
./src/mspif.obj \
./src/mux.obj \
./src/protect.obj \
./src/pstr.obj \
./src/record_app.obj \
./src/sar.obj \
./src/schedule.obj \
./src/sd.obj \
./src/sensor.obj \
./src/serialEepromV2.obj \
./src/serialInit.obj \
./src/sine.obj \
./src/stats_asm.obj \
./src/swVer.obj \
./src/sysControl.obj \
./src/temperatureLogger.obj \
./src/timr.obj \
./src/uart.obj \
./src/x3cmpv2.obj \
./src/x3cmpv2a.obj 

ASM_DEPS += \
./src/sine.d \
./src/x3cmpv2a.d 

OBJS__QUOTED += \
"src\accelerometer.obj" \
"src\adcLogger.obj" \
"src\audio.obj" \
"src\audioOut.obj" \
"src\audio_if.obj" \
"src\blkpwr.obj" \
"src\blkpwr_asm.obj" \
"src\board.obj" \
"src\bpwr.obj" \
"src\bwdet.obj" \
"src\cdet_v2.obj" \
"src\cfg.obj" \
"src\config.obj" \
"src\crc.obj" \
"src\crc_asm.obj" \
"src\data.obj" \
"src\decfilt.obj" \
"src\decm.obj" \
"src\decmc_asm.obj" \
"src\devdep.obj" \
"src\dma.obj" \
"src\dmem.obj" \
"src\fft.obj" \
"src\filt.obj" \
"src\filtmc_asm.obj" \
"src\flsh.obj" \
"src\fmem.obj" \
"src\fs.obj" \
"src\gpio.obj" \
"src\gpsLogger.obj" \
"src\hid.obj" \
"src\hwafft.obj" \
"src\i2cID.obj" \
"src\idle.obj" \
"src\info.obj" \
"src\ioExpander.obj" \
"src\irq.obj" \
"src\job.obj" \
"src\logg.obj" \
"src\main.obj" \
"src\memCardSelect.obj" \
"src\misc.obj" \
"src\misc_asm.obj" \
"src\mk_library.obj" \
"src\mspInterruptHandler.obj" \
"src\mspif.obj" \
"src\mux.obj" \
"src\protect.obj" \
"src\pstr.obj" \
"src\record_app.obj" \
"src\sar.obj" \
"src\schedule.obj" \
"src\sd.obj" \
"src\sensor.obj" \
"src\serialEepromV2.obj" \
"src\serialInit.obj" \
"src\sine.obj" \
"src\stats_asm.obj" \
"src\swVer.obj" \
"src\sysControl.obj" \
"src\temperatureLogger.obj" \
"src\timr.obj" \
"src\uart.obj" \
"src\x3cmpv2.obj" \
"src\x3cmpv2a.obj" 

S55_DEPS__QUOTED += \
"src\blkpwr_asm.d" \
"src\crc_asm.d" \
"src\decmc_asm.d" \
"src\filtmc_asm.d" \
"src\hwafft.d" \
"src\misc_asm.d" \
"src\stats_asm.d" 

C_DEPS__QUOTED += \
"src\accelerometer.d" \
"src\adcLogger.d" \
"src\audio.d" \
"src\audioOut.d" \
"src\audio_if.d" \
"src\blkpwr.d" \
"src\board.d" \
"src\bpwr.d" \
"src\bwdet.d" \
"src\cdet_v2.d" \
"src\cfg.d" \
"src\config.d" \
"src\crc.d" \
"src\data.d" \
"src\decfilt.d" \
"src\decm.d" \
"src\devdep.d" \
"src\dma.d" \
"src\dmem.d" \
"src\fft.d" \
"src\filt.d" \
"src\flsh.d" \
"src\fmem.d" \
"src\fs.d" \
"src\gpio.d" \
"src\gpsLogger.d" \
"src\hid.d" \
"src\i2cID.d" \
"src\idle.d" \
"src\info.d" \
"src\ioExpander.d" \
"src\irq.d" \
"src\job.d" \
"src\logg.d" \
"src\main.d" \
"src\memCardSelect.d" \
"src\misc.d" \
"src\mk_library.d" \
"src\mspInterruptHandler.d" \
"src\mspif.d" \
"src\mux.d" \
"src\protect.d" \
"src\pstr.d" \
"src\record_app.d" \
"src\sar.d" \
"src\schedule.d" \
"src\sd.d" \
"src\sensor.d" \
"src\serialEepromV2.d" \
"src\serialInit.d" \
"src\swVer.d" \
"src\sysControl.d" \
"src\temperatureLogger.d" \
"src\timr.d" \
"src\uart.d" \
"src\x3cmpv2.d" 

ASM_DEPS__QUOTED += \
"src\sine.d" \
"src\x3cmpv2a.d" 

C_SRCS__QUOTED += \
"../src/accelerometer.c" \
"../src/adcLogger.c" \
"../src/audio.c" \
"../src/audioOut.c" \
"../src/audio_if.c" \
"../src/blkpwr.c" \
"../src/board.c" \
"../src/bpwr.c" \
"../src/bwdet.c" \
"../src/cdet_v2.c" \
"../src/cfg.c" \
"../src/config.c" \
"../src/crc.c" \
"../src/data.c" \
"../src/decfilt.c" \
"../src/decm.c" \
"../src/devdep.c" \
"../src/dma.c" \
"../src/dmem.c" \
"../src/filt.c" \
"../src/flsh.c" \
"../src/fmem.c" \
"../src/fs.c" \
"../src/gpio.c" \
"../src/gpsLogger.c" \
"../src/hid.c" \
"../src/i2cID.c" \
"../src/idle.c" \
"../src/info.c" \
"../src/ioExpander.c" \
"../src/irq.c" \
"../src/job.c" \
"../src/logg.c" \
"../src/main.c" \
"../src/memCardSelect.c" \
"../src/misc.c" \
"../src/mk_library.c" \
"../src/mspInterruptHandler.c" \
"../src/mspif.c" \
"../src/mux.c" \
"../src/protect.c" \
"../src/pstr.c" \
"../src/record_app.c" \
"../src/sar.c" \
"../src/schedule.c" \
"../src/sd.c" \
"../src/sensor.c" \
"../src/serialEepromV2.c" \
"../src/serialInit.c" \
"../src/swVer.c" \
"../src/sysControl.c" \
"../src/temperatureLogger.c" \
"../src/timr.c" \
"../src/uart.c" \
"../src/x3cmpv2.c" 

S55_SRCS__QUOTED += \
"../src/blkpwr_asm.s55" \
"../src/crc_asm.s55" \
"../src/decmc_asm.s55" \
"../src/filtmc_asm.s55" \
"../src/misc_asm.s55" \
"../src/stats_asm.s55" 

ASM_SRCS__QUOTED += \
"../src/sine.asm" \
"../src/x3cmpv2a.asm" 


