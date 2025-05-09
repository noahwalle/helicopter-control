/**
 * @filename: main.c
 * @authors: Mark Day, Noah Walle
 * @date: 16.05.2024
 * @purpose: Main for helicopter project,
 * adds tasks to scheduler and calls it
**/

#include "buttons4.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

// Helper modules
#include "altitude.h"
#include "display.h"
#include "yaw.h"
#include "motors.h"
#include "switch.h"
#include "kernel.h"
#include "serial.h"

// Macros for the kernel
#define KERNEL_RATE_HZ 2000
#define OFF 0
#define ON 1

/*
 * Priorities and ticks for each task.
 *
 * You can calculate the equivalent frequency of a given task by:
 * TASK_FREQUENCY = KERNEL_RATE_HZ / TASK_TICKS
 */
#define ADC_PRIORITY 0
#define ADC_TICKS 10

#define SETPOINT_PRIORITY 1
#define SETPOINT_TICKS 75

#define SWITCH_PRIORITY 6
#define SWITCH_TICKS 200

#define CONTROL_PRIORITY 3
#define CONTROL_TICKS 45

#define DISPLAY_PRIORITY 4
#define DISPLAY_TICKS 100

#define GND_PRIORITY 2
#define GND_TICKS 3000

#define UART_PRIORITY 5
#define UART_TICKS 500

#define RESET_PRIORITY 7
#define RESET_TICKS 500

/*
 * Main initialiser function
 * Must be called first
 */
void
MainInit(void)
{
    InitKernel(KERNEL_RATE_HZ);
    InitADC();
    initButtons();
    InitDisplay();
    InitMotors();
    InitSwitch();
    InitUart();
    InitQuad();
    IntMasterEnable();
}

void
ADCTask(void)
{
    ADCProcessTrigger();
}

void
ControlTask(void)
{
    int32_t altitude_effort = AltController();
    int32_t yaw_effort = YawController();
    SetMainPWM(altitude_effort);
    SetTailPWM(yaw_effort);
}

/*
 * Button checks for setpoint values
 */
void
SetPointTask(void)
{
    updateButtons();
    CheckAltitudeSetButton();
    CheckYawSetButton();
}

/*
 * FSM for the heli flight mode
 */
void
SwitchLogicTask(void)
{
    if(SwitchUp() && Hover() && Stable()) {
        TaskEnable(&ControlTask);
        TaskEnable(&SetPointTask);

    } else if (!SwitchUp() && !AltitudeLand() && !YawLand()) {
        TaskEnable(&ControlTask);
        TaskDisable(&SetPointTask);

    } else if (!SwitchUp() && AltitudeLand() && YawLand()) {
        TaskDisable(&ControlTask);
        TaskDisable(&SetPointTask);
    } else {
        TaskDisable(&ControlTask);
    }
}

/*
 * Sets the reference once before disabling itself
 */
void
GroundRefTask(void)
{
    SetAltitudeRef();
    TaskDisable(&GroundRefTask);
    TaskEnable(&SwitchLogicTask);
}

void
DisplayTask(void)
{
    UpdateDisplay();
}

void
UARTTask(void)
{
    SendValues();
}

/*
 * Virtual switch reset
 */
void
ResetTask(void)
{
    if (ResetUp())
    {
        SysCtlReset();
    }
}

int
main(void)
{
    MainInit();

    AddTask(&ADCTask,         ADC_TICKS,      ADC_PRIORITY,      ON);
    AddTask(&SetPointTask,    SETPOINT_TICKS, SETPOINT_PRIORITY, OFF);
    AddTask(&SwitchLogicTask, SWITCH_TICKS,   SWITCH_PRIORITY,   OFF);
    AddTask(&ControlTask,     CONTROL_TICKS,  CONTROL_PRIORITY,  OFF);
    AddTask(&DisplayTask,     DISPLAY_TICKS,  DISPLAY_PRIORITY,  ON);
    AddTask(&GroundRefTask,   GND_TICKS,      GND_PRIORITY,      ON);
    AddTask(&UARTTask,        UART_TICKS,     UART_PRIORITY,     ON);
    AddTask(&ResetTask,       RESET_TICKS,    RESET_PRIORITY,    ON);

    while(1)
    {
        RunKernel();
    }
}
