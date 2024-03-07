/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_mac.h"
#include <esp_http_server.h>
#include "NV_storage.h"
#include "wifi_ap.h"
#include "epd.h"
#include "visual.h"


/*content of web page*/
char html_page[] = "<!DOCTYPE HTML><html>\n"
                   "<head>\n"
                   "  <title>LOW POWER DASHBOARD CONFIGURATION WEB PAGE</title>\n"
                   "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                   "  <link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">\n"
                   "  <link rel=\"icon\" href=\"data:,\">\n"
                   "  <style>\n"
                   "    html {font-family: Arial; display: inline-block; text-align: center;}\n"
                   "    p {  font-size: 1.2rem;}\n"
                   "    body {  margin: 0;}\n"
                   "    .topnav { overflow: hidden; background-color: #4B1D3F; color: white; font-size: 1.7rem; }\n"
                   "    .content { padding: 20px; }\n"
                   "    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }\n"
                   "    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }\n"
                   "    .reading { font-size: 2.8rem; }\n"
                   "    .card.wifi { color: #0e7c7b; }\n"                       
                   "  </style>\n"
                   "</head>\n"
                   "<body>\n"
                   "  <div class=\"topnav\">\n"
                   "    <h3>LOW POWER DASHBOARD CONFIGURATION WEB PAGE</h3>\n"
                   "  </div>\n"
                   "  <div class=\"content\">\n"
                   "   <form action=\"/get\">\n"
                   "    <div class=\"cards\">\n"
                   "      <div class=\"card wifi\">\n"
                   "        <h4><i class=\"fas wifi\"></i>WIFI SETTINGS</h4>\n"
                   "           Network SSID: <input type=\"text\" name=\"input1\" value=\"%s\" size=\"4\"><br>\n"
                   "           Network password: <input type=\"text\" name=\"input2\" value=\"%s\" size=\"4\"><br><br>\n"
                   "      </div>\n"
                   "      <div class=\"card\">\n"
                   "        <h4><i class=\"fas cloud\"></i>CLOUD SETTINGS</h4>\n"
                   "        <form action=\"/get\">Grant type: <input type=\"text\" name=\"input3\" value=\"%s\" size=\"4\">\n"
                   "        <br>Client ID: <input type=\"text\" name=\"input4\" value=\"%s\" size=\"4\">\n"
                   "        <br>Client password: <input type=\"text\" name=\"input5\" value=\"%s\" size=\"4\">\n"
                   "        <br>Auth scope: <input type=\"text\" name=\"input6\" value=\"%s\" size=\"4\">\n"
                   "        <br>Auth URL: <input type=\"text\" name=\"input7\" value=\"%s\" size=\"4\">\n"
                   "        <br>Cloud URL: <input type=\"text\" name=\"input8\" value=\"%s\" size=\"4\"><br><br>\n"
                   "      </div>\n"
                   "      <div class=\"card\">\n"
                   "        <h4><i class=\"fas refresh\"></i>DISPLAY SETTINGS</h4>\n"
                   "        Refresh time: <input type=\"text\" name=\"input9\" value=\"%d\" size=\"4\"><br><br>\n"
                   "      </div>\n"
                   "      <div class=\"card\">\n"
                   "        <h4><i class=\"fas refresh\"></i>VISUALISATION SETTINGS</h4>\n"
                   "        Choose visual: <select name=\"input10\"><option value=\"1\" %s>Line chart</option><option value=\"2\" %s>Column chart</option><option value=\"3\" %s>Scatter plot</option></select><br><br>\n"
                   "      </div>\n"  
                   "      <i class=\"fas save-button\">\n"
                   "      <a href=\"/mem\"><button>SAVE SETTINGS</button></a><br><p></p>\n"
                   "    </div>\n"
                   "  </form>\n"
                   "  </div>\n"
                   "</body>\n"
                   "</html>";




static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        printf("Station "MACSTR" join, AID=%d\n", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        printf("Station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    char buff[100];
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .channel = 0,
            .authmode = WIFI_AUTH_OPEN,
            .ssid_hidden = 0,
            .max_connection = 1,
            .beacon_interval = 100
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    printf("Wifi AP started. SSID:%s\n", WIFI_SSID);
    header();
    conf_mode();
    epd_udpate();
    setup_server();
}

/*Default web handler*/
httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL};

/*Handler for /get (set_temp form)*/    
httpd_uri_t uri_get_param = {
    .uri = "/get",
    .method = HTTP_GET,
    .handler = get_param_req_handler,
    .user_ctx = NULL};

/*Start web server*/
httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); //default configuration of webserver
    config.stack_size = 8192;
    config.max_resp_headers=50;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) //start web server
    {
        
        httpd_register_uri_handler(server, &uri_get); //create handlers for uri events
        //httpd_register_uri_handler(server, &uri_mem); //create handlers for uri events
        httpd_register_uri_handler(server, &uri_get_param);
        
    }

    return server;
}


/*A function to send a web page to a client.*/
esp_err_t send_web_page(httpd_req_t *req,char alert[])
{
    int response;
    char response_data[sizeof(html_page) + 500]; //create array for webpage and variables
    memset(response_data, 0, sizeof(response_data)); //allocate of memory
    
    char option_1[10] = "";
    char option_2[10] = "";
    char option_3[10] = "";
    
    if (nvs_struct.chosen_visual == 1) 
    {
      strcpy(option_1, " selected");
    } 
    else if (nvs_struct.chosen_visual == 2) 
    {
      strcpy(option_2, " selected");
    }
    else if (nvs_struct.chosen_visual == 3) 
    {
      strcpy(option_3, " selected");
    }
    else
    {
      strcpy(option_1, " selected");
    }
    
    sprintf(response_data, html_page,nvs_struct.wifi_ssid,nvs_struct.wifi_pass,nvs_struct.auth_grant_type,nvs_struct.auth_client_id,nvs_struct.auth_client_secret,nvs_struct.auth_scope,nvs_struct.auth_url,nvs_struct.cloud_url,nvs_struct.refresh_time,option_1,option_2,option_3,alert); //join webpage and variables
    response = httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN); //send to client

    return response;
}

/*Default web handler*/
esp_err_t get_req_handler(httpd_req_t *req)
{
    return send_web_page(req,ALERT_OK); //send webpage to client
}

esp_err_t get_param_req_handler(httpd_req_t *req)
{
    char buff[30];
    printf( "Input: %s\n", req->uri );

    //int temp;
    int counter_pass=0;
    for (char *p = strtok(req->uri,"&"); p != NULL; p = strtok(NULL, "&"))
    {
     printf( "parse: %s\n", p );
    if (sscanf(p, "/get?input1=%s",nvs_struct.wifi_ssid)) counter_pass++;
    printf("Counter 1: %d\n", counter_pass);
    if (sscanf(p, "input2=%s",nvs_struct.wifi_pass)) counter_pass+=2;
    printf("Counter 2: %d\n", counter_pass);
    if (sscanf(p, "input3=%s",nvs_struct.auth_grant_type)) counter_pass+=3;
    printf("Counter 3: %d\n", counter_pass);
    if (sscanf(p, "input4=%s",nvs_struct.auth_client_id)) counter_pass+=4;
    printf("Counter 4: %d\n", counter_pass);
    if (sscanf(p, "input5=%s",nvs_struct.auth_client_secret)) counter_pass+=5;
    printf("Counter 5: %d\n", counter_pass);
    if (sscanf(p, "input6=%s",nvs_struct.auth_scope)) counter_pass+=6;
    printf("Counter 6: %d\n", counter_pass);
    if (sscanf(p, "input7=%s",nvs_struct.auth_url)) counter_pass+=7;
    printf("Counter 7: %d\n", counter_pass);
    if (sscanf(p, "input8=%s",nvs_struct.cloud_url)) counter_pass+=8;
    printf("Counter 8: %d\n", counter_pass);
    if (sscanf(p, "input9=%d",&nvs_struct.refresh_time)) counter_pass+=9;
    printf("Counter 9: %d\n", counter_pass);
    if (sscanf(p, "input10=%d",&nvs_struct.chosen_visual)) counter_pass+=10;
    printf("Counter 10: %d\n", counter_pass);
    }

    if (counter_pass==55)
    {
        printf( "ALL inputs are valid!\n");
        printf("Save to NVS: %d\n", nvs_save());
        epd_clear();
        sprintf(buff,"NEW CONFIGURATION SAVED");
        epd_disp_string(buff, 0, 250);
        epd_udpate();
        send_web_page(req,ALERT_VALID);
        vTaskDelay(2000 / portTICK_PERIOD_MS); //cas pro odeslání stránky
        esp_restart();
            }   
            else   return send_web_page(req,ALERT_ERROR); //send webpage to client (refresh)
}
