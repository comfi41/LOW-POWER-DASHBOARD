#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

struct NVS_Data{
char wifi_ssid[50];
char wifi_pass[50];
char auth_grant_type[50];
char auth_client_id[50];
char auth_client_secret[50];
char auth_scope[50];
char auth_url[50];
char cloud_url[50];
int refresh_time;
int chosen_visual;
//add last update time
//add chosen sensor
//add number of chosen visualization type
};
extern struct NVS_Data nvs_struct;


esp_err_t nvs_load(void);
esp_err_t nvs_save(void);
esp_err_t nvs_flash_ini(void);

