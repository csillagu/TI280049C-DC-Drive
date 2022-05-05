/*
 * adc_board.c
 *
 *  Created on: Feb 28, 2022
 *      Author: ujvar
 */

#include "adc_board.h"

void Board_init2()
{
    EALLOW;

    PinMux_init2();
    EPWM_init2();
    ASYSCTL_init2();
    ADC_init2();
    INTERRUPT_init2();

    EDIS;
}

void PinMux_init2()
{
    //
    // EPWM1 -> myEPWM1 Pinmux
    //
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setPinConfig(GPIO_1_EPWM1_B);

// EPWM4
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    //SD1
    GPIO_setPinConfig(GPIO_2_GPIO2);
    //SD2
    GPIO_setPinConfig(GPIO_3_GPIO3);

}

void EPWM_init2(){
    //myEPWM1 initialization
    //myEPWM2 initialization
    //myEPWM3 initialization
    //myEPWM4 initialization
    //myEPWM5 initialization
    //myEPWM6 initialization
}
void ADC_init2(){

    ADC_setOffsetTrimAll(ADC_REFERENCE_INTERNAL,ADC_REFERENCE_3_3V); //3.3 internal volt ref

    // ADCINA6 (arammeres)
    ADC_setPrescaler(ADC_CURRENT, ADC_CLK_DIV_2_0); // valami clockot osztunk 2vel
    ADC_setInterruptPulseMode(ADC_CURRENT, ADC_PULSE_END_OF_CONV); //akkor lesz interrrupt, amikor megvannak a dolgaink
    ADC_enableConverter(ADC_CURRENT); //beizzitjuk es varunk hogy folporogjon
    DEVICE_DELAY_US(5000);

    ADC_disableBurstMode(ADC_CURRENT); //?
    ADC_setSOCPriority(ADC_CURRENT, ADC_PRI_ALL_ROUND_ROBIN); // ha tob soc akarja hasznalni az adcA-t, akkor melyikkel kezdjuk
    ADC_setupSOC(ADC_CURRENT, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN6, 8U); //hozzakapcsoljuk az adc6base-hez a 0.soc-ot, es epwm trigger, meg a 0. port
    ADC_setInterruptSOCTrigger(ADC_CURRENT, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE); // az interrupt utan ne triggetelodjon mas

    ADC_setInterruptSource(ADC_CURRENT, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_enableInterrupt(ADC_CURRENT, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER1);
    ADC_disableContinuousMode(ADC_CURRENT, ADC_INT_NUMBER1);

    // ADCINB6 (feszmeres)
   ADC_setPrescaler(ADC_VOLTAGE, ADC_CLK_DIV_2_0);
   ADC_setInterruptPulseMode(ADC_VOLTAGE, ADC_PULSE_END_OF_CONV);
   ADC_enableConverter(ADC_VOLTAGE);
   DEVICE_DELAY_US(5000);

   ADC_disableBurstMode(ADC_VOLTAGE);
   ADC_setSOCPriority(ADC_VOLTAGE, ADC_PRI_ALL_ROUND_ROBIN);

   ADC_setupSOC(ADC_VOLTAGE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN6, 8U); //??????? tesztelni kell hogy jo e

   ADC_setInterruptSOCTrigger(ADC_VOLTAGE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE); // az adcint1 utan triggerelodjon ez a cucc
   ADC_setInterruptSource(ADC_VOLTAGE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
   ADC_enableInterrupt(ADC_VOLTAGE, ADC_INT_NUMBER1);
   ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);
   ADC_disableContinuousMode(ADC_VOLTAGE, ADC_INT_NUMBER1);

   // referencia
  ADC_setPrescaler(ADC_REF, ADC_CLK_DIV_2_0);
  ADC_setInterruptPulseMode(ADC_REF, ADC_PULSE_END_OF_CONV);
  ADC_enableConverter(ADC_REF);
  DEVICE_DELAY_US(5000);

  ADC_disableBurstMode(ADC_REF);
  ADC_setSOCPriority(ADC_REF, ADC_PRI_ALL_ROUND_ROBIN);

  ADC_setupSOC(ADC_REF, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN14, 8U); //??????? tesztelni kell hogy jo e

  ADC_setInterruptSOCTrigger(ADC_REF, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
  ADC_setInterruptSource(ADC_REF, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
  ADC_enableInterrupt(ADC_REF, ADC_INT_NUMBER1);
  ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER1);
  ADC_disableContinuousMode(ADC_REF, ADC_INT_NUMBER1); //intek helyett, a legvegen kene egy darb nagy int



}
// jo kerdes
void ASYSCTL_init2(){
    // asysctl initialization
    // Disables the temperature sensor output to the ADC.
    ASysCtl_disableTemperatureSensor();
    // Set the analog voltage reference selection to internal.
    ASysCtl_setAnalogReferenceInternal( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
    // Set the internal analog voltage reference selection to 1.65V.
    ASysCtl_setAnalogReference1P65( ASYSCTL_VREFHIA | ASYSCTL_VREFHIB | ASYSCTL_VREFHIC );
}
void INTERRUPT_init2(){

    // Interrupt Setings for INT_ADC_CURR_1
    Interrupt_register(INT_ADC_CURR_1, &adcCurrent); // az a fuggveny fogja interruptolni
    Interrupt_enable(INT_ADC_CURR_1);

    // voltage
    Interrupt_register(INT_ADC_VOLTAGE_1, &adcVoltage); // az a fuggveny fogja interruptolni
        Interrupt_enable(INT_ADC_VOLTAGE_1);
    Interrupt_register(INT_ADC_REF_1, &adcRef); // az a fuggveny fogja interruptolni
            Interrupt_enable(INT_ADC_REF_1);

}





