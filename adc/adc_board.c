/*
 * adc_board.c
 *
 *  Created on: Feb 28, 2022
 *      Author: ujvar
 */

#include "adc_board.h"

void Board_init()
{
    EALLOW;

    PinMux_init();
    EPWM_init();
    ASYSCTL_init();
    ADC_init();
    INTERRUPT_init();


    EDIS;
}

void PinMux_init()
{
    //
    // EPWM1 -> myEPWM1 Pinmux
    //
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    //
    // EPWM2 -> myEPWM2 Pinmux
    //
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    //
    // EPWM3 -> myEPWM3 Pinmux
    //
    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setPinConfig(GPIO_5_EPWM3_B);
    //
    // EPWM4 -> myEPWM4 Pinmux
    //
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    //
    // EPWM5 -> myEPWM5 Pinmux
    //
    GPIO_setPinConfig(GPIO_8_EPWM5_A);
    GPIO_setPinConfig(GPIO_9_EPWM5_B);
    //
    // EPWM6 -> myEPWM6 Pinmux
    //
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setPinConfig(GPIO_11_EPWM6_B);

}

void EPWM_init(){
    //myEPWM1 initialization
    //myEPWM2 initialization
    //myEPWM3 initialization
    //myEPWM4 initialization
    //myEPWM5 initialization
    //myEPWM6 initialization
}



