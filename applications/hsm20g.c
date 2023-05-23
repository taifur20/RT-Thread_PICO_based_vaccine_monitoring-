/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-19     Md. Khairul Alam       the first version
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "drivers/adc.h"

#define TEMP_PIN 26
#define HUMID_PIN 27

typedef struct{
    float humidity;
    float temperature;
}sensor_reading;

void read_from_sensor(sensor_reading *result);
void ADC_init(void);

void read_from_sensor(sensor_reading *result){

    adc_select_input(0);
    uint humid_raw = adc_read();
    adc_select_input(1);
    uint temp_raw = adc_read();

    const float conversion_factor = 3.3f / (1 << 12);

    float temp_voltage = temp_raw * conversion_factor;
    float humid_voltage = humid_raw * conversion_factor;

    //int temperature=(5.26*pow(voltage_temp,3))-(27.34*pow(voltage_temp,2))+(68.87*voltage_temp)-17.81;
    result->temperature = (5.26*temp_voltage*temp_voltage*temp_voltage)-(27.34*temp_voltage*temp_voltage)+(68.87*temp_voltage)-7.81;
    //printf("Temp = %f\t\n", result->temperature);

    result->humidity = (3.71*pow(humid_voltage,3))-(20.65*pow(humid_voltage,2))+(64.81*humid_voltage)-27.44;
    //printf("Humidity = %.2f", result->humidity);

}

void ADC_init(void){
    adc_init();
    adc_gpio_init(TEMP_PIN);
    adc_gpio_init(HUMID_PIN);
}
