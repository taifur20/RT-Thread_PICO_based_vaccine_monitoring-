/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-13     khair       the first version
 */
/*
 * gprs.c
 *
 *  Created on: May 15, 2023
 *  Author: Md. Khairul Alam
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <rtthread.h>
#include <rthw.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

//#include "sim800.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE
#define UART_TX_PIN 8
#define UART_RX_PIN 9

void send_data(char *msg, int first_val, int second_val);
void send_apikey(char *msg, int temp, int humid);
void send_command(uint8_t *buf);
void send_test_sms(void);
void make_test_call(void);
void temperature_high_alart(void);
void humidity_high_alart(void);
void uart1_init(void);

void send_data(char *msg, int first_val, int second_val){
    rt_thread_mdelay(1000);
    send_command("AT\r\n");                                             // Check Communication
    rt_thread_mdelay(500);
    send_command("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");              // Connection type GPRS
    rt_thread_mdelay(500);
    send_command("AT+SAPBR=3,1,\"APN\",\"gpinternet\"\r\n");            // APN of the provider
    rt_thread_mdelay(500);
    send_command("AT+SAPBR=1,1\r\n");                                   // Open GPRS context
    rt_thread_mdelay(4000);
    send_command("AT+SAPBR=2,1\r\n");                                   // Query the GPRS context
    rt_thread_mdelay(3000);
    send_command("AT+HTTPINIT\r\n");                                    // Initialize HTTP service
    rt_thread_mdelay(3000);
    send_command("AT+HTTPPARA=\"CID\",1\r\n");                          // Set parameters for HTTP session
    rt_thread_mdelay(3000);
    send_command("AT+HTTPPARA=\"URL\",\"api.thingspeak.com/update\"\r\n"); // Set parameters for HTTP session
    rt_thread_mdelay(5000);
    send_command("AT+HTTPDATA=33,10000\r\n");                           // POST data of size 33 Bytes with maximum latency time of 10seconds for inputting the data
    rt_thread_mdelay(2000);

    send_apikey(msg, first_val, second_val);
    rt_thread_mdelay(5000);

    send_command("AT+HTTPACTION=1\r\n");                                 // Start POST session
    rt_thread_mdelay(4000);
    send_command("AT+HTTPTERM\r\n");                                     // Terminate HTTP service
    rt_thread_mdelay(3000);
    send_command("AT+SAPBR=0,1\r\n");                                    // Close GPRS context
    rt_thread_mdelay(2000);
}

void send_apikey(char *msg, int temp, int humid){
    uint8_t count = 0;
    uint8_t *api_key;
    char temp_str[20];
    char humid_str[20];
    char tmp1[60];
    char key_final[60];

    rt_sprintf(temp_str, "%d", temp);
    rt_sprintf(humid_str, "%d", humid);
    rt_strcpy(tmp1, msg);
    strcat(tmp1, temp_str);
    strcat(tmp1, "&field2=");
    strcat(tmp1, humid_str);
    rt_strcpy(key_final, tmp1);
    strcat(key_final, "\r\n");

    while(count < strlen(key_final))
        {
        uart_putc_raw(uart1, key_final[count++]);
        }

}

void send_command(uint8_t *buf){
    uint8_t *command, count = 0;
    command = buf;
    while(count < strlen(command))
        {
        uart_putc_raw(uart1, command[count++]);
        }
}


void send_test_sms(void)
{
   send_command("AT\r\n");                         /* Check Communication */
   rt_thread_mdelay(500);
   send_command("AT+CMGF=1\r\n");                  // Configuring TEXT mode
   rt_thread_mdelay(200);
   send_command("AT+CMGS=\"+8801719xxxxxx\"\r\n"); // Configuring TEXT mode, set you number
   rt_thread_mdelay(200);
   send_command("RISC-V WCH Test Message.\r\n");   // Configuring TEXT mode
   rt_thread_mdelay(200);
   uart_putc_raw(uart1, 26);
   rt_kprintf("SMS Sent");
   rt_thread_mdelay(500);
}

void make_test_call(void)
{
   send_command("AT\r\n");               /* Check Communication */
   rt_thread_mdelay(2500);
   send_command("ATD+01719xxxxxx;\r\n"); // Configuring TEXT mode
   rt_thread_mdelay(10000);
   rt_thread_mdelay(10000);
   send_command("ATH\r\n");
   rt_kprintf("\r\nCall Sent");
   rt_thread_mdelay(500);
}


void temperature_high_alart(void)
{
   send_command("AT\r\n");                         /* Check Communication */
   rt_thread_mdelay(500);
   send_command("AT+CMGF=1\r\n");                  // Configuring TEXT mode
   rt_thread_mdelay(200);
   send_command("AT+CMGS=\"+8801719xxxxxx\"\r\n"); // Configuring TEXT mode, set your number
   rt_thread_mdelay(200);
   send_command("Temperature is higher than normal.\r\n");   // Configuring TEXT mode
   rt_thread_mdelay(200);
   uart_putc_raw(uart1, 26);
   rt_kprintf("SMS Sent\n");
   rt_thread_mdelay(500);
}

void humidity_high_alart(void)
{
   send_command("AT\r\n");                         /* Check Communication */
   rt_thread_mdelay(500);
   send_command("AT+CMGF=1\r\n");                  // Configuring TEXT mode
   rt_thread_mdelay(200);
   send_command("AT+CMGS=\"+8801719xxxxxx\"\r\n"); // Configuring TEXT mode
   rt_thread_mdelay(200);
   send_command("Humidity is higher than normal.\r\n");   // Configuring TEXT mode
   rt_thread_mdelay(200);
   uart_putc_raw(uart1, 26);
   rt_kprintf("SMS Sent\n");
   rt_thread_mdelay(500);
}


void uart1_init(void)
{
   // Set up our UART with a basic baud rate.
   uart_init(UART_ID, 9600);

   // Set the TX and RX pins by using the function select on the GPIO
   // Set datasheet for more information on function select
   gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
   gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

   // Actually, we want a different speed
   // The call will return the actual baud rate selected, which will be as close as
   // possible to that requested
   uart_set_baudrate(UART_ID, BAUD_RATE);

   // Set UART flow control CTS/RTS, we don't want these, so turn them off
   uart_set_hw_flow(UART_ID, false, false);

   // Set our data format
   uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

   // Turn off FIFO's - we want to do this character by character
   ///uart_set_fifo_enabled(UART_ID, false);

   // Set up a RX interrupt
   // We need to set up the handler first
   // Select correct interrupt for the UART we are using


    // And set up and enable the interrupt handlers
    ///irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
    ///irq_set_enabled(UART1_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    ///uart_set_irq_enables(UART_ID, true, false);

    ///uart_puts(UART_ID, "\nHello, uart interrupts\n");
}
