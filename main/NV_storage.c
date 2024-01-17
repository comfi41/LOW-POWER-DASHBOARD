#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "NV_storage.h"

#define STORAGE_NAMESPACE "storage"


esp_err_t nvs_load(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    err = nvs_get_blob(my_handle, "nvs_struct", NULL, &required_size); // Search and read from NVS
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
  
    err = nvs_get_blob(my_handle, "nvs_struct", (void *)&nvs_struct, &required_size);
    if (err != ESP_OK) return err;
        nvs_close(my_handle); //NUTNO VYRSIT JAK DOSTAT DATA VEN! STRUKTURA????
    return ESP_OK;
    }


esp_err_t nvs_save(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t required_size = 0; 
   
       // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;
    //Size of data
    required_size=sizeof(nvs_struct);
    err = nvs_set_blob(my_handle,"nvs_struct", (const void*)&nvs_struct,required_size);
    if (err != ESP_OK) return err;
    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;
    
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t nvs_flash_ini(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    return ESP_OK;
}
