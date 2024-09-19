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
 * File: $Id: porttimer_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Variables ----------------------------------------*/

static TimerHandle_t timer;
static USHORT usT35TimeOut50us;
static void prvvTIMERExpiredISR(void);
static void timer_timeout_ind(TimerHandle_t timer);
static BaseType_t pxHigherPriorityTaskWoken;
/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR(void);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortTimersInit(USHORT usTimeOut50us)
{
    /* backup T35 ticks */
    usT35TimeOut50us = usTimeOut50us;
    timer = xTimerCreate(
        "Slave timer",
        (50 * usT35TimeOut50us /*计算总us数*/) / (1000 /*us->ms */ * 1000 / configTICK_RATE_HZ /* 计算 1 tick的时间*/) + 1 /*向上取整ms*/,
        pdFALSE,
        (void *)2,
        timer_timeout_ind);
    if (timer == NULL)
        return FALSE;
    return TRUE;
}

void vMBMasterPortTimersT35Enable()
{

    uint32_t timer_tick = (50 * usT35TimeOut50us) / (1000 * 1000 / configTICK_RATE_HZ) + 1;
    vMBMasterSetCurTimerMode(MB_TMODE_T35);
    if (IS_IRQ())
    {
        xTimerChangePeriodFromISR((TimerHandle_t)timer, timer_tick, &pxHigherPriorityTaskWoken);
    }
    else
    {
        xTimerChangePeriod((TimerHandle_t)timer, timer_tick, 0);
    }
}

void vMBMasterPortTimersConvertDelayEnable()
{
    uint32_t timer_tick = MB_MASTER_DELAY_MS_CONVERT * configTICK_RATE_HZ / 1000;
    vMBMasterSetCurTimerMode(MB_TMODE_CONVERT_DELAY);
    if (IS_IRQ())
    {
        xTimerChangePeriodFromISR((TimerHandle_t)timer, timer_tick, &pxHigherPriorityTaskWoken);
    }
    else
    {
        xTimerChangePeriod((TimerHandle_t)timer, timer_tick, 0);
    }
}

void vMBMasterPortTimersRespondTimeoutEnable()
{
    uint32_t timer_tick = MB_MASTER_TIMEOUT_MS_RESPOND * configTICK_RATE_HZ / 1000;
    vMBMasterSetCurTimerMode(MB_TMODE_RESPOND_TIMEOUT);
    if (IS_IRQ())
    {
        xTimerChangePeriodFromISR((TimerHandle_t)timer, timer_tick, &pxHigherPriorityTaskWoken);
    }
    else
    {
        xTimerChangePeriod((TimerHandle_t)timer, timer_tick, 0);
    }
}

void vMBMasterPortTimersDisable()
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
    (void)pxMBMasterPortCBTimerExpired();
}

static void timer_timeout_ind(TimerHandle_t timer)
{
    prvvTIMERExpiredISR();
}

#endif
