#ifndef KERNEL_H
#define KERNEL_H

/**
 * @filename: kernel.h
 * @authors: Mark Day, Noah Walle
 * @date: 13.05.2024
 * @purpose: Kernel header
**/

#include <stdint.h>

typedef struct {
    // function name
    void* FunctionPtr;

    // Number of ticks before the function gets called again
    uint16_t NumTicks;

    // The order tasks run, 0 first. 
    uint8_t Priority;

    // 0 or 1, if task should run
    uint8_t RunTask;

    // last time task ran
    uint32_t LastRun;
} Task_t ;


void
SysTickIntHandler(void);

void
InitClock(uint32_t KERNEL_RATE_HZ);

void
InitKernel(uint32_t KERNEL_RATE_HZ);

void
AddTask(void* functionPtr, uint16_t numTicks, uint8_t priority, uint8_t runTask);

void
TaskEnable(void* functionPtr);

void
TaskDisable(void* functionPtr);

void
RunKernel(void);

#endif
