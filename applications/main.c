/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2023-05-12     Md. Khairul Alam       first version
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <rtthread.h>
#include <rthw.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "ssd1306_lcd.c"
#include "sim800.c"
#include "hsm20g.c"


#define LCD_I2C i2c0
#define SSD1306_I2C_CLK 400
#define PICO_DEFAULT_I2C_SDA_PIN 16
#define PICO_DEFAULT_I2C_SCL_PIN 17

ssd1306_t disp;
float temprature_in_c;
float relative_humidity;
int temp_state = 0;
int humid_state = 0;

uint8_t api_key[] = "api_key=B3FPE7GTVY1ISGQS&field1=";

// Thread control block declaration //
rt_thread_t read_th_thread  = RT_NULL;
rt_thread_t display_th_thread  = RT_NULL;
rt_thread_t data_to_cloud_thread  = RT_NULL;
rt_thread_t notification_thread  = RT_NULL;


void I2C_init(void){
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    i2c_init(LCD_I2C, SSD1306_I2C_CLK * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

void SSD1306_init(void){
    I2C_init();
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, LCD_I2C);
    ssd1306_clear(&disp);
}

void system_init(void){
    stdio_init_all();
    uart1_init();
    SSD1306_init();
    ADC_init();
}


void read_th(void* parameter)
{

    while(1)
    {
        //rt_kprintf("Reading the sensor!\n");
        sensor_reading reading;
        read_from_sensor(&reading);
        temprature_in_c = reading.temperature;
        relative_humidity = reading.humidity;

        rt_thread_mdelay(2000);

    }
}

void display_th(void* parameter)
{

    while(1)
    {
       //rt_kprintf("Displaying data!\n");
       char pre_temp[20] = "T: ";
       char temp[20];
       char post_temp[20] = " C";

       int int_temp = (int)temprature_in_c;
       float float_part = (abs(temprature_in_c) - abs(int_temp)) * 100;
       int float_temp = (int)float_part;

       rt_sprintf(temp, "%d.%d", int_temp, float_temp);
       strcat(pre_temp, temp);
       strcat(pre_temp, post_temp);

       ssd1306_draw_string(&disp, 8, 24, 2, pre_temp);
       ssd1306_show(&disp);

       char pre_humid[20] = "H: ";
       char humid[20];
       char post_humid[20] = " %";

       int int_humid = (int)relative_humidity;
       float float_hpart = (abs(relative_humidity) - abs(int_humid)) * 100;
       int float_humid = (int)float_hpart;

       rt_sprintf(humid, "%d.%d", int_humid, float_humid);
       strcat(pre_humid, humid);
       strcat(pre_humid, post_humid);

       ssd1306_draw_string(&disp, 8, 44, 2, pre_humid);
       ssd1306_show(&disp);

       rt_thread_mdelay(15000);
       ssd1306_clear(&disp);
    }
}

void data_to_cloud(void* parameter)
{

    while(1)
    {
        //rt_kprintf("Sending to cloud!\n");
        send_data(api_key, (int)temprature_in_c, (int)relative_humidity);
        //send data to cloud every 10 seconds
        rt_thread_mdelay(20000);
    }
}

void send_notification(void* parameter)
{

    while(1)
    {
        //rt_kprintf("Sending notification!\n");
        if(temprature_in_c>4){
            if(temp_state==0){
                temperature_high_alart();
                temp_state = 1;
            }
        }
        else{
            if(temp_state==1)
                temp_state = 0;
        }
        if(relative_humidity>50){
            if(humid_state==0){
                humidity_high_alart();
                humid_state = 1;
            }
        }
        else{
            if(humid_state==1)
                humid_state = 0;
        }
        rt_thread_mdelay(3000);
    }
}


void Run(void)
{
    //         Creating the threads             //
    read_th_thread =                            //thread control block pointer
        rt_thread_create( "ReadTh",             //thread name
                          read_th,              //thread entry function
                          RT_NULL,                  //thread entry function parameters
                          512,                      //thread stack size
                          1,                        //thread priority
                          20);                      //thread time slice
        if (read_th_thread != RT_NULL)
            rt_thread_startup(read_th_thread);  //make the thread enter the ready state

    display_th_thread = rt_thread_create("Display", display_th, RT_NULL, 512, 2, 20);
    if (display_th_thread != RT_NULL)
        rt_thread_startup(display_th_thread);

    data_to_cloud_thread = rt_thread_create( "DataToCloud", data_to_cloud, RT_NULL, 512, 2, 20);
    if (data_to_cloud_thread != RT_NULL)
        rt_thread_startup(data_to_cloud_thread);

    notification_thread = rt_thread_create( "SendNotification", send_notification, RT_NULL, 512, 2, 20);
    if (notification_thread != RT_NULL)
        rt_thread_startup(notification_thread);

}


int main(void)
{
    rt_kprintf("Hello, RT-Thread!\n");

    system_init();
    Run();


}
