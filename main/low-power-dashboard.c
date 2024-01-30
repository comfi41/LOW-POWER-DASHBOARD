#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "epd.h"
#include "driver/uart.h"
#include "driver/spi_master.h"
#include "sdcard.h"
#include "NV_storage.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "sleep.h"
#include "wifi_ap.h"
#include "wifi_sta.h"



    #define BUTT_OK GPIO_NUM_33
    #define BUTT_UP GPIO_NUM_22
    #define BUTT_DOWN GPIO_NUM_23
    #define CHRG_PG GPIO_NUM_18
    #define CHRG_S1 GPIO_NUM_19
    #define CHRG_S2 GPIO_NUM_35
    #define ADC_BAT_MEAS ADC1_CHANNEL_4

  
    
    
    static esp_adc_cal_characteristics_t adc1_chars;
    struct NVS_Data nvs_struct;

    spi_device_handle_t spi2;


void app_main()
{
    char buff[30];
    uint32_t voltage;
    
    //const char *sdcard_file = MOUNT_POINT"/config.txt";
    //sdcard_read_file(sdcard_file);

    

    gpio_set_direction(BUTT_UP, GPIO_MODE_INPUT); //Set the GPIO as a push/pull output
    gpio_set_pull_mode(BUTT_UP, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTT_DOWN, GPIO_MODE_INPUT); //Set the GPIO as a push/pull output
    gpio_set_pull_mode(BUTT_DOWN, GPIO_PULLUP_ONLY);
    gpio_set_direction(BUTT_OK, GPIO_MODE_INPUT); //Set the GPIO as a push/pull output
    gpio_set_pull_mode(BUTT_OK, GPIO_PULLUP_ONLY);
    gpio_set_direction(CHRG_PG, GPIO_MODE_INPUT); //Set the GPIO as a push/pull output
    gpio_set_pull_mode(CHRG_PG, GPIO_PULLUP_ONLY);
    gpio_set_direction(CHRG_S1, GPIO_MODE_INPUT); //Set the GPIO as a push/pull output
    gpio_set_pull_mode(CHRG_S1, GPIO_PULLUP_ONLY);
    gpio_set_direction(CHRG_S2, GPIO_MODE_INPUT); //Set the GPIO as a push/pull output
    gpio_set_pull_mode(CHRG_S2, GPIO_PULLUP_ONLY);

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC_BAT_MEAS, ADC_ATTEN_DB_11);

    epd_init();
    epd_wakeup();
    epd_set_memory(MEM_NAND);
    EINK_WAKE_ON();
    //base_draw();
    epd_set_color(BLACK, WHITE);
    epd_clear();
    epd_set_en_font(ASCII32);
    //draw_text_demo();

    printf("Ini NVS: %d\n", nvs_flash_ini());
    printf("Load data: %d\n", nvs_load());
    printf("WIFI SSID:%s\n PASS:%s TIME:%d\n", nvs_struct.wifi_ssid,  nvs_struct.wifi_pass, nvs_struct.refresh_time);
   
    //printf("Save to NVS: %d\n", nvs_save());
    low_pwr_deepsleep(nvs_struct.refresh_time);
    

//while(1)
//{
  /*  EINK_WAKE_ON();
    epd_clear();
    epd_set_en_font(ASCII32);
    epd_disp_string("BUTTON STATUS:", 0, 0);
    sprintf(buff,"UP:%d",gpio_get_level(BUTT_UP));
    epd_disp_string(buff, 0, 50);
    sprintf(buff,"DOWN:%d",gpio_get_level(BUTT_DOWN));
    epd_disp_string(buff, 0, 100);
    sprintf(buff,"OK:%d",gpio_get_level(BUTT_OK));
    epd_disp_string(buff, 0, 150);

    epd_disp_string("CHARGER STATUS:", 400, 0);
    sprintf(buff,"PG:%d",gpio_get_level(CHRG_PG));
    epd_disp_string(buff, 400, 50);
    sprintf(buff,"S1:%d",gpio_get_level(CHRG_S1));
    epd_disp_string(buff, 400, 100);
    sprintf(buff,"S2:%d",gpio_get_level(CHRG_S2));
    epd_disp_string(buff, 400, 150);
    
    voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC_BAT_MEAS), &adc1_chars);
    voltage=voltage*2;
    sprintf(buff,"BATTERY VOLTAGE: %d mV",(int)voltage);
    epd_disp_string(buff, 0, 250);
    epd_udpate();   
    EINK_WAKE_ON();
    vTaskDelay(50000 / portTICK_PERIOD_MS);   
    */
// }   
}
