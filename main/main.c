/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include <time.h>
#include "hardware/gpio.h"
//#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

const int TRIGGER = 14;
const int ECHO = 15;

volatile bool trigger_state = false;
volatile bool echo_state = false;
//volatile absolute_time_t trigger_on_t;
volatile absolute_time_t echo_on_t;
volatile int echo_duration;
volatile bool p_state = false;

void echo_callback(uint gpio, uint32_t events){
    if (events == GPIO_IRQ_EDGE_RISE){
        echo_on_t = get_absolute_time();
        echo_state = true;
    } else if (events == GPIO_IRQ_EDGE_FALL) {
        if (echo_state){
            absolute_time_t echo_off_t = get_absolute_time();
            echo_duration = absolute_time_diff_us(echo_on_t, echo_off_t);
            echo_state = false;
            p_state = true;
            //trigger_state = false;
        }
        echo_state = false;
    }
}

bool trigger_timer_callback([[maybe_unused]] repeating_timer_t *rt) {
    trigger_state = true;
    //trigger_on_t = get_absolute_time();
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(TRIGGER);
    gpio_set_dir(TRIGGER, GPIO_OUT);

    gpio_init(ECHO);
    gpio_set_dir(ECHO, GPIO_IN);

    gpio_set_irq_enabled_with_callback(ECHO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);

    int trigger_timer_hz = 1;
    repeating_timer_t trigger_timer;
    
    if (!add_repeating_timer_us(1000000 / trigger_timer_hz, trigger_timer_callback, NULL, &trigger_timer)) {
        printf("Failed to add timer\n");
    }

    int start = 0;

    while (true) {
        
        //if (trigger_state){
        //    gpio_put(TRIGGER, 1);
        //    absolute_time_t trigger_off_t = get_absolute_time();
        //    int64_t trigger_duration = absolute_time_diff_us(trigger_on_t, trigger_off_t);
        //    if(trigger_duration >= 10){
        //        gpio_put(TRIGGER, 0);
        //        trigger_state = false;
        //    }
        //}

        int str = getchar_timeout_us(500);
        if(str == 's'){
            start = !start;
        }

        if(start){
            if(trigger_state){
                gpio_put(TRIGGER, 1);
                sleep_us(10);
                gpio_put(TRIGGER, 0);
                trigger_state = false;
            }
    
            if (p_state){
                //time_t tempo_atual = get_absolute_time();
                //uint64_t ms = to_ms_since_boot(tempo_atual);
    
                absolute_time_t tempo_atual = get_absolute_time();
                uint64_t ms = to_ms_since_boot(tempo_atual);
    
                uint32_t seg = (ms / 1000) % 60;
                uint32_t minutos = (ms / 60000) % 60;
                uint32_t horas = (ms / 3600000) % 24;
    
                printf("%02d:%02d:%02d - ", horas, minutos, seg);
                double d = (0.0340*echo_duration)/2.0;
                printf("%.2f cm\n", d);
                p_state = false;
            }
        }
    }
    return 0;
}
