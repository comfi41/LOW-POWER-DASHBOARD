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
#include <esp_http_server.h>
#include "esp_mac.h"

#define WIFI_SSID      "Low-Pwr-Dashboard"

#undef CONFIG_HTTPD_MAX_REQ_HDR_LEN
#define CONFIG_HTTPD_MAX_REQ_HDR_LEN 2048
#undef HTTPD_MAX_REQ_HDR_LEN
#define HTTPD_MAX_REQ_HDR_LEN 1024

extern struct NVS_Data nvs_struct;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_softap(void);
httpd_handle_t setup_server(void);
esp_err_t send_web_page(httpd_req_t *req);
esp_err_t get_req_handler(httpd_req_t *req);
esp_err_t get_param_req_handler(httpd_req_t *req);
esp_err_t send_alert_page(httpd_req_t *req);
esp_err_t send_error_page(httpd_req_t *req);