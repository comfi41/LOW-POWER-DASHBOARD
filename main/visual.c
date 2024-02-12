#include <string.h>
#include <math.h>
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
#include "visual.h"

char buff[100];
static esp_adc_cal_characteristics_t adc1_chars;
void header(void)
{
  // header visual
  // Last update info 
  sprintf(buff,"Last update:");
  epd_disp_string(buff, 10, 10);
  sprintf(buff,"01.01.2025 10:00");
  epd_disp_string(buff, 170, 10);
        
  // Battery diagram
  epd_draw_line(698, 15, 780, 15);
  epd_draw_line(698, 45, 780, 45);
        
  epd_draw_line(698, 15, 698, 45);
  epd_draw_line(780, 15, 780, 45);
        
  epd_draw_line(689, 20, 698, 20);
  epd_draw_line(689, 40, 698, 40);
        
  epd_draw_line(689, 20, 689, 40);
        
  //ADC init
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
  adc1_config_channel_atten(ADC_BAT_MEAS, ADC_ATTEN_DB_11);
        
  double voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC_BAT_MEAS), &adc1_chars);
  voltage=voltage*2;
  // 100 % - 66 % battery level
  if ((BATTERY_FULL - voltage) <= OVER_66_PER) {
    epd_fill_rect(702, 18, 724, 41);
    epd_fill_rect(728, 18, 750, 41);
    epd_fill_rect(754, 18, 776, 41);
  }
  // 65 % - 33 % battery level
  else if ((BATTERY_FULL - voltage) <= OVER_33_PER) {
    epd_fill_rect(728, 18, 750, 41);
    epd_fill_rect(754, 18, 776, 41);
  }
  // 32 % - cca 5 % battery level
  else if ((BATTERY_FULL - voltage) <= OVER_5_PER) {
    epd_fill_rect(754, 18, 776, 41);
  }
  // Very low battery level
  else {
    epd_set_color(WHITE, BLACK);
    epd_draw_line(725, 15, 752, 15);
    epd_draw_line(730, 45, 747, 45);
    epd_set_color(BLACK, WHITE);
    epd_draw_triangle(730, 10, 747, 10, 738, 47);
    epd_fill_rect(737, 51, 740, 54);
  }
}

void wifi_error(void)
{
  epd_set_en_font(ASCII64);
  sprintf(buff,"Wi-Fi");
  epd_disp_string(buff, 330, 200);
  error();
}

void cloud_error(void){
  epd_set_en_font(ASCII64);
  sprintf(buff,"CLOUD");
  epd_disp_string(buff, 318, 200);
  error();
}

void error(void)
{
  epd_set_en_font(ASCII48);
  sprintf(buff,"ERROR");
  epd_disp_string(buff, 341, 270);
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff,"Press");
  epd_disp_string(buff, 240, 420);
  epd_set_color(BLACK, WHITE);
  sprintf(buff,"'OK'");
  epd_disp_string(buff, 320, 420);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff,"for configuration");
  epd_disp_string(buff, 380, 420);
}

void conf_mode(void)
{
  epd_set_en_font(ASCII64);
  sprintf(buff,"CONFIGURATION");
  epd_disp_string(buff, 195, 120);
  sprintf(buff,"MODE");
  epd_disp_string(buff, 335, 190);
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff,"AP SSID:");
  epd_disp_string(buff, 345, 330);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff,"Low_power_dashboard");
  epd_disp_string(buff, 265, 370);
  
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff,"Configurator address:");
  epd_disp_string(buff, 275, 430);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff,"192.168.4.1");
  epd_disp_string(buff, 330, 470);
}


void value_plus_info(void)
{
  char value[] = "-8";
  int value_length = strlen(value);
  double spec_char = 0.0;
  for (int i = 0; value[i] != '\0'; i++)
  {
    if (value[i] == '.')
    {
      spec_char -= 18.666;
    }
    if (value[i] == '-'){
      spec_char += 3.0;
    }
    if (value[i] == '1'){
      spec_char -= 9.33;
    }
  }
  if (value_length > 5)
  {
    epd_set_en_font(ASCII64);
    sprintf(buff, "OUT OF RANGE");
    epd_disp_string(buff, 50, 80);
  }
  else 
  {
    epd_set_en_font(ASCII64);
    sprintf(buff, "%s", value);
    epd_disp_string(buff, 100, 80);
    epd_set_en_font(ASCII32);
    sprintf(buff, "o");
    int position = 110 + (value_length * 28) + spec_char;
    epd_disp_string(buff, position, 77);
    
    epd_set_en_font(ASCII48);
    sprintf(buff, "C");
    epd_disp_string(buff, position + 19, 80);
  }
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff, "Device ID:");
  epd_disp_string(buff, 400, 80);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff, "123456789");
  epd_disp_string(buff, 528, 80);
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff, "Device name:");
  epd_disp_string(buff, 400, 110);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff, "senzor 1");
  epd_disp_string(buff, 568.327, 110); 
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff, "Battery:");
  epd_disp_string(buff, 400, 140);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff, "50 %%");
  epd_disp_string(buff, 499.9065, 140);
}

void line_chart_visual(void)
{
  //double values[] = {-7.5, -11.1, 5.2, 8.0, 42}; //test values
  //double values[] = {1.2, 4.3, 5.2, 8.0, 2.6};
  //double values[] = {-1.2, -4.3, -5.2, -8.0};
  double values[] = {-1.2, -4.3, -5.2, -8.0, 11.1, 5.2, 8.0, 42, 1.2, 4.3};
  int length = sizeof(values) / sizeof(values[0]);
  Helper helper_struct = get_scale(values, length);
  double X_scale = 670 / (length - 1);
  epd_draw_line(70, 540, 70, 550);
  for (int i = 0; i < length; i++)
  {
    epd_draw_line(70 + ((i + 1) * X_scale), 540, 70 + ((i + 1) * X_scale), 550);
    switch(helper_struct.type)
    {
      case 1:
        if ((length - 1) == i)
        {
          epd_set_color(BLACK, WHITE);
          sprintf(buff, "%.1f", values[i]);
          epd_disp_string(buff, 70 + (i * X_scale) - 16, 395 - (values[i] / helper_struct.scale) - 45);
          epd_fill_circle((70 + (i * X_scale)), 395 - (values[i] / helper_struct.scale), 8);
        }
        else
        {
          epd_set_color(BLACK, WHITE);
          sprintf(buff, "%.1f", values[i]);
          if (i == 0)
          {
            epd_disp_string(buff, 70 + (i * X_scale) + 7, 395 - (values[i] / helper_struct.scale) - 45);
          }
          else
          {
            epd_disp_string(buff, 70 + (i * X_scale) - 16, 395 - (values[i] / helper_struct.scale) - 45);
          }
          epd_draw_line(70 + (i * X_scale), 395 - (values[i] / helper_struct.scale), 70 + ((i + 1) * X_scale), 395 - (values[i+1] / helper_struct.scale));
          epd_fill_circle((70 + (i * X_scale)), 395 - (values[i] / helper_struct.scale), 8);
        }
        break;
      case 2:
        if ((length - 1) == i)
        {
          epd_set_color(BLACK, WHITE);
          sprintf(buff, "%.1f", values[i]);
          epd_disp_string(buff, 70 + (i * X_scale) - 16, 545 - (values[i] / helper_struct.scale) - 45);
          epd_fill_circle((70 + (i * X_scale)), 545 - (values[i] / helper_struct.scale), 8);
        }
        else
        {
          epd_set_color(BLACK, WHITE);
          sprintf(buff, "%.1f", values[i]);
          if (i == 0)
          {
            epd_disp_string(buff, 70 + (i * X_scale) + 7, 545 - (values[i] / helper_struct.scale) - 45);
          }
          else
          {
            epd_disp_string(buff, 70 + (i * X_scale) - 16, 545 - (values[i] / helper_struct.scale) - 45);
          }
          epd_draw_line(70 + (i * X_scale), 545 - (values[i] / helper_struct.scale), 70 + ((i + 1) * X_scale), 545 - (values[i+1] / helper_struct.scale));
          epd_fill_circle((70 + (i * X_scale)), 545 - (values[i] / helper_struct.scale), 8);
        }
        break;
      case 3:
        if ((length - 1) == i)
        {
          epd_set_color(BLACK, WHITE);
          sprintf(buff, "%.1f", values[i]);
          epd_disp_string(buff, 70 + (i * X_scale) - 16, 245 + (values[i] / helper_struct.scale) - 45);
          epd_fill_circle((70 + (i * X_scale)), 245 + (values[i] / helper_struct.scale), 8);
        }
        else
        {
          epd_set_color(BLACK, WHITE);
          sprintf(buff, "%.1f", values[i]);
          if (i == 0)
          {
            epd_disp_string(buff, 70 + (i * X_scale) + 7, 245 + (values[i] / helper_struct.scale) - 45);
          }
          else
          {
            epd_disp_string(buff, 70 + (i * X_scale) - 16, 245 + (values[i] / helper_struct.scale) - 45);
          }
          epd_draw_line(70 + (i * X_scale), 245 + (values[i] / helper_struct.scale), 70 + ((i + 1) * X_scale), 245 + (values[i+1] / helper_struct.scale));
          epd_fill_circle((70 + (i * X_scale)), 245 + (values[i] / helper_struct.scale), 8);
        }
        break;
    }
  }
}


Helper get_scale(double values[], int length){
  Helper helper_struct;
  double max = values[0];
  double min = values[0];
  bool zero_midd = false;
  bool zero_bottom = false;
  bool zero_top = false;
  double step = 0.0;
  double diff = 0.0;
  double scale_value = 0.0;
  for (int i = 0; i < length; i++)
  {
    if (max < values[i])
    {
      max = values[i];
    }
    if (min > values[i])
    {
      min = values[i];
    }
  }
  if (min > 0.0)
  {
    zero_bottom = true;
    helper_struct.type = 2;
    step = fabs(max / 8);
    diff = fabs(max);
  }
  else
  {
    if (max < 0.0)
    {
      zero_top = true;
      helper_struct.type = 3;
      step = fabs(min / 8);
      diff = min;
    }
    else
    {
      zero_midd = true;
      helper_struct.type = 1;
      if (fabs(max) > fabs(min))
      {
        step = max / 4;
        diff = max * 2;
      }
      else
      {
        step = fabs(min / 4);
        diff = fabs(min * 2);
      }
    }
  }
  epd_fill_rect(70, 545, 740, 546);
  epd_fill_rect(70, 545, 71, 245);
  
  for (int i = 0; i < 9; i++) 
  {
    epd_draw_line(65, 545 - i * 37.5, 75, 545 - i * 37.5);
    if (zero_midd)
    {
      if (i == 4)
      {
        sprintf(buff,"0");
        epd_disp_string(buff, 35, 531 - i * 37.5);
      }
      else
      {
        scale_value = step * (-4 + i);
        sprintf(buff,"%.1f", scale_value);
        if (scale_value < 0.0)
        {
          epd_disp_string(buff, 0, 531 - i * 37.5);
        }
        else
        {
          epd_disp_string(buff, 6, 531 - i * 37.5);
        }
      }
      
    }
    else if(zero_bottom)
    {
      if (i == 0)
      {
        sprintf(buff,"0");
        epd_disp_string(buff, 35, 531);
      }
      else
      {
        scale_value = step * i;
        sprintf(buff,"%.1f", scale_value);
        epd_disp_string(buff, 6, 531 - i * 37.5);
      }
    }
    else if(zero_top)
    {
      if (i == 8)
      {
        sprintf(buff,"0");
        epd_disp_string(buff, 35, 231);
      }
      else
      {
        scale_value = -step * (8 - i);
        sprintf(buff,"%.1f", scale_value);
        epd_disp_string(buff, 0, 531 - i * 37.5);
      }
    }
  }
  helper_struct.scale = diff/300;
  return helper_struct;
}
