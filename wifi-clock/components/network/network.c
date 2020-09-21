#include "network.h"
#include "esp_log.h"
#include "cJSON.h"
#include "lwip/apps/sntp.h"
#include "esp_event_loop.h"

//#include "freertos/queue.h"

#include <string.h>

char const *WIFI_SSID[] = {"ecutjwu", "dzxxgc", "yhf-meizu"};
char const *WIFI_PASS[] = {"ec123456", "dz123456", "11111111"};

//#define DEBUG

#define CountWord(keyword)             (sizeof(keyword) / sizeof(keyword[0]))
#define SSIDTableSize                   CountWord(WIFI_SSID)

#define MAX_APs 20

static const char *TAG = "network";

TaskHandle_t scanf_handle;
TaskHandle_t sntp_handle;
EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
const int SCANF_BIT = BIT1;

extern QueueHandle_t uart_queue_handle;

QueueHandle_t wifi_queue_handle;

void wifi_init();
static esp_err_t event_handler(void *ctx, system_event_t *event);
static char* get_authname(wifi_auth_mode_t auth_mode);
static uint8_t lookup_wifi_info(char const * const objword, char const *KW_Table[], int const size);
static void ZL_Obtain_Time(void);
void scanf_task(void *arg);
void sntp_task(void *arg);



void wifi_init()
{
    tcpip_adapter_init();

    wifi_event_group = xEventGroupCreate();
    wifi_queue_handle = xQueueCreate(10, sizeof(ZL_WIFI_EVENT_t));
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
    //ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
}



static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    static ZL_WIFI_EVENT_t ZL_WIFI_EVENT;
    system_event_info_t *info = &event->event_info;

    switch (event->event_id)
    {
        case SYSTEM_EVENT_SCAN_DONE:
        {
            ZL_WIFI_EVENT = ZL_SCANF_DONE;
            ESP_LOGI("event_handler", "Scanf done");
            if(xQueueSend(wifi_queue_handle, &ZL_WIFI_EVENT, 10/portTICK_RATE_MS) == pdPASS)
                ESP_LOGI("event_handler", "Send ZL_SCANF_DONE done!");
            else
                ESP_LOGI("event_handler", "Send ZL_SCANF_DONE failed!");
            //xEventGroupSetBits(wifi_event_group, SCANF_BIT);
        }break;

        //case SYSTEM_EVENT_STA_START:
        //    esp_wifi_connect();
        //    break;

        case SYSTEM_EVENT_STA_GOT_IP:
        {
            ZL_WIFI_EVENT = ZL_GOT_IP;
            if(xQueueSend(wifi_queue_handle, &ZL_WIFI_EVENT, 10/portTICK_RATE_MS) == pdPASS)
                ESP_LOGI("event_handler", "Send ZL_GOT_IP done!");
            else
                ESP_LOGI("event_handler", "Send ZL_GOT_IP failed!");
            //xTaskCreate(sntp_task, "sntp_task", 4 * 1024, NULL, 5, &sntp_handle);
            //xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            //vTaskSuspend(scanf_handle);
        }break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            ZL_WIFI_EVENT = ZL_DISCONNECTED;
            if(xQueueSend(wifi_queue_handle, &ZL_WIFI_EVENT, 10/portTICK_RATE_MS) == pdPASS)
                ESP_LOGI("event_handler", "Send ZL_DISCONNECTED done!");
            else
                ESP_LOGI("event_handler", "Send ZL_DISCONNECTED failed!");            
            ESP_LOGE("event_handler", "Disconnect reason : %d", info->disconnected.reason);
            if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT)
            {
                /*Switch to 802.11 bgn mode */
                esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
            }
            //vTaskDelete(sntp_handle);
            //vTaskResume(scanf_handle);
            //esp_wifi_connect();
            //xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        }break;

        default:
            break;
    }
    return ESP_OK;
}



static char* get_authname(wifi_auth_mode_t auth_mode)
{
	char *names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
	return names[auth_mode];
}



static uint8_t lookup_wifi_info(char const * const objword, char const *KW_Table[], int const size)
{
    char const **kwp;
    for(kwp = KW_Table; kwp < KW_Table + size; ++kwp)
        if(strcmp(objword, *kwp) == 0)
            return kwp - KW_Table;
    return -1;
}



static void zl_sntp_init(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "cn.pool.ntp.org");
    // sntp_setservername(1, "ntp.aliyun.com");
    // sntp_setservername(2, "tw.pool.ntp.org");
    // sntp_setservername(3, "pool.ntp.org");
    sntp_init();
}



static void ZL_Obtain_Time(void)
{
    //xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
    //                    false, true, portMAX_DELAY);
    zl_sntp_init();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) 
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        if(retry == (retry_count-1))
            esp_restart();
    }
}


void scanf_task(void *arg)
{
    wifi_scan_config_t scan_config = {
	    .ssid = 0,
	    .bssid = 0,
	    .channel = 0,
        .show_hidden = true
    };
	uint16_t ap_num = MAX_APs;
	wifi_ap_record_t ap_records[MAX_APs];
    static ZL_WIFI_EVENT_t ZL_WIFI_EVENT = ZL_DISCONNECTED;
    
    //ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    while(1)
    {
        if(xQueueReceive(wifi_queue_handle, &ZL_WIFI_EVENT, 10/portTICK_RATE_MS) == pdTRUE)
        {
            ESP_LOGI("ZL_WIFI_EVENT", "Receive done");
        }
        switch (ZL_WIFI_EVENT)
        {
            case ZL_SCANF_DONE:
            {
                ZL_WIFI_EVENT = ZL_DISCONNECTED;
                ap_num = MAX_APs;
                //ESP_LOGI(TAG, "ap_num:%d",ap_num);
                ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
                ESP_LOGI(TAG, "ap_num:%d,checking ssid...",ap_num);
                for(int i = 0; i < ap_num; i++)
                {
                    ESP_LOGI(TAG, "found ap[%d]:%s",i,ap_records[i].ssid);
                    int8_t loc = lookup_wifi_info((char const * const)ap_records[i].ssid, WIFI_SSID, SSIDTableSize);
                    if(loc != -1)              //here has something wrong
                    {
                        wifi_config_t wifi_config;
                        strcpy((char *)wifi_config.sta.ssid, WIFI_SSID[loc]);
                        strcpy((char *)wifi_config.sta.password, WIFI_PASS[loc]);
                        ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
                        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
                        ESP_ERROR_CHECK(esp_wifi_start());
                        ESP_ERROR_CHECK(esp_wifi_connect());
                        ZL_WIFI_EVENT = ZL_NORMAL;
                        break;
                    }
                }
            }break;

            case ZL_GOT_IP:
            {
                //ESP_LOGI("ZL_GOT_IP", "[%d]", ZL_GOT_IP);
                vTaskResume(sntp_handle);
                ZL_WIFI_EVENT = ZL_NORMAL;
            }break;
            
            case ZL_DISCONNECTED:
            {
                //ESP_LOGI("ZL_DISCONNECTED", "[%d]", ZL_DISCONNECTED);
                //ESP_LOGI(TAG, "start scanf");
                vTaskSuspend(sntp_handle);
                ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
            }break;       

            default:
                break;
        }
        
        //xQueueSend(wifi_queue_handle, &ZL_WIFI_EVENT, 10/portTICK_RATE_MS);
    }
    ESP_LOGI("scanf_task", "[scanf_task] has been delete unexpectedly!!!");
    vTaskDelete(NULL);
}



void sntp_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    //char strftime_buf[100];
    static char timebuf[30];
    static char sendbuf[100];

    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        ZL_Obtain_Time();
    }
    setenv("TZ", "CST-8", 1);
    tzset();
    while(1)
    {
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year < (2016 - 1900))
        {
            ESP_LOGE(TAG, "The current date/time error");
        }
        else
        {
            // strftime(strftime_buf, sizeof(strftime_buf), "Dat:\"%A\" Mon:\"%b\" Day:\"%d\" Year:\"%Y\" Time:\"%H:%M:%S\"", &timeinfo);
            // strftime(my_time, sizeof(strftime_buf), "Time:\"%H:%M:%S\"Day:\"%d\"Mon:\"%b\"Week:\"%a\"", &timeinfo);
            // ESP_LOGI(TAG, "%s", my_time);
            // ESP_LOGI(TAG, "%s", strftime_buf);
            //printf("xxxxx:%d\n",timeinfo.tm_year);
            printf("shi:%d\n",timeinfo.tm_hour);
            printf("fen:%d\n",timeinfo.tm_min);
            printf("miao:%d\n",timeinfo.tm_sec);
            if(timeinfo.tm_hour == 12 && timeinfo.tm_min == 00 && timeinfo.tm_sec == 00)
                esp_restart();
            cJSON *root = cJSON_CreateObject();

            strftime(timebuf, sizeof(timebuf), "%Y", &timeinfo);
            cJSON_AddStringToObject(root, "Year", timebuf);
            //strftime(timebuf, sizeof(timebuf), "%b", &timeinfo);
            sprintf(timebuf, "%d", timeinfo.tm_mon + 1);
            cJSON_AddStringToObject(root, "Mon", timebuf);
            strftime(timebuf, sizeof(timebuf), "%d", &timeinfo);
            cJSON_AddStringToObject(root, "Day", timebuf);
            strftime(timebuf, sizeof(timebuf), "%H", &timeinfo);
            cJSON_AddStringToObject(root, "Hour", timebuf);
            strftime(timebuf, sizeof(timebuf), "%M", &timeinfo);
            cJSON_AddStringToObject(root, "Min", timebuf);            
            strftime(timebuf, sizeof(timebuf), "%S", &timeinfo);
            cJSON_AddStringToObject(root, "Sec", timebuf);   
            
            char *string = cJSON_Print(root);
            memcpy(sendbuf, string, strlen(string));
            free(string);
            cJSON_Delete(root);
            ESP_LOGI(TAG, "%s", sendbuf);

            if(xQueueSend(uart_queue_handle, sendbuf, 10/portTICK_RATE_MS) == pdPASS)
                ESP_LOGI(TAG, "send data to uart success!");
            else
                ESP_LOGI(TAG, "send data to uart error!");
            memset(sendbuf, 0, sizeof(sendbuf));
        }
        vTaskDelay(1000 / portTICK_RATE_MS);    
    }
    vTaskDelete(NULL);
}
