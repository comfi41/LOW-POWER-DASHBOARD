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
#include "get_sntp.h"
#include "data_processing.h"

extern int noOfRecords;

char deviceName[50];
char groupName[50];
double actual_value;
double *device_values;
int values_count;
char unit[50];
char param[50];

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
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 nvs_struct.wifi_ssid, nvs_struct.wifi_pass);
        
        //sprintf(buff,"WiFi connected to:%s",nvs_struct.wifi_ssid);
        //epd_disp_string(buff, 0, 100);


        esp_task_wdt_add(NULL); 
        esp_task_wdt_reset();
        parsing_pointer=TOKEN;
        client_post_function();
        esp_task_wdt_reset();
        parsing_pointer=GET_NUMBER_DEVS;
        client_get_function(parsing_pointer);
        esp_task_wdt_reset();
        parsing_pointer=GET_GROUPS;
        client_get_function(parsing_pointer);
         esp_task_wdt_reset();
         parsing_pointer=GET_SENSORS_VALUES;
        client_get_function(parsing_pointer);
        esp_task_wdt_reset();
        //client_get_function(3);
                
        get_time_sntp();
        printf("NUMBER OF GROUPS!!!: %d\n",noOfRecords);
        int chosen_sensor = 6; //only for testing
        int number_of_sensors; // == devices
        //from device (sensor) UID gets more device info - > name only right now 
        for (int i = 0; i < noOfRecords; i++) 
        {
            if ((sensor_struct+i)->sensor_id == chosen_sensor)
            {
              strcpy(deviceName, (sensor_struct+i)->name);
              goto deviceName_end;
            }
        }
        deviceName_end:;
        //from device (sensor) UID gets more group info - > name only right now 
        for (int i = 0; i < noOfRecords; i++) 
        {
          number_of_sensors = sizeof((group_struct+i)->sensors_id) / sizeof((group_struct+i)->sensors_id[0]);
          for (int x = 0; x < number_of_sensors; x++)
            if ((group_struct+i)->sensors_id[x] == chosen_sensor)
            {
              strcpy(groupName, (group_struct+i)->name);
              goto groupName_end;
            }
        }
        groupName_end:;
        for (int i = 0; i < noOfRecords; i++) 
        {
            if ((history_struct+i)->sensor_id == chosen_sensor)
            {
              actual_value = (history_struct+i)->values[(history_struct+i)->number_of_records - 1];
              strcpy(unit, (history_struct+i)->unit_param);
              strcpy(param, (history_struct+i)->name_param);
              device_values = (double *)malloc((history_struct+i)->number_of_records * sizeof(double));
              values_count = (history_struct+i)->number_of_records;
              memcpy(device_values, (history_struct+i)->values, (history_struct+i)->number_of_records * sizeof(double));
              if (device_values == NULL) {
                 ESP_LOGI(TAG, "Mem allocation failed.\n");
              }
            }
        }
        printf("Device/sensor name: %s\n",deviceName);
        printf("Group name: %s\n",groupName);
        
        header();
        if (nvs_struct.chosen_visual == 1) 
        {
          line_chart_visual();
        } 
        else if (nvs_struct.chosen_visual == 2) 
        {
          column_chart_visual();
        }
        else if (nvs_struct.chosen_visual == 3) 
        {
          scatter_plot_visual();
        }
        epd_udpate();
    } else if (bits & WIFI_FAIL_BIT) {
        header();
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 nvs_struct.wifi_ssid, nvs_struct.wifi_pass);
        wifi_error();
        error();
        epd_udpate();
    } else {
        header();
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        sprintf(buff,"WiFi connection error!");
        epd_disp_string(buff, 0, 150);
        sprintf(buff,"Press OK button to WEB configuration");
        epd_disp_string(buff, 0, 200);
        epd_clear();
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

 

    esp_task_wdt_reset();
    static int output_len;
    static int mem_counter;
    static char *output_buffer;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        esp_task_wdt_reset();
            if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(1024);
                        output_len = 0;
                        mem_counter = 1;
                        if (output_buffer == NULL) {
                            printf("Failed to allocate memory for output buffer\n");
                            return ESP_FAIL;
                        }
                    }
                    int copy_len = 0;
                    copy_len = evt->data_len;

                    if(((output_len+copy_len)-(mem_counter*1024))<=0)
                    {
                        mem_counter++;
                        output_buffer = (char *) realloc(output_buffer, mem_counter*1024);
                    }
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                               
                output_len += copy_len;
            //}
        break;
    case HTTP_EVENT_ON_FINISH:
        esp_task_wdt_reset();
        memcpy(output_buffer + output_len, "\0", 1);
        //(output_buffer+1+output_len)='\0';
        printf("HTTP_EVENT_ON_FINISH\n");
         if (output_buffer != NULL) 
         {
            parser(output_buffer);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    

    default:
        break;
    }
    return ESP_OK;
}
//esp_task_wdt_delete(NULL);


static void client_get_function(int type_of_req)
{
    int temp_groupID=103;
    char url_temp[150];
    char header_temp[2100];
    sprintf(header_temp,"Bearer %s",token);
    //printf("header_temp: %s\n", header_temp);

    if(type_of_req==GET_NUMBER_DEVS) sprintf(url_temp,"https://stage6.api.logimic.online/deviceModel/get/groups/number?systemId=%d",nvs_struct.systemID);
    if(type_of_req==GET_GROUPS) sprintf(url_temp,"https://stage6.api.logimic.online/kpi/get/groupsExt?systemId=%d",nvs_struct.systemID);
    if(type_of_req==GET_SENSORS_VALUES) sprintf(url_temp,"https://stage6.api.logimic.online/deviceModel/get/devices?systemId=%d&groupId=%d&verbosity=parameterTypeEnums",nvs_struct.systemID,temp_groupID);
    if(type_of_req==GET_SENSORS) sprintf(url_temp,"https://stage6.api.logimic.online/deviceModel/get/deviceModels");

    esp_http_client_config_t clientConfig = {
      .url=url_temp,
      .buffer_size_tx = 8192,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
      .crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
      .event_handler = client_event_handler};
  

        
    esp_http_client_handle_t client = esp_http_client_init(&clientConfig);
    esp_task_wdt_reset();
    esp_http_client_set_header(client, "Authorization", header_temp);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "conname", "iTemp2"); 
    esp_http_client_set_header(client, "Connection", "close");
    esp_task_wdt_reset();
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);

}

static void client_post_function(void)
{
    esp_http_client_config_t clientConfig = {
     .url = "https://logimic-itemp2.auth.us-east-1.amazoncognito.com/oauth2/token",
     //.url = nvs_struct.auth_url,
     .transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
     .method = HTTP_METHOD_POST,
     .crt_bundle_attach = esp_crt_bundle_attach, //Attach the certificate bundle 
     .event_handler = client_event_handler};

     esp_http_client_handle_t client = esp_http_client_init(&clientConfig);
    esp_task_wdt_reset();
    //const char *post_data = "grant_type=client_credentials&client_id=7q7eo84rddr67i3a6mjf7t9cr&client_secret=qs77l0qstbhra2eod12p4afmrhec250pk3s9hmio87duku5mqf1";
    char post_data[512];
    sprintf(post_data,"grant_type=%s&client_id=%s&client_secret=%s",nvs_struct.auth_grant_type,nvs_struct.auth_client_id,nvs_struct.auth_client_secret); 
    printf("%s\n", post_data);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_task_wdt_reset();
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

