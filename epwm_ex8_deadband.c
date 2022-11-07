
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
//TEST
long long dc_of=0;
//END

uint32_t EPWM_TIMER_TBPRD = 1250UL;
float32_t duty_cycle=50; //0...100
#define DUTY_MAX 90
#define V_MAX 25
uint16_t adcResult=0;
uint16_t fed_set=110;
uint16_t red_set=110;

//interrupt1, current
uint16_t c_meas_at1=0;
uint16_t c_read_at1=0;
uint16_t c_overflow_at1=0;
float32_t current_at1=0;
//interrupt2, current
uint16_t c_meas_at2=0;
uint16_t c_read_at2=0;
uint16_t c_overflow_at2=0;

//current average value
float32_t current=0;
float32_t c_avgt[5]={0};
float32_t c_avg=0;
uint16_t c_count=0;

//interrupt1, voltage
uint16_t v_meas_at1=0;
uint16_t v_read_at1=1;
uint16_t v_overflow_at1=0;
float32_t voltage_at1=0;

//interrupt2, voltage
uint16_t v_meas_at2=0;
uint16_t v_read_at2=1;
uint16_t v_overflow_at2=0;

//voltage average value
float32_t voltage=0;
float32_t v_avgt[5]={0};
float32_t v_avg=0;
uint16_t v_count=0;

//interrupt1, reference
uint16_t r_meas_at1=0;
uint16_t r_read_at1=0;
uint16_t r_overflow_at1=0;
float32_t ref_at1=0.0;

//interrupt2, reference
uint16_t r_meas_at2=0;
uint16_t r_read_at2=0;
uint16_t r_overflow_at2=0;

//reference final value
float32_t ref=0.0;

//controller
float32_t i_ba=0; //setpoint (alapjel)
uint16_t cont=0; //enable controller mode
float32_t Ap=0.9914; //proportional term
float32_t K_ti=0.9797;
float32_t integ=1;
float32_t u_o=0;
int16_t ibamult=1;

//STATE CONTROLL
typedef enum {STOP=0, START=1} STATE;
STATE state=STOP;
STATE state_old=START;

DRIVE_TYPE drive=DRIVE_TYPE_BIPOLAR;//_PLUS;
//
// Function Prototypes
//
void initEPWMWithoutDB(uint32_t base);
void setupEPWMActiveHigh(uint32_t base);
void setupEPWMActiveLow(uint32_t base);
void setupEPWMActiveHighComplementary(uint32_t base);
void setupEPWMActiveLowComplementary(uint32_t base);
void setupEPWMOutputSwap(uint32_t base);
__interrupt void adcRef1(void);
__interrupt void adcCurrent1(void);
__interrupt void adcVoltage1(void);

__interrupt void adcRef2(void);
__interrupt void adcCurrent2(void);
__interrupt void adcVoltage2(void);
void syncEpwm();

void setupEpwm1(uint32_t EPWM_TIMER_TBPRD, float32_t duty_cycle, uint16_t fed_set, uint16_t red_set);
void setupEpwm2(uint32_t EPWM_TIMER_TBPRD, uint16_t fed_set, uint16_t red_set);
uint16_t duty_null();


void main(void)
{
    float32_t ik1=0;
    float32_t uk1=0;


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


    setupEpwm1(EPWM_TIMER_TBPRD, duty_cycle, fed_set, red_set);
    //// EPWM2
    setupEpwm2(EPWM_TIMER_TBPRD, fed_set, red_set);
    //TODO delete these if not needed
    Interrupt_enable(INT_ADCA1);
    Interrupt_enable(INT_ADCB1);
    Interrupt_enable(INT_ADCC1);

    Interrupt_enable(INT_ADCA2);
    Interrupt_enable(INT_ADCB2);
    Interrupt_enable(INT_ADCC2);
    EINT;
    ERTM;

    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_B);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC); // start pwm clock




    for(;;)
    {
        NOP;
        //change of duty cycle or timer period

        if(state!=state_old){
            state_old=state;
            if(state==STOP){
                GPIO_writePin(GPIO_SD1, 1);
                GPIO_writePin(GPIO_SD2, 1);

            }
            if(state==START&&current<20){
                GPIO_writePin(GPIO_SD1, 0);
                GPIO_writePin(GPIO_SD2, 0);

                duty_cycle=duty_null();
            }
        }

        if(state&&(duty_cycle!=duty_cycle_old||EPWM_TIMER_TBPRD_old!=EPWM_TIMER_TBPRD)){
            if(duty_cycle>100){
                dc_of++;
                duty_cycle=duty_cycle_old; /// veszmegoldas tul nagy fesz ellen
            }
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

                EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100); //beállíthatunk két comparet, meg kell nezni hogy megy
                EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, duty_cycle*EPWM_TIMER_TBPRD/100);//--------->>> 0 lesz a kimeno fesz ott
                //syncEpwm();
                break;
            }
        }
        //change of timer period
        if(EPWM_TIMER_TBPRD_old!=EPWM_TIMER_TBPRD){
            EPWM_TIMER_TBPRD_old=EPWM_TIMER_TBPRD;
            EPWM_setTimeBasePeriod(myEPWM1_BASE, EPWM_TIMER_TBPRD);
            EPWM_setTimeBasePeriod(myEPWM4_BASE, EPWM_TIMER_TBPRD);
        }
        //interrupt 1
        if(c_read_at1==1&&r_read_at1==1&&v_read_at1==1){
            //reset SET flags
            c_read_at1=0;
            r_read_at1=0;
            v_read_at1=0;
            //REFERENCE related values
            ref_at1=r_meas_at1*1.0/4096.0*3.3;
            //CURRENT related values
            current_at1=((int16_t)c_meas_at1-(int16_t)r_meas_at1)*0.0322266;//magic constant


            //VOLTAGE related values
            voltage_at1=(v_meas_at1*1.0*3.3)/4096-ref_at1;
            //v_avgt[(v_count+1)%5]=voltage_at1;
            //v_count++;
            //v_avg= (v_avgt[0]+v_avgt[1]+v_avgt[2]+v_avgt[3]+v_avgt[4])*1.0/6206.06061; // test for identification
            //CONTROLLER is in the other interrupt

            //current=(c_meas*1.0/4096-ref*1.0/4096)*3.3*
            //current=c_meas*1.0/4096*130.4348-ref/0.0253;

            //voltage=((v_avg[0]+v_avg[1]+v_avg[2]+v_avg[3]+v_avg[4])*1.0*3.3-ref)/40.960;
            //itt minden megvan, mehet a beavatkozojel szamolas
            }
        if(c_read_at2==1&&r_read_at2==1&&v_read_at2==1){
            //reset SET flags
            //c_read_at2=0;
            //r_read_at2=0;
            //v_read_at2=0;
            //REFERENCE related values
            float32_t ref_at2=r_meas_at2*1.0/4096.0*3.3;
            //CURRENT related values
            float32_t current_at2=((int16_t)c_meas_at2-(int16_t)r_meas_at2)*0.0322266;//magic constant

            ref=ref_at2;

            current=((current_at2+current_at1)/2.0+0.3555)/1.0367; //correction from ident
            c_avgt[(c_count+1)%5]=current;
            c_count++;
            c_avg= (c_avgt[0]+c_avgt[1]+c_avgt[2]+c_avgt[3]+c_avgt[4])*1.0/5.0; // test for identification
            //SAFETY
            if(current>20){
                state=STOP;
            }


            //VOLTAGE related values
            //TODO voltage is definitely not good like this (amplifiers amplification needs to be added)
            float32_t voltage_at2=(v_meas_at2*1.0*3.3)/4096.0-ref_at2;

            voltage=(voltage_at2+voltage_at1)/2.0*(-100.73)-0.7349;//corrected from ident

            v_avgt[(v_count+1)%5]=voltage;
            v_count++;
            v_avg= (v_avgt[0]+v_avgt[1]+v_avgt[2]+v_avgt[3]+v_avgt[4])/5;

            //CONTROLLER
            if(cont&&state){
               float32_t ie=i_ba*ibamult-current; //current error
               u_o=Ap*(ie-K_ti*ik1)+uk1*integ; //*100 maybe?
               //if(u_o>100)
                 //  u_o=uk1;
               ik1=ie;

               float32_t umax=0;
               float32_t umin=0;

               //integrator max output
               if(drive==DRIVE_TYPE_UNIPOLAR_PLUS){
                   duty_cycle=u_o/V_MAX*100;
                   umax=V_MAX*DUTY_MAX/100.0;
                   umin=0;
               }else if(drive==DRIVE_TYPE_BIPOLAR){
                   //TODO !!!! duty_cycle=u_o*50.0/V_MAX+50 és az umax umin is szar (+-25V kéne!!)
                   duty_cycle=u_o/V_MAX*50+50;
                   umax=V_MAX*DUTY_MAX/100.0;
                   umin=-V_MAX*DUTY_MAX/100.0;
               }


               if(duty_cycle>DUTY_MAX){
                   duty_cycle=DUTY_MAX;
                   u_o=umax;
                   dc_of++;

               }
               if(duty_cycle<0){
                   duty_cycle=0;
                   u_o=umin;
                   dc_of++;

               }
               uk1=u_o;

            }
        }

     }

}

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

__interrupt void adcVoltage1(void){
    //store result
    v_meas_at1=ADC_readResult(ADC_VOLTAGE_RESULT, ADC_SOC_NUMBER0);
    //SET bit 1
    v_read_at1=1;
    ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);// oke, lekezeltuk

    // Check if overflow has occurred
    if(true == ADC_getInterruptOverflowStatus(ADC_VOLTAGE, ADC_INT_NUMBER1))
    {
        v_overflow_at1++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER1);
    }

    // Acknowledge the interrupt ??
    Interrupt_clearACKGroup( INT_VOLTAGE_1_INTERRUPT_ACK_GROUP);
}

__interrupt void adcCurrent1(void){
    //store result
    c_meas_at1= ADC_readResult(ADC_CURRENT_RESULT, ADC_SOC_NUMBER0);
    c_read_at1=1;
    //
    // Clear the interrupt flag
    //
    ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER1);// oke, lekezeltuk

    //
    // Check if overflow has occurred
    //
    if(true == ADC_getInterruptOverflowStatus(ADC_CURRENT, ADC_INT_NUMBER1))
    {
        c_overflow_at1++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_CURRENT, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER1);
    }

    //
    // Acknowledge the interrupt ??
    //
    Interrupt_clearACKGroup(INT_ADC_CURR_1_INTERRUPT_ACK_GROUP);

}

__interrupt void adcRef1(void){
    //store result

    r_meas_at1= ADC_readResult(ADC_REF_RESULT, ADC_SOC_NUMBER0);
    r_read_at1=1;

    //
    // Clear the interrupt flag
    //
    ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER1);// oke, lekezeltuk

    //
    // Check if overflow has occurred
    //
    if(true == ADC_getInterruptOverflowStatus(ADC_REF, ADC_INT_NUMBER1))
    {
        r_overflow_at1++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_REF, ADC_INT_NUMBER1);
        ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER1);
    }

    //
    // Acknowledge the interrupt ??
    //
    Interrupt_clearACKGroup(INT_REF_1_INTERRUPT_ACK_GROUP);

}

__interrupt void adcRef2(void){
    //store result
    r_meas_at2=ADC_readResult(ADC_REF_RESULT, ADC_SOC_NUMBER1);
    //SET bit 1
    r_read_at2=1;

    ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER2);// oke, lekezeltuk

    // Check if overflow has occurred

    if(true == ADC_getInterruptOverflowStatus(ADC_REF, ADC_INT_NUMBER2))
    {
        r_overflow_at2++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_REF, ADC_INT_NUMBER2);
        ADC_clearInterruptStatus(ADC_REF, ADC_INT_NUMBER2);
    }

    // Acknowledge the interrupt ??
    Interrupt_clearACKGroup( INT_REF_2_INTERRUPT_ACK_GROUP);
}
__interrupt void adcCurrent2(void){
    //store result
    c_meas_at2=ADC_readResult(ADC_CURRENT_RESULT, ADC_SOC_NUMBER1);
    //SET bit 1
    c_read_at2=1;

    ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER2);// oke, lekezeltuk

    // Check if overflow has occurred

    if(true == ADC_getInterruptOverflowStatus(ADC_CURRENT, ADC_INT_NUMBER2))
    {
        c_overflow_at2++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_CURRENT, ADC_INT_NUMBER2);
        ADC_clearInterruptStatus(ADC_CURRENT, ADC_INT_NUMBER2);
    }

    // Acknowledge the interrupt ??
    Interrupt_clearACKGroup( INT_ADC_CURR_2_INTERRUPT_ACK_GROUP);
}
__interrupt void adcVoltage2(void){
    //store result
    v_meas_at2=ADC_readResult(ADC_VOLTAGE_RESULT, ADC_SOC_NUMBER1);
    //SET bit 1
    v_read_at2=1;

    ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER2);// oke, lekezeltuk

    // Check if overflow has occurred

    if(true == ADC_getInterruptOverflowStatus(ADC_VOLTAGE, ADC_INT_NUMBER2))
    {
        v_overflow_at2++;
        //nyilvan ide meg illene valamit tenni, de most csak leokezzuk
        ADC_clearInterruptOverflowStatus(ADC_VOLTAGE, ADC_INT_NUMBER2);
        ADC_clearInterruptStatus(ADC_VOLTAGE, ADC_INT_NUMBER2);
    }

    // Acknowledge the interrupt ??
    Interrupt_clearACKGroup( INT_VOLTAGE_2_INTERRUPT_ACK_GROUP);
}


void setupEpwm1(uint32_t EPWM_TIMER_TBPRD, float32_t duty_cycle,
                uint16_t fed_set, uint16_t red_set)
{
    EPWM_setTimeBasePeriod(myEPWM1_BASE, EPWM_TIMER_TBPRD); //up/down count
    EPWM_setPhaseShift(myEPWM1_BASE, 0U); //?
    EPWM_setTimeBaseCounter(myEPWM1_BASE, 0U); // honnan indul?
    EPWM_setTimeBaseCounterMode(myEPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN); // fel/le szamoljon
    EPWM_disablePhaseShiftLoad(myEPWM1_BASE);
    //
    // Set ePWM clock pre-scaler
    //
    EPWM_setClockPrescaler(myEPWM1_BASE, EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_2); //1*2-gyel osztjuk le a sysclk/2-t
    //50MHz-es TBCLK
    //
    // Set up shadowing
    //
    EPWM_setCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO); //ha 0, akkor toltse bele a shadow registerből az értéket
    //
    // Set-up compare
    //
    EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A,
                                duty_cycle * EPWM_TIMER_TBPRD / 100); //beállíthatunk két comparet, meg kell nezni hogy megy
    //
    // Set actions
    //
    EPWM_setActionQualifierAction(
            myEPWM1_BASE, // mi tortenjen amikor a kulonbozo compare eventek bekovetkeznek
            EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH,
            EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setRisingEdgeDeadBandDelayInput(myEPWM1_BASE, EPWM_DB_INPUT_EPWMA); // ez alapjan lesz delay mostantol
    EPWM_setFallingEdgeDeadBandDelayInput(myEPWM1_BASE, EPWM_DB_INPUT_EPWMA); // same
    EPWM_setFallingEdgeDelayCount(myEPWM1_BASE, fed_set); // eddig szamol el, amig delayeli a váltást
    EPWM_setRisingEdgeDelayCount(myEPWM1_BASE, red_set); // szintén csak felfutóélnéls
    //
    // Ez a 4 sor gyakorlatilag beállítja a megfelelő módot:  HWREGH(myEPWM1_BASE + EPWM_O_DBCTL) =0x000B; // beallitja a modot a megfelelore lsd spuri/1812.o teteje
    //
    EPWM_setDeadBandDelayPolarity(myEPWM1_BASE, EPWM_DB_RED,
                                  EPWM_DB_POLARITY_ACTIVE_HIGH); // kivonja, vagy hozzaadja a szamolashoz a deadbandet (?)
    EPWM_setDeadBandDelayPolarity(myEPWM1_BASE, EPWM_DB_FED,
                                  EPWM_DB_POLARITY_ACTIVE_LOW); //?
    EPWM_setDeadBandOutputSwapMode(myEPWM1_BASE, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(myEPWM1_BASE, EPWM_DB_OUTPUT_B, false);
    EPWM_setDeadBandDelayMode(myEPWM1_BASE, EPWM_DB_RED, true); // gyakorlatilag elmenti amit beallitottunk eddig (?)
    EPWM_setDeadBandDelayMode(myEPWM1_BASE, EPWM_DB_FED, true); //?
}


void setupEpwm2(uint32_t EPWM_TIMER_TBPRD, uint16_t fed_set, uint16_t red_set)
{
    //// EPWM2
    EPWM_setTimeBasePeriod(myEPWM4_BASE, EPWM_TIMER_TBPRD); //up/down count
    EPWM_setPhaseShift(myEPWM4_BASE, 0U); //?
    EPWM_setTimeBaseCounter(myEPWM4_BASE, 0U); // honnan indul?
    EPWM_setTimeBaseCounterMode(myEPWM4_BASE, EPWM_COUNTER_MODE_UP_DOWN); // fel/le szamoljon
    EPWM_disablePhaseShiftLoad(myEPWM4_BASE);
    //
    // Set ePWM clock pre-scaler
    //
    EPWM_setClockPrescaler(myEPWM4_BASE, EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_2); //1*2-gyel osztjuk le a sysclk/2-t
    //50MHz-es TBCLK
    //
    // Set up shadowing
    //
    EPWM_setCounterCompareShadowLoadMode(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO); //ha 0, akkor toltse bele a shadow registerből az értéket
    //
    // Set-up compare
    //
    EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A,
                                EPWM_TIMER_TBPRD); //beállíthatunk két comparet, meg kell nezni hogy megy
    //
    // Set actions
    //
    // mi tortenjen amikor a kulonbozo compare eventek bekovetkeznek
    EPWM_setActionQualifierAction(
            myEPWM4_BASE, // mi tortenjen amikor a kulonbozo compare eventek bekovetkeznek
            EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_HIGH,
            EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B,
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
    EPWM_setDeadBandDelayPolarity(myEPWM4_BASE, EPWM_DB_RED,
                                  EPWM_DB_POLARITY_ACTIVE_HIGH); // kivonja, vagy hozzaadja a szamolashoz a deadbandet (?)
    EPWM_setDeadBandDelayPolarity(myEPWM4_BASE, EPWM_DB_FED,
                                  EPWM_DB_POLARITY_ACTIVE_LOW); //?
    EPWM_setDeadBandOutputSwapMode(myEPWM4_BASE, EPWM_DB_OUTPUT_A, true);
    EPWM_setDeadBandOutputSwapMode(myEPWM4_BASE, EPWM_DB_OUTPUT_B, true);
    EPWM_setDeadBandDelayMode(myEPWM4_BASE, EPWM_DB_RED, true); // gyakorlatilag elmenti amit beallitottunk eddig (?)
    EPWM_setDeadBandDelayMode(myEPWM4_BASE, EPWM_DB_FED, true); //?
}

uint16_t duty_null(){
    return (drive==DRIVE_TYPE_BIPOLAR)?50:0;
}
