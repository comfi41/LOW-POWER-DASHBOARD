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
#include "data_processing.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"
#include "esp_sntp.h"
int noOfRecords;
void temporary_structure_initializer(void)
{

  noOfRecords=10; //vytvorime 10 group struktur a 10 senzor struktur nahrani


  group_struct = (struct Group_Data *)malloc(noOfRecords * sizeof(struct Group_Data));
  sensor_struct = (struct Sensor_Data *)malloc(noOfRecords * sizeof(struct Sensor_Data));
  history_struct = (struct Sensor_history_Data *)malloc(noOfRecords * sizeof(struct Sensor_history_Data));
  for (int i = 0; i < noOfRecords; i++) 
  {
    (group_struct+i)->group_id=i;
    sprintf((group_struct+i)->name,"Francouska Grupa:%d",i);
    (group_struct+i)->number_of_sensors=i;
    for (int x = 0; x < i; x++)
    	(group_struct+i)->sensors_id[x]=x;
  }

for (int i = 0; i < noOfRecords; i++) 
  {
    (sensor_struct+i)->sensor_id=i;
    sprintf((sensor_struct+i)->name,"Super Sensor:%d",i);
    (sensor_struct+i)->number_of_params=i;
    for (int x = 0; x < i; x++)
    {
    	sprintf((sensor_struct+i)->name_params[x],"Alkohol:%d",i);
    	(sensor_struct+i)->value_params[x]=x*5*i;
    	sprintf((sensor_struct+i)->unit_params[x],"lahvace");
    }
  }

  for (int i = 0; i < noOfRecords; i++) 
  {
    (history_struct+i)->sensor_id=i;
    (history_struct+i)->number_of_records=i;

    for (int x = 0; x < i; x++)
    {
    	(history_struct+i)->values[x]=x*2*i;
    	(history_struct+i)->time[x]=x;
    	sprintf((history_struct+i)->name_param,"Teplota c.:%d",i);
    	sprintf((history_struct+i)->unit_param,"˚C");
    }	
  }



  printf("\nDisplaying data from structures GROUP:\n");
  for (int i = 0; i < noOfRecords; i++) {
    printf("ID grupy:%d\nNazev rupy:%s\nPocet senzoru:%d\nID senzoru:", (group_struct+i)->group_id, (group_struct+i)->name, (group_struct+i)->number_of_sensors);
  for (int x = 0; x < (group_struct+i)->number_of_sensors; x++) printf("%d,",(group_struct+i)->sensors_id[x]);
  	printf("\n");
  }

  printf("\nDisplaying data from structures SENSOR:\n");
  for (int i = 0; i < noOfRecords; i++) {
    printf("ID senzoru:%d\nNazev senzoru:%s\nPocet parametru:%d\n", (sensor_struct+i)->sensor_id, (sensor_struct+i)->name, (sensor_struct+i)->number_of_params);
  printf("Data z cidel:\n"); 
  for (int x = 0; x < (sensor_struct+i)->number_of_params; x++) printf("Parametr:%d, Nazev veliciny: %s, Hodnota: %d, Jednotka: %s\n",x,(sensor_struct+i)->name_params[x],(sensor_struct+i)->value_params[x],(sensor_struct+i)->unit_params[x]);	
  }

    printf("\nDisplaying data from structures HISTORY:\n");
  for (int i = 0; i < noOfRecords; i++) {
    printf("ID senzoru:%d\nDelka zaznamu:%d\n", (history_struct+i)->sensor_id, (history_struct+i)->number_of_records);
  printf("Data zaznam dat c.:%d\n",i); 
  for (int x = 0; x < (history_struct+i)->number_of_records; x++) printf("Cas:%d, Hodnota: %d\n",(history_struct+i)->time[x],(history_struct+i)->values[x]);	
  if((history_struct+i)->number_of_records) printf("Nazev veliciny: %s, Jednotka: %s\n",(history_struct+i)->name_param,(history_struct+i)->unit_param);
  }
  
}

void parser(char *input_buffer)
{
	int number;
	float number_float;
	char buffer[250];

				printf( "INPUT: %s\n", input_buffer);

	            for (char *p = strtok(input_buffer,","); p != NULL; p = strtok(NULL, ","))
            	{
            		//parsovani tokenu
            	    //printf( "parse: %s\n", p );
                	if ((sscanf(p, "{\"access_token\":\"%s",token))&&(parsing_pointer==TOKEN)) 
                	{
                    	token[strlen(token)-1] = '\0';
                    	printf("TOKEN VYPARSOVAN: %s\n", token);
                	}
                	
                	//parsovani poctu grup a senzoru
                	if((*p >= '0' && *p <= '9')&&(parsing_pointer=GET_NUMBER_DEVS))
                	{
  						if (sscanf(p, "%d", &number) == 1) printf("VYPARSOVANO CISLO: %d\n", number);
                	}

                	//parsovani ID a nazvu grup
                	if (((sscanf(p, "{\"id\":%d",&number))||(sscanf(p, "[{\"id\":%d",&number)))&&(parsing_pointer==GET_GROUPS)) printf("VYPARSOVANO ID GRUPY: %d\n", number);
                	if (sscanf(p, "\"name\":\"%[^\t\n]",buffer))
                	{
                		buffer[strlen(buffer)-1] = '\0';
                    	printf("VYPARSOVANO NAME GRUPY: %s\n", buffer);
                	}

                	//parsovani dat ze senzoru
                	if (((sscanf(p, "{\"id\":%d",&number))||(sscanf(p, "[{\"id\":%d",&number)))&&(parsing_pointer==GET_SENSORS_VALUES)) printf("VYPARSOVANO ID SENZORU: %d\n", number);
                	if (sscanf(p, "\"deviceName\":\"%[^\t\n]",buffer))
                	{
                		buffer[strlen(buffer)-1] = '\0';
                    	printf("VYPARSOVANO DEVICE NAME: %s\n", buffer);
                	}
                	if (sscanf(p, "\"label\":\"%[^\t\n]",buffer))
                	{
                		buffer[strlen(buffer)-1] = '\0';
                    	printf("VYPARSOVANO LABEL OF PARAM: %s\n", buffer);
                	}
                	if (sscanf(p, "\"value\":%f",&number_float)) printf("VYPARSOVANO VALUE: %.2f\n", number_float);
                	
                	if (sscanf(p, "\"unit\":\"%[^\t\n]",buffer))
                	{
                		buffer[strlen(buffer)-1] = '\0';
                    	printf("VYPARSOVANO UNIT OF PARAM: %s\n", buffer);
                	}
                	
                }
}
