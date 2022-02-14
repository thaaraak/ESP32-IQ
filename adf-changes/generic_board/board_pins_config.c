/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2020 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "esp_log.h"
#include "driver/gpio.h"
#include <string.h>
#include "board.h"
#include "audio_error.h"
#include "audio_mem.h"

static const char *TAG = "MY_BOARD_V1_0";

esp_err_t get_i2c_pins(i2c_port_t port, i2c_config_t *i2c_config)
{
    return ESP_OK;
}


#define I2S_BCK_IO      (GPIO_NUM_5)
#define I2S_WS_IO       (GPIO_NUM_18)
#define I2S_DO_IO       (GPIO_NUM_19)

esp_err_t get_i2s_pins(i2s_port_t port, i2s_pin_config_t *i2s_config)
{
    AUDIO_NULL_CHECK(TAG, i2s_config, return ESP_FAIL);
    if (port == I2S_NUM_0 || port == I2S_NUM_1) {
        i2s_config->mck_io_num = GPIO_NUM_0;
        i2s_config->bck_io_num = GPIO_NUM_5;
        i2s_config->ws_io_num = GPIO_NUM_18;
        i2s_config->data_out_num = GPIO_NUM_19;
        i2s_config->data_in_num = GPIO_NUM_17;
    } else {
        memset(i2s_config, -1, sizeof(i2s_pin_config_t));
        ESP_LOGE(TAG, "i2s port %d is not supported", port);
        return ESP_FAIL;
    }

    return ESP_OK;
}


esp_err_t get_spi_pins(spi_bus_config_t *spi_config, spi_device_interface_config_t *spi_device_interface_config)
{
    return ESP_OK;
}

esp_err_t i2s_mclk_gpio_select(i2s_port_t i2s_num, gpio_num_t gpio_num)
{
    return ESP_OK;
}

// sdcard

int8_t get_sdcard_intr_gpio(void)
{
    return SDCARD_INTR_GPIO;
}

int8_t get_sdcard_open_file_num_max(void)
{
    return SDCARD_OPEN_FILE_NUM_MAX;
}

int8_t get_input_volup_id(void)
{
    return BUTTON_VOLUP_ID;
}

int8_t get_input_voldown_id(void)
{
    return BUTTON_VOLDOWN_ID;
}

int8_t get_pa_enable_gpio(void)
{
    return PA_ENABLE_GPIO;
}
