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
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "board.h"
#include "passthru_encoder.h"
#include "fir_filter.h"

#define FIR_LEN 80

float coeffs_plus45[FIR_LEN] =
{
		 0.005196746075536660,
		 0.005206403323758748,
		 0.005143802666603999,
		 0.005275330402860660,
		 0.005807037274498531,
		 0.006624022871445243,
		 0.007252827930008205,
		 0.007160576191560499,
		 0.006229510691391205,
		 0.005037258925729325,
		 0.004629571470194255,
		 0.005813821059635204,
		 0.008414399880854059,
		 0.011091103885835485,
		 0.012033784668012429,
		 0.010231393410299029,
		 0.006480206683035897,
		 0.003282752536645955,
		 0.003427737979898337,
		 0.007991696508284392,
		 0.015114451516403511,
		 0.020640240534352677,
		 0.020591174479457360,
		 0.014095099020261598,
		 0.004758629687371951,
		-0.000844420765724989,
		 0.002864665524684386,
		 0.016304161754558792,
		 0.033221509160432171,
		 0.043463377930530872,
		 0.039086556011536947,
		 0.020461562534958601,
		-0.001617416868620743,
		-0.009981812513920364,
		 0.010013095301645547,
		 0.061223096299037758,
		 0.130436824650342564,
		 0.192224478953411160,
		 0.219775962546663733,
		 0.197593174355412821,
		 0.129520065142915336,
		 0.037760237568786860,
		-0.046841842657558544,
		-0.098422053444641180,
		-0.106820752614081027,
		-0.080297510179559717,
		-0.039460097443440943,
		-0.006159451661924554,
		 0.006644602904454021,
		-0.000860730252982563,
		-0.018277962543162267,
		-0.032531706277398730,
		-0.035340011392499829,
		-0.026687992151813410,
		-0.013137327436195776,
		-0.002820945926891691,
		-0.000506034815726877,
		-0.005499454952693672,
		-0.013078356214104860,
		-0.017984146760442160,
		-0.017575485923471849,
		-0.012858445234620491,
		-0.007179461503896226,
		-0.003790988180449273,
		-0.003950953280279233,
		-0.006588415222568388,
		-0.009427968870465266,
		-0.010601382742402151,
		-0.009668837777342310,
		-0.007551181732828482,
		-0.005650067209931823,
		-0.004880169546380925,
		-0.005236870292677496,
		-0.006045902643866292,
		-0.006571213887908395,
		-0.006499068368908933,
		-0.006010882232674052,
		-0.005501799893931320,
		-0.005231768535179391,
		-0.005175778985729014
};

float coeffs_minus45[FIR_LEN] =
{
		-0.005175778985729210,
		-0.005231768535179486,
		-0.005501799893931223,
		-0.006010882232673794,
		-0.006499068368908662,
		-0.006571213887908265,
		-0.006045902643866344,
		-0.005236870292677573,
		-0.004880169546380785,
		-0.005650067209931338,
		-0.007551181732827812,
		-0.009668837777341880,
		-0.010601382742402398,
		-0.009427968870466258,
		-0.006588415222569644,
		-0.003950953280279896,
		-0.003790988180448643,
		-0.007179461503894311,
		-0.012858445234618191,
		-0.017575485923470531,
		-0.017984146760442781,
		-0.013078356214107198,
		-0.005499454952696221,
		-0.000506034815727636,
		-0.002820945926889484,
		-0.013137327436191277,
		-0.026687992151809083,
		-0.035340011392498580,
		-0.032531706277401956,
		-0.018277962543168467,
		-0.000860730252987627,
		 0.006644602904454746,
		-0.006159451661915803,
		-0.039460097443426462,
		-0.080297510179546144,
		-0.106820752614076656,
		-0.098422053444651728,
		-0.046841842657583933,
		 0.037760237568753428,
		 0.129520065142884777,
		 0.197593174355395362,
		 0.219775962546664511,
		 0.192224478953427980,
		 0.130436824650367350,
		 0.061223096299060406,
		 0.010013095301658646,
		-0.009981812513918737,
		-0.001617416868627128,
		 0.020461562534950396,
		 0.039086556011532332,
		 0.043463377930531927,
		 0.033221509160437333,
		 0.016304161754564527,
		 0.002864665524687518,
		-0.000844420765725525,
		 0.004758629687368987,
		 0.014095099020258597,
		 0.020591174479456267,
		 0.020640240534353951,
		 0.015114451516406101,
		 0.007991696508286654,
		 0.003427737979899090,
		 0.003282752536645092,
		 0.006480206683034240,
		 0.010231393410297678,
		 0.012033784668012075,
		 0.011091103885836102,
		 0.008414399880855091,
		 0.005813821059636031,
		 0.004629571470194557,
		 0.005037258925729197,
		 0.006229510691390973,
		 0.007160576191560433,
		 0.007252827930008354,
		 0.006624022871445456,
		 0.005807037274498624,
		 0.005275330402860563,
		 0.005143802666603785,
		 0.005206403323758570,
		 0.005196746075536631
};



static const char *TAG = "PASSTHRU";

void app_main(void)
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

    ESP_LOGI(TAG, "[3.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
/*
    ESP_LOGI(TAG, "[3.2] Create Passthru Encoder");
    passthru_encoder_cfg_t passthru_cfg = DEFAULT_passthru_encoder_CONFIG();
    passthru_encoder = passthru_encoder_init(&passthru_cfg);
*/

    ESP_LOGI(TAG, "[3.2] Create FIR Filter");
    fir_filter_cfg_t fir_filter_cfg = DEFAULT_fir_filter_CONFIG();

    fir_filter_cfg.firLen = FIR_LEN;
    fir_filter_cfg.coeffsLeft = coeffs_minus45;
    fir_filter_cfg.coeffsRight = coeffs_plus45;

    fir_filter = fir_filter_init(&fir_filter_cfg);

    ESP_LOGI(TAG, "[3.3] Create i2s stream to read data from codec chip");
    i2s_stream_cfg_t i2s_cfg_read = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg_read.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg_read);


    ESP_LOGI(TAG, "[3.3] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s_read");
    audio_pipeline_register(pipeline, fir_filter, "fir");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s_write");

    ESP_LOGI(TAG, "[3.4] Link it together [codec_chip]-->i2s_stream_reader-->i2s_stream_writer-->[codec_chip]");

    const char *link_tag[3] = {"i2s_read", "fir", "i2s_write"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    //const char *link_tag[2] = {"i2s_read", "i2s_write"};
    //audio_pipeline_link(pipeline, &link_tag[0], 2);

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
