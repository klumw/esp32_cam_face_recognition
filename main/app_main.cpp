#include "who_camera.h"
#include "human_face_recognition.hpp"
#include "who_button.h"
#include "event_logic.hpp"
#include "app_wifi.h"
#include "mqtt_handler.hpp" 
#include "face_recognition_tool.hpp"
#include "led.h"

#define GPIO_BOOT GPIO_NUM_0
#define BUTTON_FUNC GPIO_NUM_14


static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueKeyState = NULL;
static QueueHandle_t xQueueEventLogic = NULL;
static QueueHandle_t xQueueFaceResult = NULL;
static QueueHandle_t xQueueLed = NULL;

extern "C" void app_main()
{
    app_wifi_main();//STA or AP mode
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueKeyState = xQueueCreate(1, sizeof(int *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));
    xQueueFaceResult = xQueueCreate(1,sizeof(face_info_t *));
    xQueueLed = xQueueCreate(1,sizeof(led_mstate_t *));

    register_button(BUTTON_FUNC, xQueueKeyState);
    //register_camera(PIXFORMAT_RGB565, FRAMESIZE_240X240, 2, xQueueAIFrame);
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_VGA, 2, xQueueAIFrame);
    register_event(xQueueKeyState, xQueueEventLogic);
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, xQueueFaceResult, NULL, true);
    register_led(LED_UNRECOGNIZED,LED_RECOGNIZED, xQueueLed);
    register_mqtt(xQueueFaceResult,xQueueLed,xQueueEventLogic);
}
