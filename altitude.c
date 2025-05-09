/**
 * @filename: altitude.c
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Function definitions for altitude logic
**/

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "buttons4.h"

#include "motors.h"
#include "altitude.h"

// Initialise variables
#define BUF_SIZE 24
#define HELI_ALT_SIGNAL ADC_CTL_CH9
#define MIN_ALT_OUTPUT 2
#define MAX_ALT_OUTPUT 70
#define SCALE_FACTOR_HELI 1241

// Hovering globals
static int8_t g_hoveringOffset;
static uint8_t g_hoverSpeed = 0;
static uint8_t g_hoverFlag = 1;

// Globals for the altitude reference
static int32_t g_gndRef = 0;
static uint8_t g_gndFlag = 1;

// Buffer for signal averaging
static circBuf_t g_inBuffer;

// Control globals
static int32_t g_altError;
static int32_t g_altControlEffort;
static PID_t g_altitudeControl = {.setpoint = 0,
                                  .prev_setpoint = 0,
                                  .read_value = 0,
                                  .prev_read_value = 0,
                                  .Kp = 3,
                                  .Ki = 1,
                                  .Kd = 0};

/*
 * (Original code by P.J. Bones)
 * Calls the ADC processor to read a voltage
 */
void
ADCProcessTrigger(void)
{
    // Initiate a conversion
    ADCProcessorTrigger(ADC0_BASE, 3);
}

/*
 * (Original code by P.J. Bones)
 * Calls the ADC processor to read a voltage
 */
void
ADCIntHandler(void)
{
    uint32_t ulValue;

    // Get the single sample from ADC0.  ADC_BASE is defined in
    // inc/hw_memmap.h
    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);

    // Place it in the circular buffer (advancing write index)
    writeCircBuf(&g_inBuffer, ulValue);

    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}


/*
 * (Original code by P.J. Bones)
 * Initialises the ADC peripheral to sample altitude
 */
void
InitADC(void)
{
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Since sample sequence 3 is now configured, it must be enabled
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, HELI_ALT_SIGNAL | ADC_CTL_IE | ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
    initCircBuf(&g_inBuffer, BUF_SIZE);
}

/*
 * (Original code by P.J. Bones)
 * Calculates the mean voltage by averaging
 * the contents of the circular buffer
 */
int32_t
GetAltMean(void)
{
    uint16_t i;
    int32_t sum = 0;
    for (i = 0; i < BUF_SIZE; i++) {
        sum = sum + readCircBuf (&g_inBuffer);
    }
    return ((2 * sum + BUF_SIZE) / 2 / BUF_SIZE);
}

/*
 * Converts the voltage into a percentage altitude
 */
int32_t
GetAltPercent(void)
{
    int32_t mean = GetAltMean();
    int32_t alt_percent = 100 * (g_gndRef - mean) / SCALE_FACTOR_HELI; // scales into a percentage
    g_altitudeControl.read_value = alt_percent;
    return alt_percent;
}

/*
 * Sets the initial altitude reading as a ground reference
 */
void
SetAltitudeRef(void)
{
    if (g_gndFlag) {
        g_gndRef = GetAltMean();
        g_gndFlag = 0;
    }
}

/*
 * (Code inspiration from Ciaran Moore Lecture Notes)
 * PI controller for altitude
 */
int32_t 
AltController(void)
{
    static float iSumAltitude = 0;
    float alt_delta_t = 0.01;

    if (g_altitudeControl.setpoint != g_altitudeControl.prev_setpoint) {
        iSumAltitude = 0;
        g_altitudeControl.prev_setpoint = g_altitudeControl.setpoint;
    }

    g_altError = g_altitudeControl.setpoint - g_altitudeControl.read_value;

    float pControl = g_altitudeControl.Kp * g_altError;
    float iControl = g_altitudeControl.Ki * g_altError * alt_delta_t;

    g_altControlEffort = pControl + iSumAltitude + iControl + g_hoveringOffset;

    iSumAltitude += iControl;

    if (g_altControlEffort > MAX_ALT_OUTPUT) {
        g_altControlEffort = MAX_ALT_OUTPUT;
    } else if (g_altControlEffort < MIN_ALT_OUTPUT) {
        g_altControlEffort = MIN_ALT_OUTPUT;
    }

    return g_altControlEffort;
}

/*
 * Checks the up/down buttons, alters
 * setpoint accordingly
 */
void
CheckAltitudeSetButton(void)
{
    uint8_t butState = checkButton(UP);
    if (butState == PUSHED) {
        g_altitudeControl.prev_setpoint = g_altitudeControl.setpoint;
        g_altitudeControl.setpoint += 10;
    }
    butState = checkButton(DOWN);
    if (butState == PUSHED) {
        g_altitudeControl.prev_setpoint = g_altitudeControl.setpoint;
        g_altitudeControl.setpoint -= 10;
    }
     if (g_altitudeControl.setpoint < 0) {
         g_altitudeControl.setpoint = 0;
    }
    if (g_altitudeControl.setpoint > 100) {
        g_altitudeControl.setpoint = 100;
    }
}

int32_t
GetAltitudeSetpoint(void)
{
    return g_altitudeControl.setpoint;
}

/*
 * Ramps up the main rotor
 * until hovering
 */
uint8_t
Hover(void)
{
    if ((GetAltPercent() <= 1) && g_hoverFlag){
        SetMainPWM(g_hoverSpeed);
        g_hoverSpeed++;
        return 0;
    } else {
        g_hoveringOffset = g_hoverSpeed - 5;
        g_hoverFlag = 0;
        return 1;
    }
}

/*
 * Ramps down the main rotor
 * until on the ground
 */
uint8_t
AltitudeLand(void)
{
    if (GetAltPercent() > 0){
        g_altitudeControl.setpoint = 0;
        return 0;
    } else {
        SetMainPWM(0);
        return 1;
    }
}
