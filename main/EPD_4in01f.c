/*****************************************************************************
* | File      	:   EPD_4in01f.c
* | Author      :   Waveshare team
* | Function    :   4.01inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-12-29
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

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
#include "driver/spi_master.h"
#include "sdcard.h"
#include "NV_storage.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "EPD_4in01f.h"
#include "esp_task_wdt.h"



void spi_init()
{
    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num = -1,
        .mosi_io_num = MOSI_PIN,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ret  = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = CS_PIN,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi2));
};

void write_reg(uint8_t reg)
{

    uint8_t tx_data[1]={reg};
    spi_transaction_t t={
        .tx_buffer = tx_data,
        .length = 8,
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi2, &t));
      //vTaskDelay(5 / portTICK_PERIOD_MS);
}



/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
void EPD_4IN01F_Reset(void)
{
    EPD_RESET_ON();
    vTaskDelay(300 / portTICK_PERIOD_MS);
    EPD_RESET_OFF();
    vTaskDelay(300 / portTICK_PERIOD_MS);
    EPD_RESET_ON();
    vTaskDelay(300 / portTICK_PERIOD_MS);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
void EPD_4IN01F_SendCommand(uint8_t reg)
{
    EPD_DC_OFF(); 
    write_reg(reg);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
void EPD_4IN01F_SendData(uint8_t Data)
{
    
    EPD_DC_ON(); 
    write_reg(Data);
}


void EPD_4IN01F_BusyHigh(void)// If BUSYN=0 then waiting
{
    while(!(gpio_get_level(EPD_BUSY_GPIO)))
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            printf("BUsyHIGh::%d\n",gpio_get_level(EPD_BUSY_GPIO));
        }
        printf("BUsyhigh-splneno:%d\n",gpio_get_level(EPD_BUSY_GPIO));
     
}

void EPD_4IN01F_BusyLow(void)// If BUSYN=1 then waiting
{
    while(gpio_get_level(EPD_BUSY_GPIO))
         {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            printf("BUsylow:%d\n",gpio_get_level(EPD_BUSY_GPIO));
        }
        printf("BUsylow-splneno:%d\n",gpio_get_level(EPD_BUSY_GPIO));
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_4IN01F_Init(void)
{
	EPD_4IN01F_Reset();
    EPD_4IN01F_BusyHigh();
    EPD_4IN01F_SendCommand(0x00);
    EPD_4IN01F_SendData(0x2f);
    EPD_4IN01F_SendData(0x00);
    EPD_4IN01F_SendCommand(0x01);
    EPD_4IN01F_SendData(0x37);
    EPD_4IN01F_SendData(0x00);
    EPD_4IN01F_SendData(0x05);
    EPD_4IN01F_SendData(0x05);
    EPD_4IN01F_SendCommand(0x03);
    EPD_4IN01F_SendData(0x00);
    EPD_4IN01F_SendCommand(0x06);
    EPD_4IN01F_SendData(0xC7);
    EPD_4IN01F_SendData(0xC7);
    EPD_4IN01F_SendData(0x1D);
    EPD_4IN01F_SendCommand(0x41);
    EPD_4IN01F_SendData(0x00);
    EPD_4IN01F_SendCommand(0x50);
    EPD_4IN01F_SendData(0x37);
    EPD_4IN01F_SendCommand(0x60);
    EPD_4IN01F_SendData(0x22);
    EPD_4IN01F_SendCommand(0x61);
    EPD_4IN01F_SendData(0x02);
    EPD_4IN01F_SendData(0x80);
    EPD_4IN01F_SendData(0x01);
    EPD_4IN01F_SendData(0x90);
    EPD_4IN01F_SendCommand(0xE3);
    EPD_4IN01F_SendData(0xAA);
    
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/

void EPD_4IN01F_Clear(uint8_t color)
{
    EPD_4IN01F_SendCommand(0x61);//Set Resolution setting
    EPD_4IN01F_SendData(0x02);
    EPD_4IN01F_SendData(0x80);
    EPD_4IN01F_SendData(0x01);
    EPD_4IN01F_SendData(0x90);
    EPD_4IN01F_SendCommand(0x10);
    printf("bude se plnit frame buffer \n");
    for(int i=0; i<EPD_4IN01F_HEIGHT; i++) {
        for(int j=0; j<EPD_4IN01F_WIDTH/2; j++)
        {
            EPD_4IN01F_SendData((color<<4)|color);
            //vTaskDelay(10 / portTICK_PERIOD_MS);
           // printf("%d \n",j);
        }
    }
    printf("frame ok \n");
    EPD_4IN01F_SendCommand(0x04);//0x04
    EPD_4IN01F_BusyHigh();
    vTaskDelay(300 / portTICK_PERIOD_MS);
    EPD_4IN01F_SendCommand(0x12);//0x12
    vTaskDelay(300 / portTICK_PERIOD_MS);
    EPD_4IN01F_BusyHigh();
    //vTaskDelay(300 / portTICK_PERIOD_MS);
    EPD_4IN01F_SendCommand(0x02);  //0x02
    EPD_4IN01F_BusyLow();
    printf("cleared \n");
    vTaskDelay(300 / portTICK_PERIOD_MS);
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/

void EPD_4IN01F_Display(const uint8_t *image)
{
     /*
     esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 10000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,    // Bitmask of all cores
        .trigger_panic = false,
    };
    */
    //esp_task_wdt_init(&twdt_config);  
    esp_task_wdt_add(NULL); 
    uint16_t i, j, WIDTH;
	uint8_t Rdata, Rdata1, shift;
	uint16_t Addr;
	WIDTH = EPD_4IN01F_WIDTH*3%8 == 0 ? EPD_4IN01F_WIDTH*3/8 : EPD_4IN01F_WIDTH*3/8+1;
    EPD_4IN01F_SendCommand(0x61);//Set Resolution setting
    EPD_4IN01F_SendData(0x02);
    EPD_4IN01F_SendData(0x80);
    EPD_4IN01F_SendData(0x01);
    EPD_4IN01F_SendData(0x90);
    EPD_4IN01F_SendCommand(0x10);
    printf("odesilani obrazku\n");
    for(i=0; i<EPD_4IN01F_HEIGHT; i++) {
        for(j=0; j<EPD_4IN01F_WIDTH/2; j++) {
					esp_task_wdt_reset();
                    shift = (j + i*EPD_4IN01F_WIDTH/2) % 4;
					Addr = (j*3/4 + i * WIDTH);
                    //printf("shift: %d, addr: %d\n",shift,Addr);
					if(shift == 0) {
						Rdata = image[Addr];
						EPD_4IN01F_SendData(((Rdata >> 1) & 0x70) | ((Rdata >> 2) & 0x07));
					}
					else if(shift == 1) {
						Rdata = image[Addr];
						Rdata1 = image[Addr + 1];
						EPD_4IN01F_SendData(((Rdata << 5) & 0x60) | ((Rdata1 >> 3) & 0x10) | ((Rdata1 >> 4) & 0x07));
					}
					else if(shift == 2) {
						Rdata = image[Addr];
						Rdata1 = image[Addr + 1];
						EPD_4IN01F_SendData(((Rdata << 3) & 0x70) | ((Rdata << 2) & 0x04) | ((Rdata1 >> 6) & 0x03));
					}
					else if(shift == 3) {
						Rdata = image[Addr];
						EPD_4IN01F_SendData(((Rdata << 1) & 0x70) | (Rdata & 0x07));
					}
				esp_task_wdt_reset();
                }
                printf("odeslan radek obrazku: %d\n",i);
    }
    EPD_4IN01F_SendCommand(0x04);//0x04
    EPD_4IN01F_BusyHigh();
    EPD_4IN01F_SendCommand(0x12);//0x12
    EPD_4IN01F_BusyHigh();
    EPD_4IN01F_SendCommand(0x02);  //0x02
    EPD_4IN01F_BusyLow();
    vTaskDelay(200 / portTICK_PERIOD_MS);
	esp_task_wdt_deinit();
}

/******************************************************************************
function :	Sends the part image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
/*
void EPD_4IN01F_Display_part(const UBYTE *image, UWORD xstart, UWORD ystart, 
									UWORD image_width, UWORD image_heigh)
{
    unsigned long i,j;
    EPD_4IN01F_SendCommand(0x61);//Set Resolution setting
    EPD_4IN01F_SendData(0x02);
    EPD_4IN01F_SendData(0x80);
    EPD_4IN01F_SendData(0x01);
    EPD_4IN01F_SendData(0x90);
    EPD_4IN01F_SendCommand(0x10);
    for(i=0; i<EPD_4IN01F_HEIGHT; i++) {
        for(j=0; j< EPD_4IN01F_WIDTH/2; j++) {
						if(i<image_heigh+ystart && i>=ystart && j<(image_width+xstart)/2 && j>=xstart/2) {
							EPD_4IN01F_SendData(image[(j-xstart/2) + (image_width/2*(i-ystart))]);
						}
						else {
							EPD_4IN01F_SendData(0x11);
						}
				}
    }
    EPD_4IN01F_SendCommand(0x04);//0x04
    EPD_4IN01F_BusyHigh();
    EPD_4IN01F_SendCommand(0x12);//0x12
    EPD_4IN01F_BusyHigh();
    EPD_4IN01F_SendCommand(0x02);  //0x02
    EPD_4IN01F_BusyLow();
	DEV_Delay_ms(200);
	
}
*/
/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
/*
void EPD_4IN01F_Sleep(void)
{
    DEV_Delay_ms(100);
    EPD_4IN01F_SendCommand(0x07);
    EPD_4IN01F_SendData(0xA5);
}
*/
