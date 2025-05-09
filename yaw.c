/**
 * @filename: yaw.c
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Function definitions for the yaw module
**/
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "buttons4.h"

#include "motors.h"
#include "yaw.h"

// Define encoder values and convertsion to degrees 
#define STEP_MAX 448
#define YAW_SCALE 360 * 10 / STEP_MAX

// Max/Min values for Motors
#define MAX_YAW_OUTPUT 70
#define MIN_YAW_OUTPUT 2

// Encoder Pins
#define QUAD_CHANNEL_A GPIO_PIN_0
#define QUAD_CHANNEL_B GPIO_PIN_1
#define REF_CHANNEL    GPIO_PIN_4

// Encoder Pin states
static uint8_t g_previousState;
static uint8_t g_previousState;
static int16_t g_yaw;

// Global for yawController 
static int32_t g_yawOffset = 40;
static int32_t g_yawControlEffort;
static int32_t g_yawError;
float g_yawDeltaT = 0.0005;

// flag set at first zero degree interupt
bool g_refFlag = 0;

// Struct for control parameters 
static PID_t g_yawControl = {.setpoint = 0,
                             .prev_setpoint = 0,
                             .read_value = 0,
                             .prev_read_value = 0,
                             .Kp = 0.89,
                             .Ki = 0.02,
                             .Kd = 0};

/*
 * Interrupt Handler for the encoder Pins iterates yaw based on encoder state
 */
void
QuadHandler(void)
{
    g_previousState = g_previousState;
    g_previousState = GPIOPinRead(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B);
    if (g_previousState == 0b00) {
        if (g_previousState == 0b01) {
            g_yaw++;
        } else if (g_previousState == 0b10) {
            g_yaw--;
        }
    } else if (g_previousState == 0b01) {
        if (g_previousState == 0b11) {
            g_yaw++;
        } else if (g_previousState == 0b00) {
            g_yaw--;
        }
    } else if (g_previousState == 0b10) {
        if (g_previousState == 0b11) {
            g_yaw--;
        } else if (g_previousState == 0b00) {
            g_yaw++;
        }
    } else if (g_previousState == 0b11) {
        if (g_previousState == 0b01) {
            g_yaw--;
        } else if (g_previousState == 0b10) {
            g_yaw++;
        }
    }
    // Zeros yaw if a full cycle is completed
    // Has redundancy with reference interrupt
    if (g_yaw >= STEP_MAX || g_yaw <= -STEP_MAX)
    {
        g_yaw = 0;
    }

    GPIOIntClear(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B);
}

/*
 * Interrupt Handler for Yaw Reference Pin
 * Sets yaw to 0.
 */
void
RefHandler(void)
{
    g_yaw = 0;
    g_refFlag = 1;

    GPIOIntClear(GPIO_PORTC_BASE, REF_CHANNEL);
    GPIOIntDisable(GPIO_PORTC_BASE, REF_CHANNEL);
}

/*
 * Initialises Both Interrupts (RefHandler, QuadHandler)
 */
void
InitQuad(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B);

    GPIOIntDisable(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B);
    GPIOIntClear(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B);

    GPIOIntRegister(GPIO_PORTB_BASE, QuadHandler);
    GPIOIntTypeSet(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B, GPIO_BOTH_EDGES);

    GPIOIntEnable(GPIO_PORTB_BASE, QUAD_CHANNEL_A | QUAD_CHANNEL_B);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, REF_CHANNEL);

    GPIOIntDisable(GPIO_PORTC_BASE, REF_CHANNEL);
    GPIOIntClear(GPIO_PORTC_BASE, REF_CHANNEL);

    GPIOIntRegister(GPIO_PORTC_BASE, RefHandler);
    GPIOIntTypeSet(GPIO_PORTC_BASE, REF_CHANNEL, GPIO_BOTH_EDGES);

    GPIOIntEnable(GPIO_PORTC_BASE, REF_CHANNEL);
}

/*
 * Returns Value of Yaw in degrees
 * Updates the read_value in g_yawControl
 * Updates the prev_read_value in g_yawControl
 */
int16_t
GetYaw(void)
{
    int32_t new_yaw;
    g_yawControl.prev_read_value = g_yawControl.read_value;

    if (abs(g_yaw) <= (STEP_MAX / 2))
    {
        g_yawControl.read_value = g_yaw * YAW_SCALE / 10;
        return g_yaw * YAW_SCALE;
    } else {
        if (g_yaw > 0) {
            new_yaw = g_yaw - 448;
        } else {
            new_yaw = g_yaw + 448;
        }
        g_yawControl.read_value = g_yaw * YAW_SCALE / 10;
        return new_yaw * YAW_SCALE;
    }
}

/*
 * (Inspired by Ciaran Moore Lecture notes)
 * PI Controller for Yaw
 */
int16_t 
YawController(void) 
{
    static float iSum = 0;

    // Resets I_sum for new setpoint
    if (g_yawControl.setpoint != g_yawControl.prev_setpoint) {
        iSum= 0;
        g_yawControl.prev_setpoint = g_yawControl.setpoint;
    }

    // Logic for choosing the shortest distance to setpoint 
    g_yawError = g_yawControl.read_value - g_yawControl.setpoint;
    if (abs(g_yawError) > 180) {
        g_yawError = g_yawControl.read_value + g_yawControl.setpoint;
    }

    float pControl = g_yawControl.Kp * g_yawError;
    float iControl = g_yawControl.Ki * g_yawError * g_yawDeltaT; 
    g_yawControlEffort = pControl + iSum + iControl + g_yawOffset;

    iSum = (iSum + iControl);

    // limits g_yawControlEffort to values that the motor is able to take
    if (g_yawControlEffort > MAX_YAW_OUTPUT) {
        g_yawControlEffort = MAX_YAW_OUTPUT;
    } else if(g_yawControlEffort < MIN_YAW_OUTPUT) {
        g_yawControlEffort = MIN_YAW_OUTPUT;
    }

    return g_yawControlEffort;
}

/*
 * Checks buttons and updates setpoint in g_yawControl
 */
void
CheckYawSetButton(void)
{
    uint8_t butState = checkButton(RIGHT);
    if (butState == PUSHED) {
        g_yawControl.prev_setpoint = g_yawControl.setpoint;
        g_yawControl.setpoint += 15;
    }
    butState = checkButton(LEFT);
    if (butState == PUSHED) {
        g_yawControl.prev_setpoint = g_yawControl.setpoint;
        g_yawControl.setpoint -= 15;
    }

    // sets the "wrap-around"
    if (g_yawControl.setpoint > 180) {
        g_yawControl.setpoint = g_yawControl.setpoint - 360;
    } else if (g_yawControl.setpoint <= -180) {
        g_yawControl.setpoint += 360;
    }
}


int16_t
GetYawSetpoint(void)
{
    return g_yawControl.setpoint;
}

/*
* Makes heli turn at constant rate (g_yawOffset)
* Until it reaches the Yaw Reference
*/
bool
Stable(void)
{
    if (g_yaw != 0) {
        SetTailPWM(g_yawOffset);
    }
    return g_refFlag;
}

/*
* Makes heli turn to zero degrees 
* Turns motor off once landed
*/
uint8_t
YawLand(void)
{
    if (GetYaw() != 0){
        g_yawControl.setpoint = 0;
        return 0;
    } else {
        SetTailPWM(0);
        return 1;
    }
}

