#ifndef ALT_H
#define ALT_H

/**
 * @filename: altitude.h
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Altitude Module Header
**/

// Module requirements
#include <stdint.h>
#include <stdbool.h>
#include "circBufT.h"

void
ADCProcessTrigger(void);

void
ADCIntHandler(void);

void
InitADC(void);

int32_t
GetAltMean(void);

int32_t
GetAltPercent(void);

void
SetAltitudeRef(void);

int32_t
AltController(void);

void
CheckAltitudeSetButton(void);

int32_t
GetAltitudeSetpoint(void);

uint8_t
Hover(void);

uint8_t
AltitudeLand(void);

#endif
