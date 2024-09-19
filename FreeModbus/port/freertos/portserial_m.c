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
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/
/*软件模拟串行传输 IRQ 处理程序线程*/
static TaskHandle_t thread_serial_soft_trans_irq = NULL;
/*串行事件*/
static EventGroupHandle_t event_serial;
/*Modbus 从站串行设备*/
static UART_HandleTypeDef *serial;

/*定义从机串口接收fifo*/
static Fifo_t g_Master_serial_rx_fifo;
/* 定义从机串口接收fifo最大长度 */
#define g_Master_serial_rx_fifo_maxSize (256)
static uint8_t m_rx_buff[g_Master_serial_rx_fifo_maxSize] = {0};

/* ----------------------- Defines ------------------------------------------*/
/* serial transmit event */
#define EVENT_SERIAL_TRANS_START (1 << 0)

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

/*自定义函数声明*/
static BOOL stm32_putc(CHAR c);
static BOOL stm32_getc(CHAR *c);
static void Master_RxCpltCallback(UART_HandleTypeDef *huart);

/*串口传输任务声明*/
static void serial_soft_trans_irq(void *parameter);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
                             eMBParity eParity)
{
        /* set serial name */
        if (ucPORT == 1)
        {
                serial = &huart1;
                MODBUS_DEBUG("master using uart1!\r\n");
        }
        else if (ucPORT == 2)
        {
                serial = &huart2;
                MODBUS_DEBUG("master using uart2!\r\n");
        }
        else if (ucPORT == 3)
        {
                serial = &huart3;
                MODBUS_DEBUG("master using uart3!\r\n");
        }

        /* set serial configure */
        serial->Init.StopBits = UART_STOPBITS_1;
        serial->Init.BaudRate = ulBaudRate;
        switch (eParity)
        {
        case MB_PAR_NONE:
        {
                serial->Init.WordLength = UART_WORDLENGTH_8B;
                serial->Init.Parity = UART_PARITY_NONE;
                break;
        }
        case MB_PAR_ODD:
        {
                serial->Init.WordLength = UART_WORDLENGTH_9B;
                serial->Init.Parity = UART_PARITY_ODD;
                break;
        }
        case MB_PAR_EVEN:
        {
                serial->Init.WordLength = UART_WORDLENGTH_9B;
                serial->Init.Parity = UART_PARITY_EVEN;
                break;
        }
        }
        if (HAL_UART_Init(serial) != HAL_OK)
        {
                Error_Handler();
        }
        __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
        __HAL_UART_DISABLE_IT(serial, UART_IT_TC);
        /*registe recieve callback*/
        HAL_UART_RegisterCallback(serial, HAL_UART_RX_COMPLETE_CB_ID,
                                  Master_RxCpltCallback);
        /* software initialize */

        /* fifo初始化 */
        FifoInit(&g_Master_serial_rx_fifo, m_rx_buff, g_Master_serial_rx_fifo_maxSize);

        /* 创建串口事件组 */
        event_serial = xEventGroupCreate();
        if (NULL != event_serial)
        {
                MODBUS_DEBUG("Create Master event_serial Event success!\r\n");
        }
        else
        {
                MODBUS_DEBUG("Create Master event_serial Event  Faild!\r\n");
        }

        /* 创建串口处理任务 */
        BaseType_t xReturn =
            xTaskCreate((TaskFunction_t)serial_soft_trans_irq,
                        (const char *)"Master trans",
                        (uint16_t)128,
                        (void *)NULL,
                        (UBaseType_t)12,
                        (TaskHandle_t *)&thread_serial_soft_trans_irq);

        if (xReturn == pdPASS)
        {
                MODBUS_DEBUG("xTaskCreate Master trans success\r\n");
        }
        else
        {
                MODBUS_DEBUG("xTaskCreate Master trans Faild!\r\n");
        }

        return TRUE;
}

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
        __HAL_UART_CLEAR_FLAG(serial, UART_FLAG_RXNE);
        __HAL_UART_CLEAR_FLAG(serial, UART_FLAG_TC);
        if (xRxEnable)
        {
                /* enable RX interrupt */
                __HAL_UART_ENABLE_IT(serial, UART_IT_RXNE);
        }
        else
        {
                /* disable RX interrupt */
                __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
        }
        if (xTxEnable)
        {
                /* start serial transmit */
                xEventGroupSetBits(event_serial, EVENT_SERIAL_TRANS_START);
        }
        else
        {
                /* stop serial transmit */
                xEventGroupClearBits(event_serial, EVENT_SERIAL_TRANS_START);
        }
}

void vMBMasterPortClose(void)
{
        __HAL_UART_DISABLE(serial);
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
        return stm32_putc(ucByte);
}

BOOL xMBMasterPortSerialGetByte(CHAR *pucByte)
{
        if (FifoOut(&g_Master_serial_rx_fifo, (uint8_t *)pucByte, 1) != 0)
        {
                return TRUE;
        }
        else
        {
                return FALSE;
        }
}

/*
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
        pxMBMasterFrameCBTransmitterEmpty();
}

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
        pxMBMasterFrameCBByteReceived();
}

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void serial_soft_trans_irq(void *parameter)
{
        while (1)
        {
                xEventGroupWaitBits(event_serial,
                                    EVENT_SERIAL_TRANS_START,
                                    pdFALSE,
                                    pdFALSE,
                                    portMAX_DELAY);

                prvvUARTTxReadyISR();
        }
}

/* 从机串口接收中断回调*/
static void Master_RxCpltCallback(UART_HandleTypeDef *huart)
{
        CHAR ch;
        while (1)
        {
                if (stm32_getc(&ch))
                {
                        FifoIn(&g_Master_serial_rx_fifo, (uint8_t *)&ch, 1);
                }
                else
                {
                        break;
                }
        }
        prvvUARTRxISR();
}

/*UART阻塞式发送*/
static BOOL stm32_putc(CHAR c)
{
        serial->Instance->DR = c;
        while (!(serial->Instance->SR & UART_FLAG_TC))
                ;
        return TRUE;
}
/*UART接收*/
static BOOL stm32_getc(CHAR *c)
{
        if (serial->Instance->SR & UART_FLAG_RXNE)
        {
                *c = serial->Instance->DR & 0xff; /* 读取DR寄存器可以清除RXNE标志位*/
                return TRUE;
        }
        return FALSE;
}

#endif
