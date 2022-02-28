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
void ADC_init(){

    ADC_setOffsetTrimAll(ADC_REFERENCE_INTERNAL,ADC_REFERENCE_3_3V); //3.3 internal volt ref
    ADC_setPrescaler(myADC0_BASE, ADC_CLK_DIV_2_0); // valami clockot osztunk 2vel
    ADC_setInterruptPulseMode(myADC0_BASE, ADC_PULSE_END_OF_CONV); //akkor lesz interrrupt, amikor megvannak a dolgaink
    ADC_enableConverter(myADC0_BASE); //beizzitjuk es varunk hogy folporogjon
    DEVICE_DELAY_US(5000);

    ADC_disableBurstMode(myADC0_BASE); //?
    ADC_setSOCPriority(myADC0_BASE, ADC_PRI_ALL_ROUND_ROBIN); // ennek tul sok ertelme nincs
    ADC_setupSOC(myADC0_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 8U); //hozzakapcsoljuk az adc0base-hez a 0.soc-ot, es epwm trigger, meg a 0. port
    ADC_setInterruptSOCTrigger(myADC0_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE); // az interrupt utan ne triggetelodjon mas

    ADC_setInterruptSource(myADC0_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER1); // ha a soc1 vegeter kuldje el az interrupt 1-et
    ADC_enableInterrupt(myADC0_BASE, ADC_INT_NUMBER1); //engedelyezzuk
    ADC_clearInterruptStatus(myADC0_BASE, ADC_INT_NUMBER1); //cleareljuk
    ADC_disableContinuousMode(myADC0_BASE, ADC_INT_NUMBER1); // csak clear utan johet a kovetkezo interrupt

}
// jo kerdes
void ASYSCTL_init(){
    // asysctl initialization
    // Disables the temperature sensor output to the ADC.
    ASysCtl_disableTemperatureSensor();
    // Set the analog voltage reference selection to internal.
    ASysCtl_setAnalogReferenceInternal( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
    // Set the internal analog voltage reference selection to 1.65V.
    ASysCtl_setAnalogReference1P65( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
}
void INTERRUPT_init(){

    // Interrupt Setings for INT_myADC0_1
    Interrupt_register(INT_myADC0_1, &adcA1ISR); // az a fuggveny fogja interruptolni
    Interrupt_enable(INT_myADC0_1);
}





