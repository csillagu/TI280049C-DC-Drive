
//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "adc/adc_board.h"

typedef enum{
    DRIVE_TYPE_UNIPOLAR_PLUS,
    DRIVE_TYPE_UNIPOLAR_MINUS,
    DRIVE_TYPE_BIPOLAR
}DRIVE_TYPE;

uint32_t EPWM_TIMER_TBPRD = 2500UL;
float32_t duty_cycle=10; //0...100
uint16_t adcResult=0;
uint16_t fed_set=110;
uint16_t red_set=110;

uint16_t c_meas=0;
uint16_t c_read=0;
uint16_t c_overflow=0;
float32_t current=0;
float32_t c_avgt[5]={0};
float32_t c_avg=0;
uint16_t c_count=0;


uint16_t v_meas=0;
uint16_t v_read=1;
uint16_t v_overflow=0;
float32_t voltage=0;
uint16_t v_avg[5]={0};
uint16_t v_count=0;

uint16_t r_meas=0;
uint16_t r_read=0;
uint16_t r_overflow=0;
float32_t ref=0.0;

float32_t i_ba=0; //alapjel
uint16_t cont=0; //szabalyozo mod eng



DRIVE_TYPE drive=DRIVE_TYPE_UNIPOLAR_PLUS;
//
// Function Prototypes
//
void initEPWMWithoutDB(uint32_t base);
void setupEPWMActiveHigh(uint32_t base);
void setupEPWMActiveLow(uint32_t base);
void setupEPWMActiveHighComplementary(uint32_t base);
void setupEPWMActiveLowComplementary(uint32_t base);
void setupEPWMOutputSwap(uint32_t base);
__interrupt void adcRef(void);
__interrupt void adcCurrent(void);
__interrupt void adcVoltage(void);
void syncEpwm();


void main(void)
{
    float ik1=0;
    float uk1=0;


    uint16_t EPWM_TIMER_TBPRD_old=EPWM_TIMER_TBPRD;
    float32_t duty_cycle_old=0;
    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // Configure ePWMs
    //
    Board_init2();

    //
    // Disable sync(Freeze clock to PWM as well). GTBCLKSYNC is applicable
    // only for multiple core devices. Uncomment the below statement if
    // applicable.

    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);


    EPWM_Trigger_init();


    EPWM_setTimeBasePeriod(myEPWM1_BASE, EPWM_TIMER_TBPRD); //up/down count
    EPWM_setPhaseShift(myEPWM1_BASE, 0U);  //?
    EPWM_setTimeBaseCounter(myEPWM1_BASE, 0U);  // honnan indul?
    EPWM_setTimeBaseCounterMode(myEPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN); // fel/le szamoljon
    EPWM_disablePhaseShiftLoad(myEPWM1_BASE);

    //
    // Set ePWM clock pre-scaler
    //
    EPWM_setClockPrescaler(myEPWM1_BASE,
                      EPWM_CLOCK_DIVIDER_1,
                      EPWM_HSCLOCK_DIVIDER_2); //1*2-gyel osztjuk le a sysclk/2-t
    //50MHz-es TBCLK

    //
    // Set up shadowing
    //
    EPWM_setCounterCompareShadowLoadMode(myEPWM1_BASE,
                                    EPWM_COUNTER_COMPARE_A,
                                    EPWM_COMP_LOAD_ON_CNTR_ZERO); //ha 0, akkor toltse bele a shadow registerből az értéket

    //
    // Set-up compare
    //
    EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100); //beállíthatunk két comparet, meg kell nezni hogy megy

    //
    // Set actions
    //
    EPWM_setActionQualifierAction(myEPWM1_BASE, // mi tortenjen amikor a kulonbozo compare eventek bekovetkeznek
                                     EPWM_AQ_OUTPUT_A,
                                     EPWM_AQ_OUTPUT_HIGH,
                                     EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(myEPWM1_BASE,
                                     EPWM_AQ_OUTPUT_A,
                                     EPWM_AQ_OUTPUT_LOW,
                                     EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(myEPWM1_BASE,
                                     EPWM_AQ_OUTPUT_A,
                                     EPWM_AQ_OUTPUT_HIGH,
                                     EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    EPWM_setRisingEdgeDeadBandDelayInput(myEPWM1_BASE, EPWM_DB_INPUT_EPWMA); // ez alapjan lesz delay mostantol

    EPWM_setFallingEdgeDeadBandDelayInput(myEPWM1_BASE, EPWM_DB_INPUT_EPWMA); // same

    EPWM_setFallingEdgeDelayCount(myEPWM1_BASE, fed_set); // eddig szamol el, amig delayeli a váltást
    EPWM_setRisingEdgeDelayCount(myEPWM1_BASE, red_set); // szintén csak felfutóélnéls
    //
    // Ez a 4 sor gyakorlatilag beállítja a megfelelő módot:  HWREGH(myEPWM1_BASE + EPWM_O_DBCTL) =0x000B; // beallitja a modot a megfelelore lsd spuri/1812.o teteje
    //
    EPWM_setDeadBandDelayPolarity(myEPWM1_BASE, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH); // kivonja, vagy hozzaadja a szamolashoz a deadbandet (?)
    EPWM_setDeadBandDelayPolarity(myEPWM1_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW); //?

    EPWM_setDeadBandOutputSwapMode(myEPWM1_BASE, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(myEPWM1_BASE, EPWM_DB_OUTPUT_B, false);

    EPWM_setDeadBandDelayMode(myEPWM1_BASE, EPWM_DB_RED, true); // gyakorlatilag elmenti amit beallitottunk eddig (?)
    EPWM_setDeadBandDelayMode(myEPWM1_BASE, EPWM_DB_FED, true); //?
    //// EPWM2

    EPWM_setTimeBasePeriod(myEPWM4_BASE, EPWM_TIMER_TBPRD); //up/down count
    EPWM_setPhaseShift(myEPWM4_BASE, 0U);  //?
    EPWM_setTimeBaseCounter(myEPWM4_BASE, 0U);  // honnan indul?
    EPWM_setTimeBaseCounterMode(myEPWM4_BASE, EPWM_COUNTER_MODE_UP_DOWN); // fel/le szamoljon
    EPWM_disablePhaseShiftLoad(myEPWM4_BASE);

    //
    // Set ePWM clock pre-scaler
    //
    EPWM_setClockPrescaler(myEPWM4_BASE,
                               EPWM_CLOCK_DIVIDER_1,
                               EPWM_HSCLOCK_DIVIDER_2); //1*2-gyel osztjuk le a sysclk/2-t
    //50MHz-es TBCLK

    //
    // Set up shadowing
    //
    EPWM_setCounterCompareShadowLoadMode(myEPWM4_BASE,
                                     EPWM_COUNTER_COMPARE_A,
                                     EPWM_COMP_LOAD_ON_CNTR_ZERO); //ha 0, akkor toltse bele a shadow registerből az értéket

    //
    // Set-up compare
    //
    EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, EPWM_TIMER_TBPRD); //beállíthatunk két comparet, meg kell nezni hogy megy

    //
    // Set actions
    //
    // mi tortenjen amikor a kulonbozo compare eventek bekovetkeznek

    EPWM_setActionQualifierAction(myEPWM4_BASE, // mi tortenjen amikor a kulonbozo compare eventek bekovetkeznek
                                     EPWM_AQ_OUTPUT_B,
                                     EPWM_AQ_OUTPUT_HIGH,
                                     EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(myEPWM4_BASE,
                                     EPWM_AQ_OUTPUT_B,
                                     EPWM_AQ_OUTPUT_LOW,
                                     EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(myEPWM4_BASE,
                                     EPWM_AQ_OUTPUT_B,
                                     EPWM_AQ_OUTPUT_HIGH,
                                     EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    /*EPWM_setActionQualifierAction(myEPWM4_BASE,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(myEPWM4_BASE,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(myEPWM4_BASE,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);*/

    EPWM_setRisingEdgeDeadBandDelayInput(myEPWM4_BASE, EPWM_DB_INPUT_EPWMB); // ez alapjan lesz delay mostantol

    EPWM_setFallingEdgeDeadBandDelayInput(myEPWM4_BASE, EPWM_DB_INPUT_EPWMB); // same

    EPWM_setFallingEdgeDelayCount(myEPWM4_BASE, fed_set); // eddig szamol el, amig delayeli a váltást
    EPWM_setRisingEdgeDelayCount(myEPWM4_BASE, red_set); // szintén csak felfutóélnéls
    //
    // Ez a 4 sor gyakorlatilag beállítja a megfelelő módot:  HWREGH(myEPWM1_BASE + EPWM_O_DBCTL) =0x000B; // beallitja a modot a megfelelore lsd spuri/1812.o teteje
    //
    EPWM_setDeadBandDelayPolarity(myEPWM4_BASE, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH); // kivonja, vagy hozzaadja a szamolashoz a deadbandet (?)
    EPWM_setDeadBandDelayPolarity(myEPWM4_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW); //?

    EPWM_setDeadBandOutputSwapMode(myEPWM4_BASE, EPWM_DB_OUTPUT_A, true);
    EPWM_setDeadBandOutputSwapMode(myEPWM4_BASE, EPWM_DB_OUTPUT_B, true);

    EPWM_setDeadBandDelayMode(myEPWM4_BASE, EPWM_DB_RED, true); // gyakorlatilag elmenti amit beallitottunk eddig (?)
    EPWM_setDeadBandDelayMode(myEPWM4_BASE, EPWM_DB_FED, true); //?


    Interrupt_enable(INT_ADCA1);
    Interrupt_enable(INT_ADCB1);
    Interrupt_enable(INT_ADCC1);
    EINT;
    ERTM;

    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC); // ráengedjük a clockot, ilyenkor kezd el működni

    //syncEpwm();




    for(;;)
    {
        NOP;
       // DEVICE_DELAY_US(10000000);
        if(duty_cycle!=duty_cycle_old||EPWM_TIMER_TBPRD_old!=EPWM_TIMER_TBPRD){
            if(duty_cycle>100)
                duty_cycle=duty_cycle_old; /// veszmegoldas tul nagy fesz ellen
            duty_cycle_old=duty_cycle;
            switch(drive){
            case DRIVE_TYPE_UNIPOLAR_PLUS:
                //TODO - plusznal es minusznal is hasnoloan mukodjon a duty cycle noveli-csokkenti -lenyeg, hogy tudjuk hogy mi a fasz van.

                EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100); //beállíthatunk két comparet, meg kell nezni hogy megy
                EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, EPWM_TIMER_TBPRD);//--------->>> 0 lesz a kimeno fesz ott
                break;
            case DRIVE_TYPE_UNIPOLAR_MINUS:
                EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, EPWM_TIMER_TBPRD); //beállíthatunk két comparet, meg kell nezni hogy megy
                EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100);//--------->>> 0 lesz a kimeno fesz ott
                break;
            case DRIVE_TYPE_BIPOLAR:
                //TODO itt nem ugyanazok a duty cyclek kellenek? elvileg be van állítva? DONE

                EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100); //beállíthatunk két comparet, meg kell nezni hogy megy
                EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100);//--------->>> 0 lesz a kimeno fesz ott
                //syncEpwm();
                break;
            }
        }


        if(EPWM_TIMER_TBPRD_old!=EPWM_TIMER_TBPRD){
            EPWM_TIMER_TBPRD_old=EPWM_TIMER_TBPRD;
            EPWM_setTimeBasePeriod(myEPWM1_BASE, EPWM_TIMER_TBPRD);
            EPWM_setTimeBasePeriod(myEPWM4_BASE, EPWM_TIMER_TBPRD);
        }
        if(c_read==1&&r_read==1&&v_read==1){
            c_read=0;
            r_read=0;
            v_read=0;
            current=((int16_t)c_meas-(int16_t)r_meas)*0.0322266+0.1;//magic constant
            c_avgt[(c_count+1)%5]=current;
            c_count++;
            c_avg= (c_avgt[0]+c_avgt[1]+c_avgt[2]+c_avgt[3]+c_avgt[4])*1.0/5.0; // test for identification

            if(cont){
                float32_t ih=i_ba-current;
                duty_cycle=(ih*100-37.5*ik1)/25.0+uk1;
                ik1=ih;
                uk1=duty_cycle;
            }

            //current=c_meas*1.0/4096*130.4348-ref/0.0253;

            //voltage=((v_avg[0]+v_avg[1]+v_avg[2]+v_avg[3]+v_avg[4])*1.0*3.3-ref)/40.960;
            //itt minden megvan, mehet a beavatkozojel szamolas

        }
     }

}
    //EPWM SYNC -> mindezt az AQ előtt kéne
    //EPWM_setCountModeAfterSync merre számoljon sync után (ugyanaz vagy fordítva a jó?
    //EPWM_forceSyncPulse ez elv a phasere állítja be
    //EPWM_setSyncOutPulseMode automatikusan lehet synceltetni, ha ez nem kell, elv akkoris kell valamit beallitani
    //
void syncEpwm(){
    EPWM_setCountModeAfterSync(myEPWM1_BASE, EPWM_COUNT_MODE_UP_AFTER_SYNC);
    EPWM_setCountModeAfterSync(myEPWM4_BASE, EPWM_COUNT_MODE_DOWN_AFTER_SYNC);
    EPWM_forceSyncPulse(myEPWM1_BASE);
    EPWM_forceSyncPulse(myEPWM1_BASE);
}

__interrupt void adcVoltage(void){
    //
    // Store results
    //
    v_meas=ADC_readResult(ADC_VOLTAGE_RESULT, ADC_SOC_NUMBER0);
    v_avg[(v_count+1)%5]=v_meas;
    v_count++;

    voltage= (v_avg[0]+v_avg[1]+v_avg[2]+v_avg[3]+v_avg[4])*1.0/6206.06061; // test for identification
    v_read=1;

    ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);// oke, lekezeltuk

    //
    // Check if overflow has occurred
    //
    if(true == ADC_getInterruptOverflowStatus(ADC_VOLTAGE, ADC_INT_NUMBER1))
    {
        v_overflow++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);
    }

    //
    // Acknowledge the interrupt ??
    //
    Interrupt_clearACKGroup( INT_VOLTAGE_1_INTERRUPT_ACK_GROUP);
}

__interrupt void adcCurrent(void){
    //
    // Store results
    //
    c_meas= ADC_readResult(ADC_CURRENT_RESULT, ADC_SOC_NUMBER0);


    c_read=1;


    //
    // Clear the interrupt flag
    //
    ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER1);// oke, lekezeltuk

    //
    // Check if overflow has occurred
    //
    if(true == ADC_getInterruptOverflowStatus(ADC_CURRENT, ADC_INT_NUMBER1))
    {
        c_overflow++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_CURRENT, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER1);
    }

    //
    // Acknowledge the interrupt ??
    //
    Interrupt_clearACKGroup(INT_ADC_CURR_1_INTERRUPT_ACK_GROUP);

}

__interrupt void adcRef(void){
    //
    // Store results
    //
    r_meas= ADC_readResult(ADC_REF_RESULT, ADC_SOC_NUMBER0);
    r_read=1;
    ref=r_meas*1.0/4096.0*3.3;
    //
    // Clear the interrupt flag
    //
    ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER1);// oke, lekezeltuk

    //
    // Check if overflow has occurred
    //
    if(true == ADC_getInterruptOverflowStatus(ADC_REF, ADC_INT_NUMBER1))
    {
        r_overflow++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_REF, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER1);
    }

    //
    // Acknowledge the interrupt ??
    //
    Interrupt_clearACKGroup(INT_REF_1_INTERRUPT_ACK_GROUP);

}

void setupEPWMOutputSwap(uint32_t base)
{

    //
    // Disable RED
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, false);

    //
    // Disable FED
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, false);

    //
    // Switch Output A with Output B
    //
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_A, true);
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_B, true);

}
/*
void setupEPWMActiveHigh(uint32_t base)
{
    //
    // Use EPWMA as the input for both RED and FED
    //
    EPWM_setRisingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);

    //
    // Set the RED and FED values
    //
    EPWM_setFallingEdgeDelayCount(base, 200);
    EPWM_setRisingEdgeDelayCount(base, 400);

    //
    // Do not invert the delayed outputs (AH)
    //
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);

    //
    // Use the delayed signals instead of the original signals
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);

    //
    // DO NOT Switch Output A with Output B
    //
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_B, false);

}

void setupEPWMActiveLowComplementary(uint32_t base)
{
    //
    // Use EPWMA as the input for both RED and FED
    //
    EPWM_setRisingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);

    //
    // Set the RED and FED values
    //
    EPWM_setFallingEdgeDelayCount(base, 200);
    EPWM_setRisingEdgeDelayCount(base, 400);

    //
    // Invert only the Rising Edge delayed output (ALC)
    //
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_LOW);
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_HIGH);

    //
    // Use the delayed signals instead of the original signals
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);

    //
    // DO NOT Switch Output A with Output B
    //
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_B, false);

}


void setupEPWMActiveHighComplementary(uint32_t base)
{
    //
    // Use EPWMA as the input for both RED and FED
    //
    EPWM_setRisingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);

    //
    // Set the RED and FED values
    //
    EPWM_setFallingEdgeDelayCount(base, 200);
    EPWM_setRisingEdgeDelayCount(base, 400);

    //
    // Invert only the Falling Edge delayed output (AHC)
    //
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);

    //
    // Use the delayed signals instead of the original signals
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);

    //
    // DO NOT Switch Output A with Output B
    //
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_B, false);

}



void setupEPWMActiveLow(uint32_t base)
{
    //
    // Use EPWMA as the input for both RED and FED
    //
    EPWM_setRisingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);

    //
    // Set the RED and FED values
    //
    EPWM_setFallingEdgeDelayCount(base, 200);
    EPWM_setRisingEdgeDelayCount(base, 400);

    //
    // INVERT the delayed outputs (AL)
    //
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_LOW);
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);

    //
    // Use the delayed signals instead of the original signals
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);

    //
    // DO NOT Switch Output A with Output B
    //
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_B, false);

}


//
// initEPWM - Configure ePWM1
//
void initEPWMWithoutDB(uint32_t base)
{
    //
    // Set-up TBCLK
    //
    EPWM_setTimeBasePeriod(base, EPWM_TIMER_TBPRD);
    EPWM_setPhaseShift(base, 0U);
    EPWM_setTimeBaseCounter(base, 0U);
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_disablePhaseShiftLoad(base);

    //
    // Set ePWM clock pre-scaler
    //
    EPWM_setClockPrescaler(base,
                           EPWM_CLOCK_DIVIDER_4,
                           EPWM_HSCLOCK_DIVIDER_4);

    //
    // Set up shadowing
    //
    EPWM_setCounterCompareShadowLoadMode(base,
                                         EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);

    //
    // Set-up compare
    //
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_A, EPWM_TIMER_TBPRD/4);
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_B, 3*EPWM_TIMER_TBPRD/4);

    //
    // Set actions
    //
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_NO_CHANGE,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);


    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_B,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_B,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_B,
                                      EPWM_AQ_OUTPUT_NO_CHANGE,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setActionQualifierAction(base,
                                      EPWM_AQ_OUTPUT_B,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);

}*/
