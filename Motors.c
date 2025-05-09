/**
 * @filename: motors.c
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Function definitions for the motors/PWM
 *
 *******************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#include "motors.h"

// PWM configuration
#define PWM_START_RATE_HZ  200
#define PWM_FIXED_DUTY     0

//  PWM Hardware Details M0PWM7 (gen 3)
//  ---Main Rotor PWM: PC5, J4-05
#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM  SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define MAIN_MOTOR_PIN       GPIO_PIN_5


//  ---Tail Rotor PWM: PF1
#define PWM_TAIL_BASE        PWM1_BASE
#define PWM_TAIL_GEN         PWM_GEN_2
#define PWM_TAIL_OUTNUM      PWM_OUT_5
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM  SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
#define TAIL_MOTOR_PIN       GPIO_PIN_1

#define PWM_DIVIDER 1


int32_t g_setMainDuty;
int32_t g_setTailDuty;

// Initialise PWM M0PWM7 (J4-05, PC5) is used for the main rotor motor
void
InitMotors(void)
{
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);
    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, MAIN_MOTOR_PIN);
    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);
    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, TAIL_MOTOR_PIN);
    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    // Initial PWM parameters
    SetMainPWM(PWM_FIXED_DUTY);
    SetTailPWM(PWM_FIXED_DUTY);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);
    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    // Enable the output
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);
}


// Function to set the freq, duty cycle of M0PWM7
void
SetMainPWM(uint8_t mainDuty)
{
    g_setMainDuty = mainDuty;

    uint32_t ui32Period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, ui32Period * mainDuty / 100);
}

void
SetTailPWM(uint8_t tailDuty)
{
    g_setTailDuty = tailDuty;

    uint32_t ui32Period = SysCtlClockGet() / PWM_DIVIDER / PWM_START_RATE_HZ;

    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, ui32Period);
    PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, ui32Period * tailDuty / 100);
}

uint8_t
GetMainDuty(void)
{
    return g_setMainDuty;
}

uint8_t
GetTailDuty(void)
{
    return g_setTailDuty;
}
