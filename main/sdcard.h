#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"


#define MAX_FILE_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO  GPIO_NUM_2
#define PIN_NUM_MOSI  GPIO_NUM_15 
#define PIN_NUM_CLK   GPIO_NUM_14
#define PIN_NUM_CS    GPIO_NUM_13 

static esp_err_t write_file(const char *path, char *data);
void read_file(const char *path);
void sdcard_SPI_init(void);
void sdcard_SPI_deinit(void);
void sdcard_unmount(void);
void sdcard_read_file(const char *path);
