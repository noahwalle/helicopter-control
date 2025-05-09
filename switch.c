/**
 * @filename: switch.c
 * @authors: Mark Day, Noah Walle
 * @date: 16.05.2024
 * @purpose: switch Function Definitions
**/

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#include "switch.h"

// Defines for Switch 1 
#define SW1_PERIPH       SYSCTL_PERIPH_GPIOA
#define SW1_PORT_BASE    GPIO_PORTA_BASE
#define SW1_PIN          GPIO_PIN_7
#define SW1_NORMAL       false

// Virtual Reset switch (Switch 2)
#define RESET_PERIPH     SYSCTL_PERIPH_GPIOA
#define RESET_PORT_BASE  GPIO_PORTA_BASE
#define RESET_PIN        GPIO_PIN_6
#define RESET_NORMAL     false

/*
* Enables switches and GPIO
*/ 
void
InitSwitch(void)
{
    SysCtlPeripheralEnable(SW1_PERIPH);
    GPIOPinTypeGPIOInput(SW1_PORT_BASE, SW1_PIN);
    GPIOPadConfigSet(SW1_PORT_BASE, SW1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
}

bool
ResetUp(void)
{
    return (GPIOPinRead(RESET_PORT_BASE, RESET_PIN) == RESET_PIN);
}

/*
* Returns true if switch up 
*/
bool
SwitchUp(void)
{
    return (GPIOPinRead(SW1_PORT_BASE, SW1_PIN) == SW1_PIN);
}
