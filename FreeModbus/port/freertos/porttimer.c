/*
 * FreeModbus Libary: RT-Thread Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static TimerHandle_t timer;

void prvvTIMERExpiredISR(void);
void timer_timeout_ind(TimerHandle_t parameter);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us /* modbus 会根据初始化的波特率来计算出间隔时间是多少个50us*/)
{
    timer = xTimerCreate(
        "Slave timer",
        (50 * usTim1Timerout50us/*计算总us数*/) / (1000/*us->ms */ * 1000 / configTICK_RATE_HZ /* 计算 1 tick的时间*/) + 1/*向上取整ms*/,
        pdFALSE,
        (void *)2,
        timer_timeout_ind);
    if (timer == NULL)
        return FALSE;
    return TRUE;
}

void vMBPortTimersEnable()
{
    if (IS_IRQ())
    {
        xTimerStartFromISR((TimerHandle_t)timer, 0);
    }
    else
    {
        xTimerStart((TimerHandle_t)timer, 0);
    }
}
void vMBPortTimersDisable()
{
    if (IS_IRQ())
    {
        xTimerStopFromISR((TimerHandle_t)timer, 0);
    }
    else
    {
        xTimerStop((TimerHandle_t)timer, 0);
    }
}

void prvvTIMERExpiredISR(void)
{
    pxMBPortCBTimerExpired(); /*freemodbus库回调函数*/
}

/*freertos 软件定时器回调*/
void timer_timeout_ind(TimerHandle_t parameter)
{
    prvvTIMERExpiredISR();
}
