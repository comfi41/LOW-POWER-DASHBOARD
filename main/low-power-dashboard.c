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

void base_draw(void)
{
    int i, j;
    
    
    /*
    draw pixel
    */
    epd_clear();
    for(j = 0; j < 600; j += 50)
    {
        for(i = 0; i < 800; i += 50)
        {
            epd_draw_pixel(i, j);
            epd_draw_pixel(i, j + 1);
            epd_draw_pixel(i + 1, j);
            epd_draw_pixel(i + 1, j + 1);
        }
    }
    epd_udpate();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
  /*
    draw line
  */    
    epd_clear();
    for(i = 0; i < 800; i += 100)
    {
        epd_draw_line(0, 0, i, 599);
        epd_draw_line(799, 0, i, 599);
    }
    epd_udpate();
    vTaskDelay(3000 / portTICK_PERIOD_MS);    
    
    /*
    fill rect
    */
    epd_clear();
    epd_set_color(BLACK, WHITE);
    epd_fill_rect(10, 10, 100, 100);
    
    epd_set_color(DARK_GRAY, WHITE);
    epd_fill_rect(110, 10, 200, 100);
    
    epd_set_color(GRAY, WHITE);
    epd_fill_rect(210, 10, 300, 100);   
    
    epd_udpate();
    vTaskDelay(3000 / portTICK_PERIOD_MS);        
    
    /*
    draw circle
    */
    epd_set_color(BLACK, WHITE);
    epd_clear();
    for(i = 0; i < 300; i += 40)
    {
        epd_draw_circle(399, 299, i);
    }
    epd_udpate();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
    /*
    fill circle
    */
    epd_clear();
    for(j = 0; j < 6; j++)
    {
        for(i = 0; i < 8; i++)
        {
            epd_fill_circle(50 + i * 100, 50 + j * 100, 50);
        }
    }
    epd_udpate();
    vTaskDelay(3000 / portTICK_PERIOD_MS);

  /*
    draw triangle
    */
    epd_clear();
    for(i = 1; i < 5; i++)
    {
        epd_draw_triangle(399, 249 - i * 50, 349 - i * 50, 349 + i * 50, 449 + i * 50, 349 + i * 50);
    }
    epd_udpate();
    vTaskDelay(3000 / portTICK_PERIOD_MS);    
}


void draw_text_demo(void)
{
    epd_set_color(BLACK, WHITE);
    epd_clear();
    epd_set_ch_font(GBK32);
    epd_disp_string("£Ç£Â£Ë£³£²£ºÄãºÃÊÀ½ç", 0, 50);
    epd_set_ch_font(GBK48);
    epd_disp_string("£Ç£Â£Ë£´£¸£ºÄãºÃÊÀ½ç", 0, 100);
    epd_set_ch_font(GBK64);
    epd_disp_string("£Ç£Â£Ë£¶£´£ºÄãºÃÊÀ½ç", 0, 160);
    
    epd_set_en_font(ASCII32);
    epd_disp_string("ASCII32: Hello, World!", 0, 300);
    epd_set_en_font(ASCII48);
    epd_disp_string("ASCII48: Hello, World!", 0, 350);  
    epd_set_en_font(ASCII64);
    epd_disp_string("BUDU BC. TOMAS FISER!!!!", 0, 450);
    epd_udpate();   
    vTaskDelay(3000 / portTICK_PERIOD_MS);   
}


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

    vTaskDelay(2000 / portTICK_PERIOD_MS); //cas pro inicializaci disleje

    epd_init();
    epd_wakeup();
    epd_set_memory(MEM_NAND);
    EINK_WAKE_ON();
    //base_draw();
    epd_set_color(BLACK, WHITE);
    epd_clear();
    epd_set_en_font(ASCII32);
    epd_disp_string("STISKNI OK PRO ZAPNUTI WIFI", 0, 0);
    epd_udpate(); 
    //draw_text_demo();

    printf("Ini NVS: %d\n", nvs_flash_ini());
    printf("Load data: %d\n", nvs_load());
    printf("WIFI SSID:%s\n PASS:%s TIME:%d\n", nvs_struct.wifi_ssid,  nvs_struct.wifi_pass, nvs_struct.refresh_time);
   
    //printf("Save to NVS: %d\n", nvs_save());
    low_pwr_deepsleep(nvs_struct.refresh_time);
    epd_clear();
    epd_set_en_font(ASCII32);
    epd_disp_string("WIFI ZAPNUTA :)", 0, 0);
    epd_udpate(); 
    

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
