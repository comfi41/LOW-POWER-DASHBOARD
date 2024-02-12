#ifndef VISUAL_H
#define VISUAL_H

#include <string.h>
#include <stdlib.h>
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
#include "wifi_sta.h"
#include "esp_crt_bundle.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


#define ADC_BAT_MEAS ADC1_CHANNEL_4

//IN mV
#define BATTERY_FULL 4200
#define OVER_66_PER 300
#define OVER_33_PER 600
#define OVER_5_PER 800

void header(void);

void line_chart_visual(void);
void get_scale(void);
//void column_chart_visual(void);
//void scatter_plot_visual(void);

//void table_visual(void);
void value_plus_info(void);


void wifi_error(void);
void cloud_error(void);
void error(void);

void conf_mode(void);
#endif
