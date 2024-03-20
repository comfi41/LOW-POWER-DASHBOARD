
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "epd.h"
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define EXAMPLE_ESP_MAXIMUM_RETRY  5
extern char token[1536];
extern struct NVS_Data nvs_struct;
extern RingbufHandle_t xRingbuffer;
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 4096

#define GET_NUMBER_DEVS 0
#define GET_GROUPS 1
#define GET_SENSORS 3
#define GET_SENSORS_VALUES 2

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_sta(void);
void Get_current_date_time(char *date_time);
void Set_SystemTime_SNTP(void);
static void client_get_function(int type_of_req);
esp_err_t client_event_handler(esp_http_client_event_handle_t evt);
static void client_post_function(void);

