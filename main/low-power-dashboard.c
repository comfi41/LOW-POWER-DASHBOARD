#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "epd.h"
#include "driver/uart.h"
#include "sdcard.h"

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
    const char *sdcard_file = MOUNT_POINT"/config.txt";
    sdcard_read_file(sdcard_file);

    gpio_set_direction(EINK_RESET_GPIO, GPIO_MODE_OUTPUT); //Set the GPIO as a push/pull output
    gpio_set_direction(EINK_WAKE_GPIO, GPIO_MODE_OUTPUT); //Set the GPIO as a push/pull output

    epd_init();
    epd_wakeup();
    epd_set_memory(MEM_NAND);
    base_draw();
    draw_text_demo();

    
}
