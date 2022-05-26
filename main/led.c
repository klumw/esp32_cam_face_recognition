#include "led.h"
#include "esp_log.h"

static const char *TAG = "app_led";
static QueueHandle_t xQueueLEDControlI = NULL;

void led_task(void *arg)
{
    led_mstate_t led_op,*pLed_op = NULL;
    while (1)
    {
        xQueueReceive(xQueueLEDControlI, &(pLed_op), portMAX_DELAY);
        led_op = *pLed_op;
#ifdef APP_DEBUG        
        ESP_LOGI("LED_TASK","High Watermark: %d",uxTaskGetStackHighWaterMark(NULL));
#endif        
        switch (led_op.state)
        {
        case LED_ALWAYS_OFF:
            gpio_set_level(led_op.led_num, 0);
            break;
        case LED_ALWAYS_ON:
            gpio_set_level(led_op.led_num, 1);
            break;
        case LED_OFF_1S:
            gpio_set_level(led_op.led_num, 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(led_op.led_num, 1);
            break;
        case LED_OFF_2S:
            gpio_set_level(led_op.led_num, 0);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            gpio_set_level(led_op.led_num, 1);
            break;
        case LED_OFF_4S:
            gpio_set_level(led_op.led_num, 0);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            gpio_set_level(led_op.led_num, 1);
            break;
        case LED_ON_1S:
            gpio_set_level(led_op.led_num, 1);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(led_op.led_num, 0);
            break;
        case LED_ON_2S:
            gpio_set_level(led_op.led_num, 1);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            gpio_set_level(led_op.led_num, 0);
            break;
        case LED_ON_4S:
            gpio_set_level(led_op.led_num, 1);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            gpio_set_level(led_op.led_num, 0);
            break;
        case LED_BLINK_1S:
            for (int i = 0; i < 2; ++i)
            {
                gpio_set_level(led_op.led_num, 1);
                vTaskDelay(250 / portTICK_PERIOD_MS);
                gpio_set_level(led_op.led_num, 0);
                vTaskDelay(250 / portTICK_PERIOD_MS);
            }
            break;
        case LED_BLINK_2S:
            for (int i = 0; i < 4; ++i)
            {
                gpio_set_level(led_op.led_num, 1);
                vTaskDelay(250 / portTICK_PERIOD_MS);
                gpio_set_level(led_op.led_num, 0);
                vTaskDelay(250 / portTICK_PERIOD_MS);
            }
            break;
        case LED_BLINK_4S:
            for (int i = 0; i < 8; ++i)
            {
                gpio_set_level(led_op.led_num, 1);
                vTaskDelay(250 / portTICK_PERIOD_MS);
                gpio_set_level(led_op.led_num, 0);
                vTaskDelay(250 / portTICK_PERIOD_MS);
            }
            break;

        default:
            break;
        }
    }
}

void register_led(const gpio_num_t led_io_num_0,const gpio_num_t led_io_num_1, const QueueHandle_t control_i)
{
    esp_err_t err;
    xQueueLEDControlI = control_i;
    gpio_config_t gpio_conf;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pull_up_en = 1;
    gpio_conf.pull_down_en = 0;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.pin_bit_mask = (1ULL << (int)led_io_num_0) | (1ULL << (int)led_io_num_1);
    err = gpio_config(&gpio_conf);
    if(err != ESP_OK){
        ESP_LOGE(TAG,"led gpio config error");
    }
    xTaskCreatePinnedToCore(&led_task, "led_task", 3 * 1024, NULL, 5, NULL, 0);
}
