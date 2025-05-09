/**
 * @filename: serial.c
 * @authors: Mark Day, Noah Walle
 * @date: 16.05.2024
 * @purpose: serial Function Definitions:
 *           Is used to send flight data across uart
**/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"

#include "serial.h"
#include "yaw.h"
#include "altitude.h"
#include "motors.h"
#include "switch.h"

// Define Rx, Tx pins 
#define RX_PIN GPIO_PIN_0
#define TX_PIN GPIO_PIN_1

// Define Buffer Size and speed to be sent
#define UART_BUFFER_SIZE 40
#define BAUD_RATE 9600

// Global pointer to Buffer
static char *g_buffer;

/*
 * Enables Uart0 and Rx Tx pins
 */
void
InitUart(void)
{
    g_buffer = calloc(UART_BUFFER_SIZE, sizeof(char));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeUART(GPIO_PORTA_BASE, RX_PIN | TX_PIN);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTFIFOEnable(UART0_BASE);
    UARTEnable(UART0_BASE);
}

/*
 * (Original Code by P.J. Bones)
 * Formats the string to send via UART and then sends it
 */
void UartSend(const char *t_buffer)
{
    while (*t_buffer)
    {
        UARTCharPut(UART0_BASE, *t_buffer);
        t_buffer++;
    }
}

/*
 * Gets values and adds them to string g_buffer
 * Calls uart_send to send string
 */
void SendValues(void)
{
    int32_t altitude = GetAltPercent();
    int32_t altitude_setpoint = GetAltitudeSetpoint();
    int32_t yaw = GetYaw();
    int16_t yaw_setpoint = GetYawSetpoint();
    uint8_t main_duty = GetMainDuty();
    uint8_t tail_duty = GetTailDuty();
    int flight_mode = SwitchUp();

    usprintf(g_buffer,
            "a%d\tA%d\ty%d\tY%d\tMD%d\tTD%d\tOM%d\r\n", 
            altitude, 
            altitude_setpoint, 
            yaw/10, 
            yaw_setpoint, 
            main_duty, 
            tail_duty, 
            flight_mode);

    UartSend(g_buffer);
}
