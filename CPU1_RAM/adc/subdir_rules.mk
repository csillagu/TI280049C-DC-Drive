################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
adc/%.obj: ../adc/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/Programs/ti/ccs/tools/compiler/ti-cgt-c2000_21.6.0.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu0 -Ooff --fp_mode=strict --include_path="D:/Documents/BME/temalabor/mcu/epwm_ex8_deadband" --include_path="D:/Documents/BME/temalabor/mcu/epwm_ex8_deadband/device" --include_path="C:/ti/c2000/C2000Ware_4_00_00_00/driverlib/f28004x/driverlib" --include_path="D:/Programs/ti/ccs/tools/compiler/ti-cgt-c2000_21.6.0.LTS/include" --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="adc/$(basename $(<F)).d_raw" --include_path="D:/Documents/BME/temalabor/mcu/epwm_ex8_deadband/CPU1_RAM/syscfg" --obj_directory="adc" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


