#ifndef MOTORS_H
#define MOTORS_H

/**
 * @filename: motors.h
 * @authors: Mark Day, Noah Walle
 * @date: 07.05.2024
 * @purpose: Header file for the motors and PWM code
**/

#include <stdint.h>

/*
 * Control struct
 */
typedef struct {
    int32_t setpoint;
    int32_t prev_setpoint;
    int32_t read_value;
    int32_t prev_read_value;
    float Kp;
    float Ki;
    int32_t Kd;
} PID_t;

void
InitMotors(void);

void
SetMainPWM(uint8_t mainDuty);

void
SetTailPWM(uint8_t tailDuty);

uint8_t
GetMainDuty(void);

uint8_t
GetTailDuty(void);

#endif // MOTORS_H
