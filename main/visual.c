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
#include "NV_storage.h"

extern char deviceName[50];
extern char groupName[50];
extern double actual_value;
extern double *device_values;
extern int values_count;
extern char unit[50];
extern char param[50];


char buff[100];
static esp_adc_cal_characteristics_t adc1_chars;
void header(void)
{
  // header visual
  // Last update info 
  sprintf(buff,"Last update:");
  epd_disp_string(buff, 10, 10);
  sprintf(buff,"%s", nvs_struct.last_update);
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

void new_conf(void)
{
  epd_set_en_font(ASCII64);
  sprintf(buff,"NEW CONFIGURATION");
  epd_disp_string(buff, 140, 150);
  sprintf(buff,"SAVED");
  epd_disp_string(buff, 330, 220);
  
  sprintf(buff,"Rebooting...");
  epd_disp_string(buff, 270, 380);
}

void value_plus_info(void)
{
  char value[50];
  sprintf(value, "%.1f", actual_value);
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
    epd_disp_string(buff, 100, 70);
    if (strcmp(unit, "˚C") == 0)
    {
      epd_set_en_font(ASCII32);
      sprintf(buff, "o");
      int position = 110 + (value_length * 28) + spec_char;
      epd_disp_string(buff, position, 67);
      
      epd_set_en_font(ASCII48);
      sprintf(buff, "C");
      epd_disp_string(buff, position + 19, 70);
    }
    else
    {
      epd_set_en_font(ASCII64);
      sprintf(buff, "%s", unit);
      int position = 110 + (value_length * 28) + spec_char;
      epd_disp_string(buff, position, 67);
    }
  }
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff, "Device:");
  epd_disp_string(buff, 400, 60);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff, "%s", deviceName);
  epd_disp_string(buff, 500, 60);
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff, "Group:");
  epd_disp_string(buff, 400, 90);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff, "%s", groupName);
  epd_disp_string(buff, 490, 90); 
  
  epd_set_en_font(ASCII32);
  epd_set_color(DARK_GRAY, WHITE);
  sprintf(buff, "Parameter:");
  epd_disp_string(buff, 400, 120);
  
  epd_set_color(BLACK, WHITE);
  sprintf(buff, "%s",param);
  epd_disp_string(buff, 529.9065, 120);
}

//Function that creates and renders line chart
void line_chart_visual(void)
{
  value_plus_info();
  
  //double values[] = {-7.5, -11.1, 5.2, 8.0, 42}; //test values
  //double values[] = {1.2, 4.3, 5.2, 8.0, 1.2};
  //double values[] = {-1.2, -4.3, -5.2, -8.0, -1.2};
  double values[values_count];
  memcpy(values, device_values, values_count * sizeof(double));
  free(device_values);
  char *X_values[][MAX_DATETIME_LENGTH] = {{"01.01.2024", "10:00"}, {"01.01.2024", "12:00"}, {"01.01.2024", "14:00"}, {"01.01.2024", "16:00"}, {"01.01.2024", "18:00"}, {"01.01.2024", "20:00"}};
  
  //char *X_values[][MAX_DATETIME_LENGTH] = {{"01.01.2024", "10:00"}, {"01.01.2024", "12:00"}, {"01.01.2024", "14:00"}, {"01.01.2024", "16:00"}, {"01.01.2024", "18:00"}, {"01.01.2024", "20:00"}, {"01.01.2024", "22:00"}, {"02.01.2024", "00:00"}, {"02.01.2024", "02:00"}, {"02.01.2024", "04:00"}};
  //double values[] = {-1.2, -4.3, -5.2, -8.0, 11.1, 5.2, 8.0, 42, 1.2, 4.3};
  
  int X_value_length = sizeof(X_values) / sizeof(X_values[0]);
  int length = sizeof(values) / sizeof(values[0]);
  
  if (X_value_length == length)
  {
    //chart header showing the time range from which the data is 
    char unique_X_values[X_value_length][MAX_DATE_LENGTH];
    int unique_values_count = 1;
    strcpy(unique_X_values[0], X_values[0][0]);
     
    for (int i = 1; i < X_value_length; i++) 
    {
      if (strcmp(X_values[i][0], X_values[i - 1][0]) != 0) 
      {
        strcpy(unique_X_values[unique_values_count], X_values[i][0]);
        unique_values_count++;
      }
    }
      
    if (unique_values_count >= 2)
    {
      epd_draw_line(0, 172, 455, 172);
      sprintf(buff, "Data from: %s -> %s", unique_X_values[0], unique_X_values[unique_values_count-1]);
      epd_disp_string(buff, 0, 175);
    }
    else
    {
      epd_draw_line(0, 172, 279, 172);
      sprintf(buff, "Data from: %s", unique_X_values[0]);
      epd_disp_string(buff, 0, 175);
    }
    Helper helper_struct = get_scale(values, length, LINE_CHART);
    
    double X_scale = 670 / (length - 1);
    epd_draw_line(70, 540, 70, 550);
    
    for (int i = 0; i < length; i++)
    {
      epd_draw_line(70 + ((i + 1) * X_scale), 540, 70 + ((i + 1) * X_scale), 550);
      sprintf(buff, "%s", X_values[i][1]);
      epd_disp_string(buff, 40 + ((i) * X_scale), 560);    
      switch(helper_struct.type)
      {
        case 1:
          if ((length - 1) == i)
          {
            epd_set_color(BLACK, WHITE);
            sprintf(buff, "%.1f", values[i]);
            epd_disp_string(buff, 70 + (i * X_scale) - 16, 395 - (values[i] / helper_struct.scale) - 45);
            epd_fill_circle((70 + (i * X_scale)), 395 - (values[i] / helper_struct.scale), 4);
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
            epd_fill_circle((70 + (i * X_scale)), 395 - (values[i] / helper_struct.scale), 4);
          }
          break;
        case 2:
          if ((length - 1) == i)
          {
            epd_set_color(BLACK, WHITE);
            sprintf(buff, "%.1f", values[i]);
            epd_disp_string(buff, 70 + (i * X_scale) - 16, 545 - (values[i] / helper_struct.scale) - 45);
            epd_fill_circle((70 + (i * X_scale)), 545 - (values[i] / helper_struct.scale), 4);
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
            epd_fill_circle((70 + (i * X_scale)), 545 - (values[i] / helper_struct.scale), 4);
          }
          break;
        case 3:
          if ((length - 1) == i)
          {
            epd_set_color(BLACK, WHITE);
            sprintf(buff, "%.1f", values[i]);
            epd_disp_string(buff, 70 + (i * X_scale) - 16, 245 + (values[i] / helper_struct.scale) - 45);
            epd_fill_circle((70 + (i * X_scale)), 245 + (values[i] / helper_struct.scale), 4);
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
            epd_fill_circle((70 + (i * X_scale)), 245 + (values[i] / helper_struct.scale), 4);
          }
          break;
      }
    }
  }
  else
  {
    epd_set_en_font(ASCII64);
    sprintf(buff, "INVALID DATA INPUT!");
    epd_disp_string(buff, 134, 300);
  }
}

//Function that creates and renders column chart
void column_chart_visual(void)
{
  value_plus_info();
  int temp;
  double avg_of_dim;

  double values[][3] = {{-7.5, -11.1}, {5.2}, {42.0, 42.0, 42.0}, {-7.5, -11.1}, {1.0, 2.8}, {8.3, 3.2}, {-20.0}, {18.3, 4.8}, {3.2}, {6.4}, {19.3}, {-2.0}};
  //double values[][3] = {{7.5, 11.1}, {5.2}, {42.0, 42.0, 42.0}, {7.5, 11.1}, {8.3, 3.2}, {-20.0}, {18.3, 4.8}};
  //double values[][3] = {{-7.5, -11.1}, {-5.2}, {-42.0, -42.0, -42.0}, {-7.5, -11.1}};
  
  //char *X_values[] = {"Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat.", "Sun."};
  char *X_values[] = {"Jan.", "Feb.", "Mar.", "Apr.", "May.", "Jun.", "Jul.", "Aug.", "Sep.", "Oct.", "Nov.", "Dec."};

  int X_value_length = sizeof(X_values) / sizeof(X_values[0]);
  int length = sizeof(values) / sizeof(values[0]);
  int second_dim_len = sizeof(values[0]) / sizeof(values[0][0]);
  double avg_values[length];
  
  if (X_value_length == length)
  {
    for (int i = 0; i < length; i++) {
      avg_of_dim = 0.0;
      temp = second_dim_len;
      for (int j = 0; j < second_dim_len; j++) {
          if (values[i][j] != 0)
          {
            avg_of_dim += values[i][j];
          }
          else
          {
            temp -= 1;
          }
      }
      avg_values[i] = (avg_of_dim / temp);
    }
    length = sizeof(avg_values) / sizeof(avg_values[0]);
    Helper helper_struct = get_scale(avg_values, length, COLUMN_CHART);
    
    double X_scale = 690 / (length + 1);
    for (int i = 0; i < length; i++)
    {
      
      epd_draw_line(60 + X_scale + ((i) * X_scale), 540, 60 + X_scale + ((i) * X_scale), 550);
      sprintf(buff, "%s", X_values[i]);
      epd_disp_string(buff, 40 + X_scale + ((i) * X_scale), 560);
      
      switch(helper_struct.type)
      {
          case 1:
            sprintf(buff, "%.1f", avg_values[i]);
            if (avg_values[i] > 0 && avg_values[i] < 10)
            {
              epd_disp_string(buff, 40 + X_scale + ((i) * X_scale), 350 - (avg_values[i] / helper_struct.scale));
            }
            else if (avg_values[i] <= -10)
            {
              epd_disp_string(buff, 30 + X_scale + ((i) * X_scale), 350 - (avg_values[i] / helper_struct.scale));
            }
            else
            {
              epd_disp_string(buff, 33 + X_scale + ((i) * X_scale), 350 - (avg_values[i] / helper_struct.scale));
            }
            epd_fill_rect(45 + X_scale + ((i) * X_scale), 545, 75 + X_scale + ((i) * X_scale), 395 - (avg_values[i] / helper_struct.scale));
            break;
          default:
            sprintf(buff, "%.1f", avg_values[i]);
            if (avg_values[i] > 0 && avg_values[i] < 10)
            {
              epd_disp_string(buff, 40 + X_scale + ((i) * X_scale), 500 - (avg_values[i] / helper_struct.scale));
            }
            else if (avg_values[i] <= -10)
            {
              epd_disp_string(buff, 30 + X_scale + ((i) * X_scale), 500 - (avg_values[i] / helper_struct.scale));
            }
            else
            {
              epd_disp_string(buff, 33 + X_scale + ((i) * X_scale), 500 - (avg_values[i] / helper_struct.scale));
            }
            epd_fill_rect(45 + X_scale + ((i) * X_scale), 545, 75 + X_scale + ((i) * X_scale), 545 - (avg_values[i] / helper_struct.scale));
      }
    }
  }
  else
  {
    epd_set_en_font(ASCII64);
    sprintf(buff, "INVALID DATA INPUT!");
    epd_disp_string(buff, 134, 300);
  }
}

void scatter_plot_visual(void)
{
  value_plus_info();
  //double values[] = {-7.5, -11.1, 5.2, 8.0, 42}; //test values
  //double values[] = {1.2, 4.3, 5.2, 8.0, 1.2};
  char *values[][3] = {{"Data Set 1", "Data Set 2", "Data Set 3"},{"3.2", "-7.5", "-20.1"}, {"1.2", "8.3", "-5.2"}, {"-10.0", "10.0", "-42.0"}, {"7.5", "-2.1",  "-11.1"}};
  //char *values[][3] = {{"Data Set 1", "Data Set 2", "Data Set 3"},{NULL, "7.5", "20.1"}, {NULL, NULL, "5.2"}, {"10.0", "10.0", "42.0"}, {"7.5", NULL,  "11.1"}};
  
  //char *values[][3] = {{"Data Set 1", "Data Set 2", "Data Set 3"},{NULL, "-7.5", "-20.1"}, {NULL, NULL, "-5.2"}, {"-10.0", "-10.0", "-42.0"}, {"-7.5", NULL,  "-11.1"}};
  
  char *X_values[][MAX_DATETIME_LENGTH] = {{"01.01.2024", "10:00"}, {"01.01.2024", "12:00"}, {"01.01.2024", "14:00"}, {"01.01.2024", "16:00"}};
  
  
  int X_value_length = sizeof(X_values) / sizeof(X_values[0]); //count of X scale values
  int length = sizeof(values) / sizeof(values[0]); //length of input data
  
  int second_dim_len = 0;
  
  //start of chart header showing the time range from which the data is
  char unique_X_values[X_value_length][MAX_DATE_LENGTH];
  int unique_values_count = 1;
  strcpy(unique_X_values[0], X_values[0][0]);
  
  for (int i = 1; i < X_value_length; i++) 
  {
    if (strcmp(X_values[i][0], X_values[i - 1][0]) != 0) 
    {
      strcpy(unique_X_values[unique_values_count], X_values[i][0]);
      unique_values_count++;
    }
  }
  
  if (unique_values_count >= 2)
  {
    epd_draw_line(0, 172, 455, 172);
    sprintf(buff, "Data from: %s -> %s", unique_X_values[0], unique_X_values[unique_values_count-1]);
    epd_disp_string(buff, 0, 175);
  }
  else
  {
    epd_draw_line(0, 172, 279, 172);
    sprintf(buff, "Data from: %s", unique_X_values[0]);
    epd_disp_string(buff, 0, 175);
  }
  
  if (values[0][0] == NULL && values[0][1] == NULL && values[0][2] == NULL)
  {
    second_dim_len = 0;
  }
  else if (values[0][1] == NULL && values[0][2] == NULL)
  {
      epd_set_color(BLACK, WHITE);
      epd_fill_circle(300, 225, 8);
      sprintf(buff, "- %s", values[0][0]);
      epd_disp_string(buff, 325, 205);
      second_dim_len = 1;
  }
  else if (values[0][2] == NULL)
  {
      epd_set_color(BLACK, WHITE);
      epd_fill_circle(200, 220, 8);
      sprintf(buff, "- %s", values[0][0]);
      epd_disp_string(buff, 225, 205);
      
      epd_set_color(DARK_GRAY, WHITE);
      epd_fill_circle(400, 220, 8);
      epd_set_color(WHITE, BLACK);
      epd_fill_circle(400, 220, 4);
      epd_set_color(BLACK, WHITE);
      sprintf(buff, "- %s", values[0][1]);
      epd_disp_string(buff, 425, 205);
      
      second_dim_len = 2;
  }
  else
  {
      epd_set_color(BLACK, WHITE);
      epd_fill_circle(100, 220, 8);
      sprintf(buff, "- %s", values[0][0]);
      epd_disp_string(buff, 125, 205);
      
      epd_set_color(DARK_GRAY, WHITE);
      epd_fill_circle(300, 220, 8);
      epd_set_color(WHITE, BLACK);
      epd_fill_circle(300, 220, 4);
      epd_set_color(BLACK, WHITE);
      sprintf(buff, "- %s", values[0][1]);
      epd_disp_string(buff, 325, 205);
      
      epd_set_color(GRAY, WHITE);
      epd_fill_circle(500, 220, 8);
      epd_set_color(BLACK, WHITE);
      sprintf(buff, "- %s", values[0][2]);
      epd_disp_string(buff, 525, 205);
      
      second_dim_len = 3;
  }
  //end of chart header
  
  
  if (X_value_length == (length - 1))
  {
    double min = 10000.0;
    double max = -10000.0;
    for (int i = 1; i < length; i++) {
      for (int j = 0; j < second_dim_len; j++) {
        if (values[i][j] != NULL)
        {
          if (atof(values[i][j]) < min)
          {
            min = atof(values[i][j]);
          }
          if (atof(values[i][j]) > max)
          {
            max = atof(values[i][j]);
          }
        }
      }
    }
    double min_max[] = {max, min};
    
    Helper helper_struct = get_scale(min_max, 2, LINE_CHART);
    
    double X_scale = 670 / (length - 2);
    epd_draw_line(70, 540, 70, 550);
    
    for (int i = 1; i < length; i++)
    {
      epd_set_color(BLACK, WHITE);
      epd_draw_line(70 + ((i - 1) * X_scale), 540, 70 + ((i - 1) * X_scale), 550);
      sprintf(buff, "%s", X_values[i-1][1]);
      epd_disp_string(buff, 40 + ((i - 1) * X_scale), 560); 
      for (int j = 0; j < second_dim_len; j++) {
        if (values[i][j] != NULL)
        {
          switch(helper_struct.type)
          {
            case 1:
              if ((length - 1) == i)
              {
                sprintf(buff, "%.1f", atof(values[i][j]));
                switch(j) //TENTO SWITCH JAKO FCE, TAKTO JE TO HROZNEJ SHIT CODE :)
                {
                  case 0:
                    epd_set_color(BLACK, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 8);   
                    break;
                  case 1:
                    epd_set_color(DARK_GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 8);
                    epd_set_color(WHITE, BLACK);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 4); 
                    break;
                  case 2:
                    epd_set_color(GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 8);                    
                    break;
                }
                
                epd_set_color(BLACK, WHITE);
                if (atof(values[i][j]) == max && max >= fabs(min))
                {
                  epd_disp_string(buff, ((i - 1) * X_scale), 435 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
                else
                {
                  epd_disp_string(buff, ((i - 1) * X_scale), 415 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
              }
              else
              {
                sprintf(buff, "%.1f", atof(values[i][j]));
                switch(j)
                {
                  case 0:
                    epd_set_color(BLACK, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 8);   
                    break;
                  case 1:
                    epd_set_color(DARK_GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 8);
                    epd_set_color(WHITE, BLACK);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 4); 
                    break;
                  case 2:
                    epd_set_color(GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 395 - (atof(values[i][j]) / helper_struct.scale), 8);                    
                    break;
                }
                
                epd_set_color(BLACK, WHITE);
                if (atof(values[i][j]) == max && max >= fabs(min))
                {
                  epd_disp_string(buff, 80 + ((i - 1) * X_scale), 435 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
                else
                {
                  epd_disp_string(buff, 80 + ((i - 1) * X_scale), 415 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
              }
              break;
            case 2:
              if ((length - 1) == i)
              {
                sprintf(buff, "%.1f", atof(values[i][j]));
                switch(j)
                {
                  case 0:
                    epd_set_color(BLACK, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 8);   
                    break;
                  case 1:
                    epd_set_color(DARK_GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 8);
                    epd_set_color(WHITE, BLACK);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 4); 
                    break;
                  case 2:
                    epd_set_color(GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 8);                    
                    break;
                }
                
                epd_set_color(BLACK, WHITE);
                if (atof(values[i][j]) == max && max >= fabs(min))
                {
                  epd_disp_string(buff, ((i - 1) * X_scale), 585 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
                else
                {
                  epd_disp_string(buff, ((i - 1) * X_scale), 565 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
              }
              else
              {
                sprintf(buff, "%.1f", atof(values[i][j]));
               switch(j)
                {
                  case 0:
                    epd_set_color(BLACK, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 8);   
                    break;
                  case 1:
                    epd_set_color(DARK_GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 8);
                    epd_set_color(WHITE, BLACK);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 4); 
                    break;
                  case 2:
                    epd_set_color(GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 545 - (atof(values[i][j]) / helper_struct.scale), 8);                    
                    break;
                }
                
                epd_set_color(BLACK, WHITE);
                if (atof(values[i][j]) == max && max >= fabs(min))
                {
                  epd_disp_string(buff, 80 + ((i - 1) * X_scale), 585 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
                else
                {
                  epd_disp_string(buff, 80 + ((i - 1) * X_scale), 565 - (atof(values[i][j]) / helper_struct.scale) - 45);
                }
              }
              break;
            case 3:
              if ((length - 1) == i)
              {
                sprintf(buff, "%.1f", atof(values[i][j]));
                switch(j)
                {
                  case 0:
                    epd_set_color(BLACK, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 8);   
                    break;
                  case 1:
                    epd_set_color(DARK_GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 8);
                    epd_set_color(WHITE, BLACK);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 4); 
                    break;
                  case 2:
                    epd_set_color(GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 8);                    
                    break;
                }
                
                epd_set_color(BLACK, WHITE);
                if (atof(values[i][j]) == max && max >= fabs(min))
                {
                  epd_disp_string(buff, ((i - 1) * X_scale), 285 + (atof(values[i][j]) / helper_struct.scale) - 45);
                }
                else
                {
                  epd_disp_string(buff, ((i - 1) * X_scale), 265 + (atof(values[i][j]) / helper_struct.scale) - 45);
                }
              }
              else
              {
                sprintf(buff, "%.1f", atof(values[i][j]));
                switch(j)
                {
                  case 0:
                    epd_set_color(BLACK, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 8);   
                    break;
                  case 1:
                    epd_set_color(DARK_GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 8);
                    epd_set_color(WHITE, BLACK);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 4); 
                    break;
                  case 2:
                    epd_set_color(GRAY, WHITE);
                    epd_fill_circle((70 + ((i - 1) * X_scale)), 245 + (atof(values[i][j]) / helper_struct.scale), 8);                    
                    break;
                }
                
                epd_set_color(BLACK, WHITE);
                if (atof(values[i][j]) == max && max >= fabs(min))
                {
                  epd_disp_string(buff, 80 + ((i - 1) * X_scale), 285 + (atof(values[i][j]) / helper_struct.scale) - 45);
                }
                else
                {
                  epd_disp_string(buff, 80 + ((i - 1) * X_scale), 265 + (atof(values[i][j]) / helper_struct.scale) - 45);
                }
              }
              break;
          }
        }
      }
    } 
  }
  else
  {
    epd_set_en_font(ASCII64);
    sprintf(buff, "INVALID DATA INPUT!");
    epd_disp_string(buff, 134, 300);
  }  
}

void table_visual(void)
{ 
  char *values[][20] = {{"Dataaaaaa1", "Dataaaaaa2Dataaaaaa2Dataaaaaa2Dataaaaaa2Dataaaaaa2", "Data1"},{NULL, "-7.5", "-20.1"}, {NULL, NULL, "-5.2"}, {"-10.0", "10.0", "-42.0"}, {"7.5", NULL,  "-11.1"}};
  //char *values[][3] = {{"Data Set 1", "Data Set 2", "Data Set 3"},{NULL, "7.5", "20.1"}, {NULL, NULL, "5.2"}, {"10.0", "10.0", "42.0"}, {"7.5", NULL,  "11.1"}};
  
  //char *values[][3] = {{"Data Set 1", "Data Set 2", "Data Set 3"},{NULL, "-7.5", "-20.1"}, {NULL, NULL, "-5.2"}, {"-10.0", "-10.0", "-42.0"}, {"-7.5", NULL,  "-11.1"}};
  
  char *Table_header[] = {"Monsadddddddddd.", "Tue.", "Wsssssssssssssssssssssssssssssssssssssed.", "Thu."};
  
  int Header_length = sizeof(Table_header) / sizeof(Table_header[0]); //count of X scale values
  int length = sizeof(values) / sizeof(values[0]); //length of input data
  int second_dim_len = (sizeof(values[0]) / sizeof(values[0][0]));
  
  double cell_height, cell_width = 0.0;
  if (Header_length == (length - 1))
  {
    for (int i = 0; i < (sizeof(values[0]) / sizeof(values[0][0])); i++){
      if (values[0][i] == NULL)
      {
        second_dim_len -= 1;
      }
    }
    
    cell_height = TABLE_HEIGHT / (second_dim_len + 1);
    cell_width = TABLE_WIDTH / length;
    
    if (TABLE_HEIGHT > 400)
    {
      //default table dimensions X: 50 -> 750, Y: 100 -> 550 (700x450)
      epd_fill_rect((800 - TABLE_WIDTH) / 2, 100, 800 - ((800 - TABLE_WIDTH) / 2), 102); //table top line
      epd_fill_rect((800 - TABLE_WIDTH) / 2, 100, ((800 - TABLE_WIDTH) / 2) + 2, 600 - (600 - TABLE_HEIGHT) + 100); //table left line
      epd_fill_rect((800 - TABLE_WIDTH) / 2, 600 - (600 - TABLE_HEIGHT) + 100, 800 - ((800 - TABLE_WIDTH) / 2), 600 - (600 - TABLE_HEIGHT) + 102); //table bottom line
      epd_fill_rect(800 - ((800 - TABLE_WIDTH) / 2), 100, 800 - ((800 - TABLE_WIDTH) / 2) + 2, 600 - (600 - TABLE_HEIGHT) + 100); //table right line
      
      for (int i = 1; i < (second_dim_len + 1); i++)
      {
        if (i == 1)
        {
          epd_fill_rect((800 - TABLE_WIDTH) / 2, 100 + (i * cell_height), 800 - ((800 - TABLE_WIDTH) / 2), 102 + (i * cell_height));
        }
        else
        {
          epd_draw_line((800 - TABLE_WIDTH) / 2, 100 + (i * cell_height), 800 - ((800 - TABLE_WIDTH) / 2), 100 + (i * cell_height));
        }
      }
      
      for (int i = 1; i < length; i++)
      {
        if (i == 1)
        {
          epd_fill_rect(((800 - TABLE_WIDTH) / 2) + (i * cell_width), 100, ((800 - TABLE_WIDTH) / 2) + (i * cell_width) + 2, 600 - (600 - TABLE_HEIGHT) + 100);
        }
        else
        {
          epd_draw_line(((800 - TABLE_WIDTH) / 2) + (i * cell_width), 100, ((800 - TABLE_WIDTH) / 2) + (i * cell_width), 600 - (600 - TABLE_HEIGHT) + 100);
        }
      }
      
      int chars_on_line = (int) round((cell_width - 10) / SMALLEST_CHAR_WIDTH); //pocet znaku, co se vejde do jedne bunky
      int lines_in_cell = (int) round((cell_height - 10) / SMALLEST_CHAR_HEIGHT); //max pocet radku v jedne bunce
      
      printf("Pocet znaku na radku: %d \n", chars_on_line);
      printf("Pocet radku: %d \n", lines_in_cell);
      
      char *string = "";
      char *header_string = "";
      double X,Y,Y_header = 0.0;
      int str_len = 0;
      int required_lines;
      for (int i = 0; i < length; i++)
      {
        X = ((800 - TABLE_WIDTH) / 2) + (i * cell_width);
        Y = 0.0;
        Y_header = 0.0;
        for (int j = 0; j < second_dim_len; j++)
        {
          Y = 100 + ((j + 1) * cell_height);
          Y_header = 100 + (j * cell_height);
          str_len = 0;
          if (values[i][j] == NULL)
          {
            string = "No data";
          }
          else
          {
            string = values[i][j];
          }
          double LR_border_width = 0.0; //pro vystredeni -> sirka leveho a praveho borderu
          double TB_border_height = 0.0;//pro vystredeni -> vyska od topu a bottomu
          
          if (j == 0 && i >= 1) // subrule listing the header elements 
          {
            header_string = Table_header[i - 1];
            str_len = strlen(header_string);
            required_lines = (int) round(str_len / chars_on_line) + 1;
            int lines_done = 1;
            int chars_done = 0;
            if (required_lines == 1)
            {
              LR_border_width = (cell_width - (str_len * SMALLEST_CHAR_WIDTH)) / 2;
              TB_border_height = (cell_height - SMALLEST_CHAR_HEIGHT) / 2;
              
              sprintf(buff, "%s", header_string);
              epd_disp_string(buff, X + LR_border_width, Y_header + TB_border_height);
            }
            else if (required_lines > lines_in_cell)
            {
              TB_border_height = 5;
              
              required_lines = lines_in_cell;
              str_len = required_lines * chars_on_line;
              
              print_chars(header_string, TB_border_height, X, Y_header, lines_done, required_lines, str_len, chars_done, chars_on_line);
            }
            else
            {
              TB_border_height = (cell_height - (required_lines * SMALLEST_CHAR_HEIGHT)) / 2;
              print_chars(header_string, TB_border_height, X, Y_header, lines_done, required_lines, str_len, chars_done, chars_on_line);
            }
          }
          
          str_len = strlen(string); //delka konkretniho stringu
          required_lines = (int) round(str_len / chars_on_line) + 1; //na kolik je radku
          printf("Pocet radku: %d, X: %d Y: %d \n", required_lines, i, j);
          int lines_done = 1;
          int chars_done = 0;
          if (required_lines == 1)
          {
            LR_border_width = (cell_width - (str_len * SMALLEST_CHAR_WIDTH)) / 2;
            TB_border_height = (cell_height - SMALLEST_CHAR_HEIGHT) / 2;
            
            sprintf(buff, "%s", string);
            epd_disp_string(buff, X + LR_border_width, Y + TB_border_height);
          }
          else if (required_lines > lines_in_cell)
          {
            TB_border_height = 5;
            
            required_lines = lines_in_cell;
            str_len = required_lines * chars_on_line;
            
            print_chars(string, TB_border_height, X, Y, lines_done, required_lines, str_len, chars_done, chars_on_line);
          }
          else
          {
            TB_border_height = (cell_height - (required_lines * SMALLEST_CHAR_HEIGHT)) / 2;
            print_chars(string, TB_border_height, X, Y, lines_done, required_lines, str_len, chars_done, chars_on_line);
          }
        }
      }
    }
    else //table auto centering with different dimensions
    {
      epd_fill_rect((800 - TABLE_WIDTH) / 2, (600 - TABLE_HEIGHT) / 2, 800 - ((800 - TABLE_WIDTH) / 2), ((600 - TABLE_HEIGHT) / 2) + 2); //table top line
      epd_fill_rect((800 - TABLE_WIDTH) / 2, (600 - TABLE_HEIGHT) / 2, ((800 - TABLE_WIDTH) / 2) + 2, 600 - (600 - TABLE_HEIGHT) + ((600 - TABLE_HEIGHT) / 2)); //table left line
      epd_fill_rect((800 - TABLE_WIDTH) / 2, 600 - (600 - TABLE_HEIGHT) + ((600 - TABLE_HEIGHT) / 2), 800 - ((800 - TABLE_WIDTH) / 2), 600 - (600 - TABLE_HEIGHT) + ((600 - TABLE_HEIGHT) / 2) + 2); //table bottom line
      epd_fill_rect(800 - ((800 - TABLE_WIDTH) / 2), (600 - TABLE_HEIGHT) / 2, 800 - ((800 - TABLE_WIDTH) / 2) + 2, 600 - (600 - TABLE_HEIGHT) + ((600 - TABLE_HEIGHT) / 2)); //table right line
      
      for (int i = 1; i < (second_dim_len + 1); i++)
      {
        if (i == 1)
        {
          epd_fill_rect((800 - TABLE_WIDTH) / 2, ((600 - TABLE_HEIGHT) / 2) + (i * cell_height), 800 - ((800 - TABLE_WIDTH) / 2), ((600 - TABLE_HEIGHT) / 2) + (i * cell_height) + 2);
        }
        else
        {
          epd_draw_line((800 - TABLE_WIDTH) / 2, ((600 - TABLE_HEIGHT) / 2) + (i * cell_height), 800 - ((800 - TABLE_WIDTH) / 2), ((600 - TABLE_HEIGHT) / 2) + (i * cell_height));
        }
      }
      
      for (int i = 1; i < length; i++)
      {
        if (i == 1)
        {
          epd_fill_rect(((800 - TABLE_WIDTH) / 2) + (i * cell_width), (600 - TABLE_HEIGHT) / 2, ((800 - TABLE_WIDTH) / 2) + (i * cell_width) + 2, 600 - (600 - TABLE_HEIGHT) + ((600 - TABLE_HEIGHT) / 2));
        }
        else
        {
          epd_draw_line(((800 - TABLE_WIDTH) / 2) + (i * cell_width), (600 - TABLE_HEIGHT) / 2, ((800 - TABLE_WIDTH) / 2) + (i * cell_width), 600 - (600 - TABLE_HEIGHT) + ((600 - TABLE_HEIGHT) / 2));
        }
      }
      int chars_on_line = (int) round((cell_width - 10) / SMALLEST_CHAR_WIDTH); //pocet znaku, co se vejde do jedne bunky
      int lines_in_cell = (int) round((cell_height - 10) / SMALLEST_CHAR_HEIGHT); //max pocet radku v jedne bunce
      
      printf("Pocet znaku na radku: %d \n", chars_on_line);
      printf("Pocet radku: %d \n", lines_in_cell);
      
      char *string = "";
      char *header_string = "";
      double X,Y,Y_header = 0.0;
      int str_len = 0;
      int required_lines;
      for (int i = 0; i < length; i++)
      {
        X = ((800 - TABLE_WIDTH) / 2) + (i * cell_width);
        Y = 0.0;
        Y_header = 0.0;
        for (int j = 0; j < second_dim_len; j++)
        {
          Y = ((600 - TABLE_HEIGHT) / 2) + ((j + 1) * cell_height);
          Y_header = ((600 - TABLE_HEIGHT) / 2) + (j * cell_height);
          str_len = 0;
          if (values[i][j] == NULL)
          {
            string = "No data";
          }
          else
          {
            string = values[i][j];
          }
          double LR_border_width = 0.0; //pro vystredeni -> sirka leveho a praveho borderu
          double TB_border_height = 0.0;//pro vystredeni -> vyska od topu a bottomu
          
          if (j == 0 && i >= 1) // subrule listing the header elements 
          {
            header_string = Table_header[i - 1];
            str_len = strlen(header_string);
            required_lines = (int) round(str_len / chars_on_line) + 1;
            int lines_done = 1;
            int chars_done = 0;
            if (required_lines == 1)
            {
              LR_border_width = (cell_width - (str_len * SMALLEST_CHAR_WIDTH)) / 2;
              TB_border_height = (cell_height - SMALLEST_CHAR_HEIGHT) / 2;
              
              sprintf(buff, "%s", header_string);
              epd_disp_string(buff, X + LR_border_width, Y_header + TB_border_height);
            }
            else if (required_lines > lines_in_cell)
            {
              TB_border_height = 5;
              
              required_lines = lines_in_cell;
              str_len = required_lines * chars_on_line;
              
              print_chars(header_string, TB_border_height, X, Y_header, lines_done, required_lines, str_len, chars_done, chars_on_line);
            }
            else
            {
              TB_border_height = (cell_height - (required_lines * SMALLEST_CHAR_HEIGHT)) / 2;
              print_chars(header_string, TB_border_height, X, Y_header, lines_done, required_lines, str_len, chars_done, chars_on_line);
            }
          }
          
          str_len = strlen(string); //delka konkretniho stringu
          required_lines = (int) round(str_len / chars_on_line) + 1; //na kolik je radku
          printf("Pocet radku: %d, X: %d Y: %d \n", required_lines, i, j);
          int lines_done = 1;
          int chars_done = 0;
          if (required_lines == 1)
          {
            LR_border_width = (cell_width - (str_len * SMALLEST_CHAR_WIDTH)) / 2;
            TB_border_height = (cell_height - SMALLEST_CHAR_HEIGHT) / 2;
            
            sprintf(buff, "%s", string);
            epd_disp_string(buff, X + LR_border_width, Y + TB_border_height);
          }
          else if (required_lines > lines_in_cell)
          {
            TB_border_height = 5;
            
            required_lines = lines_in_cell;
            str_len = required_lines * chars_on_line;
            
            print_chars(string, TB_border_height, X, Y, lines_done, required_lines, str_len, chars_done, chars_on_line);
          }
          else
          {
            TB_border_height = (cell_height - (required_lines * SMALLEST_CHAR_HEIGHT)) / 2;
            print_chars(string, TB_border_height, X, Y, lines_done, required_lines, str_len, chars_done, chars_on_line);
          }
        }
      }
    }
  }
  else
  {
    epd_set_en_font(ASCII64);
    sprintf(buff, "INVALID DATA INPUT!");
    epd_disp_string(buff, 134, 300);
  }
}

void print_chars(char *string, double distance_from_top, double X, double Y, int lines_done, int required_lines, int str_len, int chars_done, int chars_on_line)
{
  printf("Here 2\n");
  printf("Done: %d\n", chars_done);
  printf("Required lines: %d\n", required_lines);
  printf("Done lines: %d\n", lines_done);
  if (lines_done == required_lines)
  {
    printf("Here 3\n");
    chars_on_line = str_len - chars_done;
    sprintf(buff, "%.*s", chars_on_line, string + chars_done);
    epd_disp_string(buff, X + 5, Y + distance_from_top);
  }
  else
  {
    printf("Here 4\n");
    if (chars_done == 0)
    {
      printf("Here 5\n");
      sprintf(buff, "%.*s", chars_on_line, string);
      epd_disp_string(buff, X + 5, Y + distance_from_top);
    }
    else
    {
      printf("Here 6\n");
      //printf("String: %s\n", string);
      //printf("Line limit: %d\n", chars_on_line);
      //printf("Done: %d\n", chars_done);
      //printf("X: %f\n", X + 5);
      //printf("Y: %f\n", Y);
      //printf("From top: %f\n", distance_from_top);
      //sprintf(buff, "%.*s", chars_on_line, string + chars_done);
      epd_disp_string(buff, X + 5, Y + distance_from_top);
    }      
    lines_done += 1;
    chars_done += chars_on_line;
    distance_from_top += SMALLEST_CHAR_HEIGHT;
            
    print_chars(string, distance_from_top, X, Y, lines_done, required_lines, str_len, chars_done, chars_on_line);
  }
}

Helper get_scale(double values[], int length, int chart_type){
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
      if(chart_type == 2)
      {
        if (i == 0)
        {
          sprintf(buff,"0");
          epd_disp_string(buff, 35, 531);
        }
        else
        {
          scale_value = -step * i;
          sprintf(buff,"%.1f", scale_value);
          epd_disp_string(buff, 0, 531 - i * 37.5);
        }
      }
      else
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
  }
  helper_struct.scale = diff/300;
  return helper_struct;
}
