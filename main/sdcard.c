/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "sdcard.h"

const char mount_point[] = MOUNT_POINT;
esp_err_t ret;
sdmmc_card_t *card;
sdmmc_host_t host = {
    .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG,
    .slot = SDSPI_DEFAULT_HOST,
    .max_freq_khz = SDMMC_FREQ_DEFAULT,
    .io_voltage = 3.3f,
    .init = &sdspi_host_init,
    .set_bus_width = NULL,
    .get_bus_width = NULL,
    .set_bus_ddr_mode = NULL,
    .set_card_clk = &sdspi_host_set_card_clk,
    .set_cclk_always_on = NULL,
    .do_transaction = &sdspi_host_do_transaction,
    .deinit_p = &sdspi_host_remove_device,
    .io_int_enable = &sdspi_host_io_int_enable,
    .io_int_wait = &sdspi_host_io_int_wait,
    .command_timeout_ms = 0,
    .get_real_freq = &sdspi_host_get_real_freq,
    .input_delay_phase = SDMMC_DELAY_PHASE_0,
    .set_input_delay = NULL,
}; 


static esp_err_t write_file(const char *path, char *data)
{
    printf("Opening file %s\n", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        printf("Failed to open file for writing\n");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);

    printf("File written\n");

    return ESP_OK;
}

void sdcard_read_file(const char *path)
{
    sdcard_SPI_init();
    read_file(path);
    sdcard_unmount();
    sdcard_SPI_deinit();
}


void read_file(const char *path)
{
    printf("Reading file %s\n", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        printf("Failed to open file for reading\n");
       
    }
    char line[MAX_FILE_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    printf("Read from file: '%s'\n", line);

}

void sdcard_SPI_init(void)
{
    printf("Inicialization SPI peripheral\n");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    sdspi_device_config_t slot_config = {
        .gpio_cs = PIN_NUM_CS,
        .host_id = host.slot,
        .gpio_cd   = SDSPI_SLOT_NO_CD,
        .gpio_wp   = SDSPI_SLOT_NO_WP,
        .gpio_int  = GPIO_NUM_NC,
        .gpio_wp_polarity = SDSPI_IO_ACTIVE_LOW,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        printf("Failed to initialize bus.\n");
        return;
    }    

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {   
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    printf("Mounting filesystem\n");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf("Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.\n");
        } else {
            printf("Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.\n", esp_err_to_name(ret));
        }
        return;
    }
    printf("Filesystem mounted\n");
    sdmmc_card_print_info(stdout, card);
}

void sdcard_SPI_deinit(void)
{
        //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
    printf("SPI free\n");

}
void sdcard_unmount(void)
{
      // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    printf("Card unmounted\n");

}
/*
const char *file_hello = MOUNT_POINT"/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);
    ret = s_example_write_file(file_hello, data);
*/

