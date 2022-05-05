################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../28004x_generic_flash_lnk.cmd 

SYSCFG_SRCS += \
../epwm_ex8_deadband.syscfg 

LIB_SRCS += \
C:/ti/c2000/C2000Ware_4_00_00_00/driverlib/f28004x/driverlib/ccs/Debug/driverlib.lib 

C_SRCS += \
../epwm_ex8_deadband.c \
./syscfg/board.c 

GEN_FILES += \
./syscfg/board.c 

GEN_MISC_DIRS += \
./syscfg/ 

C_DEPS += \
./epwm_ex8_deadband.d \
./syscfg/board.d 

OBJS += \
./epwm_ex8_deadband.obj \
./syscfg/board.obj 

GEN_MISC_FILES += \
./syscfg/board.h \
./syscfg/pinmux.csv 

GEN_MISC_DIRS__QUOTED += \
"syscfg\" 

OBJS__QUOTED += \
"epwm_ex8_deadband.obj" \
"syscfg\board.obj" 

GEN_MISC_FILES__QUOTED += \
"syscfg\board.h" \
"syscfg\pinmux.csv" 

C_DEPS__QUOTED += \
"epwm_ex8_deadband.d" \
"syscfg\board.d" 

GEN_FILES__QUOTED += \
"syscfg\board.c" 

C_SRCS__QUOTED += \
"../epwm_ex8_deadband.c" \
"./syscfg/board.c" 

SYSCFG_SRCS__QUOTED += \
"../epwm_ex8_deadband.syscfg" 


