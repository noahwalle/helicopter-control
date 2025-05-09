/**
 * @filename: kernel.c
 * @authors: Mark Day, Noah Walle
 * @date: 24.04.2024
 * @purpose: Function definitions for the kernel
 * and scheduling tasks
**/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "kernel.h"

#define MAXTASKS 8

static uint8_t g_numTasks;
static Task_t* g_taskArray;
static volatile uint32_t g_count = 0;
static volatile uint32_t g_lastCount = 0;

void
SysTickIntHandler(void)
{
    g_count++;
}

void
InitClock(uint32_t KERNEL_RATE_HZ)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / KERNEL_RATE_HZ);

    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);

    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

void 
InitKernel(uint32_t KERNEL_RATE_HZ)
{
    g_numTasks = 0;
    g_taskArray = calloc(MAXTASKS, sizeof(Task_t));

    InitClock(KERNEL_RATE_HZ);
}

int 
TaskCompare(const void * t_task_A, const void * t_task_B)
{
    const Task_t* task_A = (const Task_t *)t_task_A;
    const Task_t* task_B = (const Task_t *)t_task_B;

    return (int)task_A->Priority - (int)task_B->Priority;
}

void 
AddTask(void* functionPtr, uint16_t numTicks, uint8_t priority, uint8_t runTask)
{
    Task_t task = {functionPtr, numTicks, priority, runTask, 0};
    g_taskArray[g_numTasks] = task;
    g_numTasks++;

    if (g_numTasks >= 2) {
        qsort(g_taskArray, g_numTasks, sizeof(Task_t), TaskCompare);
    }
}

void 
TaskEnable(void* functionPtr)
{
    uint8_t i;
    for (i = 0; i < g_numTasks; i++)
    {
        if (functionPtr == g_taskArray[i].FunctionPtr)
        {
            g_taskArray[i].RunTask = 1;
        }
    }
}

void 
TaskDisable(void* functionPtr)
{
    uint8_t i;
    for (i = 0; i < g_numTasks; i++)
    {
        if (functionPtr == g_taskArray[i].FunctionPtr)
        {
            g_taskArray[i].RunTask = 0;
        }
    }
}

void 
RunKernel(void)
{
    if (g_count != g_lastCount){
        uint8_t i;
        for (i = 0; i < g_numTasks; i++) {

            Task_t task = g_taskArray[i];
            uint32_t delta_ticks = g_count - task.LastRun;
            if (task.RunTask && (task.NumTicks == 0 || delta_ticks >= task.NumTicks)){ //runs if enabled and 0 or its due to

                task.LastRun = g_count;
                g_taskArray[i] = task;
                //run task
                ((void(*)(Task_t*))(task.FunctionPtr))(&task);
            }
        }
        g_lastCount= g_count;
    }
}
