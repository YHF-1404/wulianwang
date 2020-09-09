#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "driver/uart.h"

#include "network.h"


#define BUF_SIZE (1024)
static const char *TAG = "wificlock";
QueueHandle_t uart_queue_handle;
TaskHandle_t uart_handle;

static void uart_init(void);
void uart1_task(void *arg);



void app_main()
{
	// initialize NVS
    cJSON *root;
	ESP_ERROR_CHECK(nvs_flash_init());
	uart_queue_handle = xQueueCreate(10, sizeof(char [100]));
	
    wifi_init();
    uart_init();
    
    xTaskCreate(scanf_task, "scanf_task", 8 * 1024, NULL, 5, &scanf_handle);
    xTaskCreate(uart1_task, "uart1_task", 4 * 1024, NULL, 6, &uart_handle);
    xTaskCreate(sntp_task, "sntp_task", 4 * 1024, NULL, 5, &sntp_handle);
    vTaskSuspend(sntp_handle);

    while(1)
    {
        ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
        vTaskDelay(3000 / portTICK_RATE_MS);
    }
}



static void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, NULL);
}



void uart1_task(void *arg)
{
    const uint8_t tail[2] = {0x0A, 0x0D};
    static char my_time[100];
    while(1)
    {
       if(xQueueReceive(uart_queue_handle, my_time, 10/portTICK_RATE_MS) == pdTRUE)
       {
           uart_write_bytes(UART_NUM_1, my_time, strlen(my_time));
           uart_write_bytes(UART_NUM_1, (const char *)tail, sizeof(tail));
           ESP_LOGI("uart", "%s", my_time);
           memset(my_time, 0, sizeof(my_time));
       }
       vTaskDelay(30/portTICK_RATE_MS);
    }
}