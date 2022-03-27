#ifndef adcboard
#define adcboard

#include "driverlib.h"
#include "device.h"

#define GPIO_PIN_EPWM1_A 0
#define GPIO_PIN_EPWM1_B 1
#define GPIO_PIN_EPWM2_A 2
#define GPIO_PIN_EPWM2_B 3
#define GPIO_PIN_EPWM3_A 4
#define GPIO_PIN_EPWM3_B 5
#define GPIO_PIN_EPWM4_A 6
#define GPIO_PIN_EPWM4_B 7
#define GPIO_PIN_EPWM5_A 8
#define GPIO_PIN_EPWM5_B 9
#define GPIO_PIN_EPWM6_A 10
#define GPIO_PIN_EPWM6_B 11

#define myEPWM1_BASE EPWM1_BASE
#define myEPWM2_BASE EPWM2_BASE
#define myEPWM3_BASE EPWM3_BASE
#define myEPWM4_BASE EPWM4_BASE
#define myEPWM5_BASE EPWM5_BASE
#define myEPWM6_BASE EPWM6_BASE


#define ADC_CURRENT ADCA_BASE
#define ADC_CURRENT_RESULT ADCARESULT_BASE

#define ADC_VOLTAGE ADCB_BASE
#define ADC_VOLTAGE_BASE ADCBRESULT_BASE

#define ADC_REF ADCC_BASE
#define ADC_REF_BASE ADCCRESULT_BASE


// Interrupt Setings for INT_myADC0_1
#define INT_ADC_CURR_1 INT_ADCA1
#define INT_ADC_CURR_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
#define INT_ADC_VOLTAGE_1 INT_ADCB1
#define INT_VOLTAGE_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP2
#define INT_ADC_REF_1 INT_ADCC1
#define INT_REF_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP3


extern __interrupt void adcA1ISR(void);



void    ADC_init2();
void    ASYSCTL_init2();
void    INTERRUPT_init2();
void    Board_init2();
void    EPWM_init2();
void    PinMux_init2();

#endif  // end of BOARD_H definition
