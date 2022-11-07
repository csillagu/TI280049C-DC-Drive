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
#define myEPWM4_BASE EPWM4_BASE



#define ADC_CURRENT ADCA_BASE
#define ADC_CURRENT_RESULT ADCARESULT_BASE

#define ADC_VOLTAGE ADCB_BASE
#define ADC_VOLTAGE_RESULT ADCBRESULT_BASE

#define ADC_REF ADCC_BASE
#define ADC_REF_RESULT ADCCRESULT_BASE


// interrupt 1
#define INT_ADC_CURR_1 INT_ADCA1
#define INT_ADC_CURR_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
#define INT_ADC_VOLTAGE_1 INT_ADCB1
#define INT_VOLTAGE_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
#define INT_ADC_REF_1 INT_ADCC1
#define INT_REF_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1

//interrupt 2
#define INT_ADC_CURR_2 INT_ADCA2
#define INT_ADC_CURR_2_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP10
#define INT_ADC_VOLTAGE_2 INT_ADCB2
#define INT_VOLTAGE_2_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP10
#define INT_ADC_REF_2 INT_ADCC2
#define INT_REF_2_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP10

#define GPIO_SD1 2 //GPIO2 and GPIO3 theoretically
#define GPIO_SD2 3


extern __interrupt void adcCurrent1(void);
extern __interrupt void adcVoltage1(void);
extern __interrupt void adcRef1(void);

extern __interrupt void adcCurrent2(void);
extern __interrupt void adcVoltage2(void);
extern __interrupt void adcRef2(void);

void    EPWM_Trigger_init();
void    ADC_init2();
void    ASYSCTL_init2();
void    INTERRUPT_init2();
void    Board_init2();
void    EPWM_init2();
void    PinMux_init2();
void    GPIO_init2();
#endif  // end of BOARD_H definition
