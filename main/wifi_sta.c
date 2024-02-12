#include <string.h>
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

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;


static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    char buff[100];
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    strcpy((char *)wifi_config.sta.ssid,(char *)nvs_struct.wifi_ssid);
    strcpy((char *)wifi_config.sta.password,(char *)nvs_struct.wifi_pass);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    epd_clear();
    if (bits & WIFI_CONNECTED_BIT) {
        header();
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 nvs_struct.wifi_ssid, nvs_struct.wifi_pass);
        
        sprintf(buff,"WiFi connected to:%s",nvs_struct.wifi_ssid);
        epd_disp_string(buff, 0, 100);
        client_get_function();
    } else if (bits & WIFI_FAIL_BIT) {
        header();
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 nvs_struct.wifi_ssid, nvs_struct.wifi_pass);
        value_plus_info();
        line_chart_visual();
        epd_udpate();
    } else {
        header();
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        sprintf(buff,"WiFi connection error!");
        epd_disp_string(buff, 0, 150);
        sprintf(buff,"Press OK button to WEB configuration");
        epd_disp_string(buff, 0, 200);
        epd_udpate();
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
    esp_wifi_disconnect();

}


esp_err_t client_event_handler(esp_http_client_event_handle_t evt)
{
    char buff[100];
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        sprintf(buff,"GET request to:");
        epd_disp_string(buff, 0, 150);
        sprintf(buff,"stage6.api.logimic.online/alive/alive");
        epd_disp_string(buff, 0, 200);
        sprintf(buff,"Response: %.*s", evt->data_len, (char *)evt->data);
        printf("HTTP_EVENT_ON_DATA: %s\n", buff);
        epd_disp_string(buff, 0, 250);
        epd_udpate();
        break;

    default:
        break;
    }
    return ESP_OK;
}



static void client_get_function(void)
{
 
esp_http_client_config_t clientConfig = {
    //.url = "https://worldtimeapi.org/api/timezone/Europe/London/",
     .url = "https://stage6.api.logimic.online/alive/alive",
    //.url = "http://httpbin.org/post",
     //.url = "https://www.google.com",
      .transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
      .crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
      .event_handler = client_event_handler};
  

        
    esp_http_client_handle_t client = esp_http_client_init(&clientConfig);

    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);

}

