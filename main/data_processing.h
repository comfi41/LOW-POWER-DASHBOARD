#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

struct Group_Data{
int group_id;
char name[50];
int number_of_sensors;
int sensors_id[50];
};

struct Sensor_Data{
int sensor_id;
char name[50];
int number_of_params;
char name_params[20][50];
int value_params[20];
char unit_params[20][50];
};

struct Sensor_history_Data{
int sensor_id;
int number_of_records;
int values[50];
int time[50]; //zatim fakt nevim
char name_param[50];
char unit_param[50];
};
	void temporary_structure_initializer(void);

	extern struct Group_Data *group_struct;
    extern struct Sensor_Data *sensor_struct;
    extern struct Sensor_history_Data *history_struct;
