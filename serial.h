#ifndef SERIAL_H
#define SERIAL_H

/**
 * @filename: serial.h
 * @authors: Mark Day, Noah Walle
 * @date: 16.05.2024
 * @purpose: serial Module Header
**/

#include <stdint.h>

void
InitUart(void);

void
UartSend(const char *t_buffer);

void
SendValues(void);

#endif
