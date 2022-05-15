
#include "mqtt_handler.hpp"

#define BROKER_URI CONFIG_MQTT_HOST
#define TOPIC_FACE_RECOGNITION CONFIG_MQTT_TOPIC_RECOGNITION
#define TOPIC_CAM_COMMAND CONFIG_MQTT_TOPIC_COMMAND
#define TAG "MQTT"

static QueueHandle_t xQueueMqtt = NULL;
static QueueHandle_t xQueueLed = NULL;
static QueueHandle_t xQueueEvent = NULL;
static face_info_t face_info;
static led_mstate_t led_state;

static struct
{
    esp_mqtt_client_handle_t client;
    uint8_t connected;
} mqtt;

static void sendCamCommand(esp_mqtt_event_handle_t event)
{
    recognizer_state_t state;
    state = (recognizer_state_t)atoi(event->data);
    xQueueSend(xQueueEvent, &state, 0);
}

/*mqtt event handler*/
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        mqtt.connected = 1;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, TOPIC_CAM_COMMAND, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        mqtt.connected = 0;
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
#ifdef APP_DEBUG
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
#endif
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        sendCamCommand(event);
#ifdef APP_DEBUG
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
#endif
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

/* Standard esp event handler that calls the mqtt event handler */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
}

static void mqtt_app_start()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URI,
    };
    mqtt.client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt.client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt.client);
    esp_mqtt_client_start(mqtt.client);
}

static esp_err_t mqtt_publish(const char *topic, const char *data)
{
    int msg_id = esp_mqtt_client_publish(mqtt.client, topic, data, 0, 1, 1);
    if (msg_id < 0)
    {
        ESP_LOGE(TAG, "Unable to publish topic to %s", BROKER_URI);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void mqtt_task(void *arg)
{
    char buf[3];
    led_mstate_t *pLedState;
    mqtt_app_start();
    while (true)
    {
        xQueueReceive(xQueueMqtt, &face_info, portMAX_DELAY);
        itoa(face_info.id, buf, 10);
        mqtt_publish(TOPIC_FACE_RECOGNITION, buf);
#ifdef APP_DEBUG
        ESP_LOGI(TAG, "High Watermark: %d", uxTaskGetStackHighWaterMark(NULL));
        ESP_LOGI(TAG, "Free Heap size: %d", xPortGetFreeHeapSize());
#endif
        if (face_info.id > 0)
        {
            led_state.led_num = LED_RECOGNIZED;
            led_state.state = LED_ON_1S;
#ifdef APP_DEBUG
            ESP_LOGI(TAG, "Face Recognized, led_num: %d, state: %d", led_state.led_num, led_state.state);
#endif
        }
        else
        {
            led_state.led_num = LED_UNRECOGNIZED;
            led_state.state = LED_ON_1S;
#ifdef APP_DEBUG
            ESP_LOGE(TAG, "Face unrecognized, led_num: %d, state: %d", led_state.led_num, led_state.state);
#endif
        }
        pLedState = &led_state;
        xQueueSend(xQueueLed, &pLedState, portMAX_DELAY);
        vTaskDelay(1000);
    }
}

void register_mqtt(const QueueHandle_t xQueue_mqtt_i, const QueueHandle_t xQueue_led_o, const QueueHandle_t xQueue_event_o)
{
    xQueueMqtt = xQueue_mqtt_i;
    xQueueLed = xQueue_led_o;
    xQueueEvent = xQueue_event_o;
    xTaskCreatePinnedToCore(mqtt_task, "mqtt_task", 3 * 1024, NULL, 5, NULL, 1);
}