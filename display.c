/**
 * @filename: display.c
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Function definitions for display logic
**/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"

#include "display.h"
#include "yaw.h"
#include "altitude.h"
#include "motors.h"

uint8_t tick_count;

void
DisplayBlank(void)
{
    OLEDStringDraw("                 ", 0, 0);
    OLEDStringDraw("                 ", 0, 1);
    OLEDStringDraw("                 ", 0, 2);
    OLEDStringDraw("                 ", 0, 3);
}

void
InitDisplay(void)
{
    OLEDInitialise();
    DisplayBlank();
}

void
UpdateDisplay(void)
{
    int32_t yaw = GetYaw();

    char altitudeString[17];  // 16 characters across the display
    char yawString[17];
    char mainDutyString[17];
    char tailDutyString[17];

    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf(altitudeString, sizeof(altitudeString), "A:%4d%%  S: %3d", GetAltPercent(), GetAltitudeSetpoint());
    usnprintf(yawString, sizeof(yawString), "Y:%4d.%1d S:%4d", yaw/10, abs(yaw%10), GetYawSetpoint());
    usnprintf(mainDutyString, sizeof(mainDutyString), "MAIN: %3d", GetMainDuty());
    usnprintf(tailDutyString, sizeof(tailDutyString), "TAIL: %3d", GetTailDuty());
    // Update line on display.
    OLEDStringDraw(altitudeString, 0, 0);
    OLEDStringDraw(yawString, 0, 1);
    OLEDStringDraw(mainDutyString, 0, 2);
    OLEDStringDraw(tailDutyString, 0, 3);
}
