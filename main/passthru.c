/* Audio passthru

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "audio_hal.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "board.h"
#include "passthru_encoder.h"
#include "fir_filter.h"
#include "coeffs.h"

#include "fir_coeffs_211Taps_44100_200_9000.h"
#include "fir_coeffs_361Taps_44100_200_9000.h"
static const char *TAG = "PASSTHRU";

float x[1024];
float y[1024];
float y_compare[1024];

float coeffs[361];
float delay[361];
float delay_compare[100];


void run_performance_test()
{

    int len = sizeof(x) / sizeof(float);
    int fir_len = sizeof(coeffs) / sizeof(float);
    int repeat_count = 1;

    fir_f32_t fir1;
    for (int i = 0 ; i < fir_len ; i++) {
        coeffs[i] = i;
    }

    for (int i = 0 ; i < len ; i++) {
        x[i] = 0;
    }
    x[0] = 1;

    dsps_fir_init_f32(&fir1, coeffs, delay, fir_len);

    unsigned int start_b = xthal_get_ccount();
    for (int i = 0 ; i < repeat_count ; i++) {
        dsps_fir_f32_ae32(&fir1, x, y, len);
    }
    unsigned int end_b = xthal_get_ccount();

    float total_b = end_b - start_b;
    float cycles = total_b / (len * repeat_count);

    ESP_LOGI(TAG, "dsps_fir_f32_ae32 - %f per sample for for %i coefficients, %f per tap \n", cycles, fir_len, cycles / (float)fir_len);

}

void app_main(void)
{
	run_performance_test();
}

void app_main2(void)
{
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_writer, i2s_stream_reader, passthru_encoder, fir_filter;

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    ESP_LOGI(TAG, "[ 1 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();

    // Set codec mode to "BOTH" to enabled ADC and DAC. Coded Mode line in simply
    // adds the Line In2 to the DAC bypassing the ADC

    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 2 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);

    // Needed to change i2s_stream_init function in i2s_stream.c to accept a bool
    // which tells the function whether to install the driver. Installing the driver
    // twice for the same port now causes an error (didn't before 4.4). Also in 4.4
    // dont need to call convoluted MCLK gpio function - just assign it in i2s_pins

    ESP_LOGI(TAG, "[3.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_cfg.i2s_port = I2S_NUM_0;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg, true);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to read data from codec chip");
    i2s_stream_cfg_t i2s_cfg_read = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg_read.type = AUDIO_STREAM_READER;
    i2s_cfg_read.i2s_port = I2S_NUM_0;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg_read, false);

    if ( i2s_stream_reader == NULL || i2s_stream_writer == NULL )
    	return;

    ESP_LOGI(TAG, "[3.3] Create FIR Filter");
    fir_filter_cfg_t fir_filter_cfg = DEFAULT_fir_filter_CONFIG();

/*
    fir_filter_cfg.firLen = FIR_LEN;
    fir_filter_cfg.coeffsLeft = coeffs_minus45;
    fir_filter_cfg.coeffsRight = coeffs_plus45;
*/
    fir_filter_cfg.firLen = 361;
    fir_filter_cfg.coeffsLeft = coeffs_hilbert_361Taps_44100_200_9000;
    fir_filter_cfg.coeffsRight = coeffs_delay_361;
/*
    fir_filter_cfg.firLen = 60;
    fir_filter_cfg.coeffsLeft = coeffs_60minus45;
    fir_filter_cfg.coeffsRight = coeffs_60plus45;
    fir_filter_cfg.firLen = 30;
    fir_filter_cfg.coeffsLeft = coeffs_30minus45;
    fir_filter_cfg.coeffsRight = coeffs_30plus45;
*/

    fir_filter = fir_filter_init(&fir_filter_cfg);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s_read");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s_write");
    audio_pipeline_register(pipeline, fir_filter, "fir");


    ESP_LOGI(TAG, "[3.5] Link it together [codec_chip]-->i2s_stream_reader-->i2s_stream_writer-->[codec_chip]");
/*
    const char *link_tag[2] = {"i2s_read", "i2s_write"};
    audio_pipeline_link(pipeline, &link_tag[0], 2);
*/
    const char *link_tag[3] = {"i2s_read", "fir", "i2s_write"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);


    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    ESP_LOGI(TAG, "[ 6 ] Listen for all pipeline events");

    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }

    ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(i2s_stream_writer);
}
