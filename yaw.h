#ifndef YAW_H
#define YAW_H

/**
 * @filename: yaw.h
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Yaw Module Header
**/

// Module requirements
#include <stdint.h>
#include <stdbool.h>

void
QuadHandler(void);

void
RefHandler(void);

void
InitQuad(void);

int16_t
GetYaw(void);

int16_t 
YawController(void);

void
CheckYawSetButton(void);

int16_t
GetYawSetpoint(void);

bool
Stable(void);

uint8_t
YawLand(void);

#endif
