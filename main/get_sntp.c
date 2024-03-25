#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "epd.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "NV_storage.h"
#include "wifi_sta.h"
#include "esp_crt_bundle.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "visual.h"
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_task_wdt.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"
#include <sys/time.h>
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

char Current_Date_Time[100];
static const char *TAG = "wifi station";

void get_time_sntp(void)
{
    printf("Before update NV = %s\n",nvs_struct.last_update);
    Set_SystemTime_SNTP();
    Get_current_date_time(Current_Date_Time);
    printf("Before update NV 2 = %s\n",nvs_struct.last_update);
    printf("Current = %s\n",Current_Date_Time);
    if (strcmp(nvs_struct.last_update, "") != 0)
    {
      int min1, hour1, day1, month1, year1;
      int min2, hour2, day2, month2, year2;
          
      sscanf(nvs_struct.last_update, "%d.%d.%d %d:%d", &day1, &month1, &year1, &hour1, &min1);
      sscanf(Current_Date_Time, "%d.%d.%d %d:%d", &day2, &month2, &year2, &hour2, &min2);
      if (year1 < year2)
      {
        strcpy(nvs_struct.last_update, Current_Date_Time);
      }
      else if (year1 > year2)
      {
        strcpy(Current_Date_Time, nvs_struct.last_update);
        if (strstr(Current_Date_Time, " - SNTP error") == 0)
        {
          strcat(Current_Date_Time, " - SNTP error");
        }
        strcpy(nvs_struct.last_update, Current_Date_Time);
      }
          else if (month1 < month2)
          {
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else if (month1 > month2)
          {
            strcpy(Current_Date_Time, nvs_struct.last_update);
            if (strstr(Current_Date_Time, " - SNTP error") == 0)
            {
              strcat(Current_Date_Time, " - SNTP error");
            }
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else if (day1 < day2)
          {
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else if (day1 > day2)
          {
            strcpy(Current_Date_Time, nvs_struct.last_update);
            if (strstr(Current_Date_Time, " - SNTP error") == 0)
            {
              strcat(Current_Date_Time, " - SNTP error");
            }
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else if (hour1 < hour2)
          {
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else if (hour1 > hour2)
          {
            strcpy(Current_Date_Time, nvs_struct.last_update);
            if (strstr(Current_Date_Time, " - SNTP error") == 0)
            {
              strcat(Current_Date_Time, " - SNTP error");
            }
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else if (min1 <= min2)
          {
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
          else
          {
            strcpy(Current_Date_Time, nvs_struct.last_update);
            if (strstr(Current_Date_Time, " - SNTP error") == 0)
            {
              strcat(Current_Date_Time, " - SNTP error");
            }
            strcpy(nvs_struct.last_update, Current_Date_Time);
          }
        }
        else
        {
          strcpy(nvs_struct.last_update, Current_Date_Time);
        }
        nvs_save();
        
    printf("Current date and time is = %s\n",nvs_struct.last_update);
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void Get_current_date_time(char *date_time){
	char strftime_buf[64];
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);

    	// Set timezone
  	setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
	tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%d.%m.%Y %H:%M", &timeinfo);
  	ESP_LOGI(TAG, "The current date/time in Brno is: %s", strftime_buf);
    strcpy(date_time,strftime_buf);
}


static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    #ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
        sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    #endif
    esp_sntp_init();
}

static void obtain_time(void)
{
    initialize_sntp();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 5;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void Set_SystemTime_SNTP()  {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
	ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
	obtain_time();
	time(&now);
    }
}
