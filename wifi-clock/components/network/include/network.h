#ifndef _NETWORK_H_
#define _NETWORK_H_


#include "esp_wifi.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t ssid[33];
    wifi_auth_mode_t authmode;
} wifi_device_t;


typedef enum
{
    ZL_SCANF_DONE = 0,
    ZL_GOT_IP,
    ZL_DISCONNECTED,
    ZL_NORMAL
}ZL_WIFI_EVENT_t;



extern TaskHandle_t scanf_handle;
extern TaskHandle_t sntp_handle;
extern EventGroupHandle_t wifi_event_group;
extern const int CONNECTED_BIT;
extern const int SCANF_BIT;


void wifi_init(void);
void scanf_task(void *arg);
void sntp_task(void *arg);

#ifdef __cplusplus
}
#endif

#endif