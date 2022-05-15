#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "face_recognition_tool.hpp"
#include "human_face_recognition.hpp"
#include "led.h"

void register_mqtt(const QueueHandle_t xQueue_mqtt_i,const QueueHandle_t xQueue_led_o,const QueueHandle_t xQueue_event_o);