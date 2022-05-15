#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if CONFIG_LED_ILLUMINATOR_ENABLED
    void app_led_init();
    void app_led_duty(int duty)
#endif

#define LED_RECOGNIZED GPIO_NUM_4
#define LED_UNRECOGNIZED GPIO_NUM_2

typedef enum
{
    LED_ALWAYS_OFF = 0,
    LED_ALWAYS_ON,
    LED_OFF_1S,
    LED_OFF_2S,
    LED_OFF_4S,
    LED_ON_1S,
    LED_ON_2S,
    LED_ON_4S,
    LED_BLINK_1S,
    LED_BLINK_2S,
    LED_BLINK_4S,
} led_state_t;



typedef struct
{
    uint led_num;
    led_state_t state;
    
}led_mstate_t;

void register_led(const gpio_num_t led_io_num_0,const gpio_num_t led_io_num_1, const QueueHandle_t control_i);

#ifdef __cplusplus
}
#endif
