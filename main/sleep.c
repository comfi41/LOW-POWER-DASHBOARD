#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "sleep.h"
#include "epd.h"
#include "wifi_ap.h"


void low_pwr_deepsleep(int refresh_time)
{

    int reset_enable = 1;
	

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT1: {
            
        wifi_init_softap();
        printf("Wake up from GPIO\n");
            
        reset_enable=0;
        break;
        }

        case ESP_SLEEP_WAKEUP_TIMER: {
            printf("Wake up from timer.\n");
            break;
        }

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset\n");
    }

    if(reset_enable)
    {
    const int wakeup_time_sec = refresh_time;
    printf("Enabling timer wakeup, %ds\n", wakeup_time_sec);
    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);


    const int ext_wakeup_pin_1 = GPIO_NUM_33;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;

    printf("Enabling EXT1 wakeup on pins GPIO%d\n", ext_wakeup_pin_1);
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ALL_LOW);


    rtc_gpio_isolate(GPIO_NUM_12);

    printf("Entering deep sleep\n");
    epd_enter_stopmode();
    esp_deep_sleep_start();

    vTaskDelay(1000 / portTICK_PERIOD_MS); //toto smazat, pouze pro debug
}
}
