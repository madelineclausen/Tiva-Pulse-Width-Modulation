#include "tm4c123gh6pm.h"
#include <stdint.h>

#define ADC_TIMEOUT 1000000  // Timeout value

// Function prototypes
void ADC0_PWM_Init(uint32_t period);
uint32_t ADC0_Read(void);
void PWM0_SetDutyCycle(float dutyCycle, uint32_t period);
void DelayMs(uint32_t ms);

int main(void) {
    uint32_t adcValue, dutyCycle;
    uint32_t period = 16000;

    // Initialize ADC and PWM
    ADC0_PWM_Init(period); // Example period for 1 kHz

    while (1) {
        adcValue = ADC0_Read(); // Read the ADC value from the potentiometer

        dutyCycle = (adcValue * 100) / 2047; // Map the ADC value to a duty cycle range (0-100%)

        if (dutyCycle < 0) dutyCycle = 0;
        if (dutyCycle > 100) dutyCycle = 100;

        PWM0_SetDutyCycle(dutyCycle, period); // Update PWM duty cycle

        DelayMs(100); // Delay
    }
}

void ADC0_PWM_Init(uint32_t period) {
    SYSCTL_RCGCGPIO_R |= (1U << 4); // Enable GPIO Port F and Port E clock
    SYSCTL_RCGCADC_R |= (1U << 0);  // Enable ADC0 clock
    while ((SYSCTL_PRADC_R & (1U << 0)) == 0);   // Wait for ADC0 to be ready

    // Configure PE3 as an analog input (for potentiometer)
    GPIO_PORTE_AFSEL_R |= (1U << 3);  // Enable alternate function on PE3
    GPIO_PORTE_DEN_R &= ~(1U << 3);   // Disable digital function on PE3
    GPIO_PORTE_AMSEL_R |= (1U << 3);  // Enable analog function on PE3

    // Configure ADC0 sequencer 3
    ADC0_PC_R = 0x1;                  // Set ADC0 sample rate to 1.5 MSPS (maximum rate)
    ADC0_ACTSS_R &= ~(1U << 3);       // Disable SS3 (before configuring)
    ADC0_EMUX_R &= ~0xF000; //~0x00;           // Select processor trigger for SS3
    ADC0_SSMUX3_R = 3; //0;                // Select channel 3 (PE3) for SS3
    ADC0_SSCTL3_R = 0x06;             // Enable IE0 (Interrupt Enable) and END0 (End of Sequence) flags
    ADC0_ACTSS_R |= (1U << 3);        // Enable SS3



    SYSCTL_RCGCPWM_R |= 2;       /* Enable clock to PWM1 module */
    SYSCTL_RCGCGPIO_R |= 0x20;   /* Enable system clock to PORTF */
    SYSCTL_RCC_R &= ~0x00100000; /* Directly feed clock to PWM1 module without pre-divider */

    /* Setting of PF2 pin for M1PWM6 channel output pin */
    GPIO_PORTF_AFSEL_R |= (1<<2);     /* PF2 sets a alternate function */
    GPIO_PORTF_PCTL_R &= ~0x00000F00; /*set PF2 as output pin */
    GPIO_PORTF_PCTL_R |= 0x00000500; /* make PF2 PWM output pin */
    GPIO_PORTF_DEN_R |= (1<<2);      /* set PF2 as a digital pin */

    /* PWM1 channel 6 setting */
    PWM1_3_CTL_R &= ~(1<<0);   /* Disable Generator 3 counter */
    PWM1_3_CTL_R &= ~(1<<1);   /* select down count mode of counter 3*/
    PWM1_3_GENA_R = 0x0000008C; /* Set PWM output when counter reloaded and clear when matches PWMCMPA */
    PWM1_3_LOAD_R = period;   /* set load value for 1kHz (16MHz/16000) */
    PWM1_3_CMPA_R = period/2 - 1;  /* set duty cyle to 50% by loading of 16000 to PWM1CMPA */
    PWM1_3_CTL_R = 1;         /* Enable Generator 3 counter */
    PWM1_ENABLE_R = 0x40;      /* Enable PWM1 channel 6 output */

}

uint32_t ADC0_Read(void) {
    ADC0_PSSI_R |= (1U << 3); // Start SS3 conversion

    uint32_t timeout_counter = 0;
    while ((ADC0_RIS_R & (1U << 3)) == 0) {
        timeout_counter++;
        if (timeout_counter > ADC_TIMEOUT) {
            ADC0_ISC_R = (1U << 3); // Clear the completion flag in case of timeout
            return 0xFFFFFFFF;    // Return a special value to indicate timeout
        }
    }

    uint32_t result = ADC0_SSFIFO3_R & 0xFFF; // Read 12-bit result
    ADC0_ISC_R = (1U << 3);                   // Clear completion flag
    return result;
}

void PWM0_SetDutyCycle(float dutyCycle, uint32_t period) {
    // Calculate CMPA with proper type casting
   float cmpValue = (1.0f - (dutyCycle / 100.0f)) * period;

   // Ensure CMPA does not go negative
   if (cmpValue < 1) {
       PWM1_3_CMPA_R = 0;
   } else {
       PWM1_3_CMPA_R = (uint32_t)(cmpValue - 1);
   }
}

void DelayMs(uint32_t ms) {
    volatile uint32_t count;
    for (count = 0; count < (ms * 4000); count++);
}





//#include "tm4c123gh6pm.h"
//#include <stdint.h>
//int main(void)
//{
//
//void Delay_ms(int n);
///* Clock setting for PWM and GPIO PORT */
//SYSCTL_RCGCPWM_R |= 2;       /* Enable clock to PWM1 module */
//SYSCTL_RCGCGPIO_R |= 0x20;   /* Enable system clock to PORTF */
//SYSCTL_RCC_R &= ~0x00100000; /* Directly feed clock to PWM1 module without pre-divider */
//
///* Setting of PF2 pin for M1PWM6 channel output pin */
//GPIO_PORTF_AFSEL_R |= (1<<2);     /* PF2 sets a alternate function */
//GPIO_PORTF_PCTL_R &= ~0x00000F00; /*set PF2 as output pin */
//GPIO_PORTF_PCTL_R |= 0x00000500; /* make PF2 PWM output pin */
//GPIO_PORTF_DEN_R |= (1<<2);      /* set PF2 as a digital pin */
//
///* PWM1 channel 6 setting */
//PWM1_3_CTL_R &= ~(1<<0);   /* Disable Generator 3 counter */
//PWM1_3_CTL_R &= ~(1<<1);   /* select down count mode of counter 3*/
//PWM1_3_GENA_R = 0x0000008C; /* Set PWM output when counter reloaded and clear when matches PWMCMPA */
//PWM1_3_LOAD_R = 16000;   /* set load value for 1kHz (16MHz/16000) */
//PWM1_3_CMPA_R = 1000;  /* set duty cyle to 50% by loading of 16000 to PWM1CMPA */
//PWM1_3_CTL_R = 1;         /* Enable Generator 3 counter */
//PWM1_ENABLE_R = 0x40;      /* Enable PWM1 channel 6 output */
//
//    while(1)
//    {
//        Delay_ms(2);
//        PWM1_3_LOAD_R = PWM1_3_LOAD_R - 1;
//        if (PWM1_3_LOAD_R == 0)
//        {
//            PWM1_3_LOAD_R = 16000;
//        }
//            //do nothing
//    }
//}
//
//
///* This function generates delay in ms */
///* calculations are based on 16MHz system clock frequency */
//
//void Delay_ms(int time_ms)
//{
//    int i, j;
//    for(i = 0 ; i < time_ms; i++)
//        for(j = 0; j < 3180; j++)
//            {}  /* excute NOP for 1ms */
//}








//#include <stdint.h>
//#include "tm4c123gh6pm.h" // Include the board header file
//
//#define ADC_TIMEOUT 1000000  // Timeout value
//
//// Function prototypes
//void ADC0_PWM_Init(uint32_t period);
//uint32_t ADC0_Read(void);
////void PWM0_SetDutyCycle(uint32_t dutyCycle);
//void PWM0_SetDutyCycle(float dutyCycle, uint32_t period);
//void DelayMs(uint32_t ms);
//uint32_t calculateDutyCycle(uint32_t adcValue);
//
//int main(void) {
//    uint32_t adcValue, dutyCycle;
//    uint32_t period = 1000;
//
//    // Initialize ADC and PWM
//    ADC0_PWM_Init(period); // Example period for 1 kHz
//
//    while (1) {
//        GPIO_PORTF_DATA_R |= (1U << 2);  // Turn on PF2
//        DelayMs(100);
//        GPIO_PORTF_DATA_R &= ~(1U << 2); // Turn off PF2
//        DelayMs(100);
//
//        adcValue = ADC0_Read(); // Read the ADC value from the potentiometer
//
//        dutyCycle = (adcValue * 100) / 2047; // Map the ADC value to a duty cycle range (0-100%)
//        //dutyCycle = (adcValue * 100) / 4095; // CAPS AT 53
//        //dutyCycle = calculateDutyCycle(adcValue); // UNSTABLE
//
//        if (dutyCycle < 0) dutyCycle = 0;
//        if (dutyCycle > 100) dutyCycle = 100;
//
//        PWM0_SetDutyCycle(dutyCycle, period); // Update PWM duty cycle
//
//        DelayMs(100); // Delay
//    }
//}
//
//void ADC0_PWM_Init(uint32_t period) {
//    SYSCTL_RCGCGPIO_R |= (1U << 5) | (1U << 4); // Enable GPIO Port F and Port E clock
//    SYSCTL_RCGCPWM_R |= (1U << 0);  // Enable PWM0 clock
//    SYSCTL_RCGCADC_R |= (1U << 0);  // Enable ADC0 clock
//    SYSCTL_RCC_R &= ~0x00100000;
//    while ((SYSCTL_PRADC_R & (1U << 0)) == 0);   // Wait for ADC0 to be ready
//
//    // Configure PE3 as an analog input (for potentiometer)
//    GPIO_PORTE_AFSEL_R |= (1U << 3);  // Enable alternate function on PE3
//    GPIO_PORTE_DEN_R &= ~(1U << 3);   // Disable digital function on PE3
//    GPIO_PORTE_AMSEL_R |= (1U << 3);  // Enable analog function on PE3
//
//    // Configure ADC0 sequencer 3
//    ADC0_PC_R = 0x1;                  // Set ADC0 sample rate to 1.5 MSPS (maximum rate)
//    ADC0_ACTSS_R &= ~(1U << 3);       // Disable SS3 (before configuring)
//    ADC0_EMUX_R &= ~0xF000; //~0x00;           // Select processor trigger for SS3
//    ADC0_SSMUX3_R = 3; //0;                // Select channel 3 (PE3) for SS3
//    ADC0_SSCTL3_R = 0x06;             // Enable IE0 (Interrupt Enable) and END0 (End of Sequence) flags
//    ADC0_ACTSS_R |= (1U << 3);        // Enable SS3
//
////    // Configure PB1 for PWM output
//    GPIO_PORTF_AFSEL_R |= (1U << 2);  // Enable alternate function for PF2
//    GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R & ~(0xF << 4)) | (0x4 << 4); // Set PF2 to PWM
//    GPIO_PORTF_DEN_R |= (1U << 2);    // Enable digital functionality for PF2
//    GPIO_PORTF_DIR_R |= (1U << 2);    // Set PF2 as output
//
////    // PWM signal generation
//    PWM0_0_CTL_R = 0;                 // Disable PWM0 while configuring
//    PWM0_0_LOAD_R = period - 1;       // Set PWM period
//    PWM0_0_CMPA_R = (period / 2) - 1; // Set 50% duty cycle initially
//    PWM0_0_GENA_R = 0x0000080C;       // Configure PWM0 to go low on down-count
//    PWM0_0_CTL_R |= 1;                // Enable PWM0
//    PWM0_ENABLE_R |= (1U << 1);       // Enable PWM0 output
//}
//
//uint32_t ADC0_Read(void) {
//    ADC0_PSSI_R |= (1U << 3); // Start SS3 conversion
//
//    uint32_t timeout_counter = 0;
//    while ((ADC0_RIS_R & (1U << 3)) == 0) {
//        timeout_counter++;
//        if (timeout_counter > ADC_TIMEOUT) {
//            ADC0_ISC_R = (1U << 3); // Clear the completion flag in case of timeout
//            return 0xFFFFFFFF;    // Return a special value to indicate timeout
//        }
//    }
//
//    uint32_t result = ADC0_SSFIFO3_R & 0xFFF; // Read 12-bit result
//    ADC0_ISC_R = (1U << 3);                   // Clear completion flag
//    return result;
//}
//
////void PWM0_SetDutyCycle(uint32_t dutyCycle) {
////    uint32_t loadValue = PWM0_0_LOAD_R + 1;
////    PWM0_0_CMPA_R = ((loadValue * (100 - dutyCycle)) / 100) - 1; // Update CMPA based on duty cycle
////    int test = ((loadValue * (100 - dutyCycle)) / 100) - 1;
////}
//
//void PWM0_SetDutyCycle(float dutyCycle, uint32_t period) {
//
////    // Calculate CMPA based on duty cycle
////    PWM0_0_CMPA_R = (uint32_t)((1.0 - (dutyCycle / 100.0)) * period) - 1;
////    int test = (uint32_t)((1.0 - (dutyCycle / 100.0)) * period) - 1;
////
////    // Check for valid CMPA values
////    if (PWM0_0_CMPA_R >= period) {
////        PWM0_0_CMPA_R = period - 1; // Ensure CMPA is less than LOAD
////    }
//    // Calculate CMPA with proper type casting
//   float cmpValue = (1.0f - (dutyCycle / 100.0f)) * period;
//
//   // Ensure CMPA does not go negative
//   if (cmpValue < 1) {
//       PWM0_0_CMPA_R = 0;
//   } else {
//       PWM0_0_CMPA_R = (uint32_t)(cmpValue - 1);
//   }
//}
//
//void DelayMs(uint32_t ms) {
//    volatile uint32_t count;
//    for (count = 0; count < (ms * 4000); count++);
//}
//
//uint32_t calculateDutyCycle(uint32_t adcValue) {
//    static uint32_t adcMin = 4095, adcMax = 0;
//
//    if (adcValue < adcMin) {
//        adcMin = adcValue;  // Update minimum value
//    }
//    if (adcValue > adcMax) {
//        adcMax = adcValue;  // Update maximum value
//    }
//
//    if (adcMax > adcMin) {
//        return ((adcValue - adcMin) * 100) / (adcMax - adcMin);
//    }
//    return 0;
//}
