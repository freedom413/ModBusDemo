/*
 * FreeModbus Libary: BARE Port
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
 * File: $Id: port.h ,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#ifndef _PORT_H
#define _PORT_H

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "main.h"
#include "FIFO_lib.h"
#include "usart.h"
#include "mbconfig.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>

#define INLINE
#define PR_BEGIN_EXTERN_C           extern "C" {
#define PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION()    EnterCriticalSection()
#define EXIT_CRITICAL_SECTION()    ExitCriticalSection()

typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif



/*定义调试接口默认输出函数名和行号*/
/*IF DEBUG*/
#define DEBUG 0

#if DEBUG == 1 /*DEBUG Start*/
#define MODBUS_DEBUG(fmt, args...)                                             \
  fprintf(stderr, "  MODBUS_DEBUG(%s:%d):  \t" fmt, __func__, __LINE__, ##args)
#elif DEBUG == 0
#define MODBUS_DEBUG(fmt, args...)                                             \
  do {                                                                         \
  } while (0)
#endif /*DEBUG End*/


void EnterCriticalSection(void);
void ExitCriticalSection(void);
extern __inline BOOL IS_IRQ(void);


#endif
