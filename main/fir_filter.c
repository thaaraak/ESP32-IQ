// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#include "esp_log.h"
#include "audio_mem.h"
#include "audio_element.h"
#include "fir_filter.h"
#include "wav_head.h"
#include "audio_error.h"

static const char *TAG = "fir_filter";


typedef struct fir_filter {

    fir_f32_t 	firLeft;
    fir_f32_t 	firRight;
    float*		srcLeft;
    float*		srcRight;
    float*		destLeft;
    float*		destRight;
    float*		delayLeft;
    float*		delayRight;

    int			firLen;
    float*		coeffsLeft;
    float*		coeffsRight;

} fir_filter_t;

static esp_err_t _fir_filter_destroy(audio_element_handle_t self)
{
    fir_filter_t *fir = (fir_filter_t *)audio_element_getdata(self);

    audio_free( fir->delayLeft );
    audio_free( fir->delayRight );

    audio_free( fir->srcLeft );
    audio_free( fir->srcRight );
    audio_free( fir->destLeft );
    audio_free( fir->destRight );

    audio_free(fir);
    return ESP_OK;
}
static esp_err_t _fir_filter_open(audio_element_handle_t self)
{
    ESP_LOGD(TAG, "_fir_filter_open");
    return ESP_OK;
}

static esp_err_t _fir_filter_close(audio_element_handle_t self)
{
    ESP_LOGD(TAG, "_fir_filter_close");
    if (AEL_STATE_PAUSED != audio_element_get_state(self)) {
        audio_element_set_byte_pos(self, 0);
        audio_element_set_total_bytes(self, 0);
    }
    return ESP_OK;
}

int printed = 0;

static void _fir_filter( audio_element_handle_t self, int16_t* arr, int len )
{
    fir_filter_t *fir = (fir_filter_t *)audio_element_getdata(self);

    for ( int i = 0 ; i < len ; i += 2 )
    {
    	fir->srcLeft[i/2] = arr[i];
    	fir->srcRight[i/2] = arr[i+1];
    }

    dsps_fir_f32_ae32(&(fir->firLeft), fir->srcLeft, fir->destLeft, len/2);
    dsps_fir_f32_ae32(&(fir->firRight), fir->srcRight, fir->destRight, len/2);

    for ( int i = 0 ; i < len ; i += 2 )
    {
    	arr[i] = fir->destLeft[i/2];
    	arr[i+1] = fir->destRight[i/2];
    }

}

void _fir_filter2( int16_t* arr, int len )
{
	if ( printed++ % 800 == 0 )
	{
		for ( int i = 0 ; i < len ; i++ )
			printf( "%d\n", arr[i] );
	}
}


static int _fir_filter_process(audio_element_handle_t self, char *in_buffer, int in_len)
{
    int r_size = audio_element_input(self, in_buffer, in_len);

    int out_len = r_size;
    if (r_size > 0) {

    	_fir_filter( self, (int16_t*)in_buffer, r_size/2 );

    	out_len = audio_element_output(self, in_buffer, r_size);
        if (out_len > 0) {
            audio_element_update_byte_pos(self, out_len);
        }
    }

    return out_len;
}

audio_element_handle_t fir_filter_init(fir_filter_cfg_t *config)
{
    fir_filter_t *fir = audio_calloc(1, sizeof(fir_filter_t));
    AUDIO_MEM_CHECK(TAG, fir, {return NULL;});

    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.destroy = _fir_filter_destroy;
    cfg.process = _fir_filter_process;
    cfg.open = _fir_filter_open;
    cfg.close = _fir_filter_close;
    cfg.task_stack = fir_filter_TASK_STACK;

    if (config) {
        if (config->task_stack) {
            cfg.task_stack = config->task_stack;
        }
        cfg.stack_in_ext = config->stack_in_ext;
        cfg.task_prio = config->task_prio;
        cfg.task_core = config->task_core;
        cfg.out_rb_size = config->out_rb_size;
    }

    cfg.tag = "fir";

    int n = cfg.buffer_len/4;

    fir->firLen = config->firLen;
    fir->coeffsLeft = config->coeffsLeft;
    fir->coeffsRight = config->coeffsRight;
    fir->delayLeft = audio_calloc( fir->firLen, sizeof(float) );
    fir->delayRight = audio_calloc( fir->firLen, sizeof(float) );

    fir->srcLeft = audio_calloc( n, sizeof(float) );
    fir->srcRight = audio_calloc( n, sizeof(float) );
	fir->destLeft = audio_calloc( n, sizeof(float) );
	fir->destRight = audio_calloc( n, sizeof(float) );

    dsps_fir_init_f32(&(fir->firLeft), fir->coeffsLeft, fir->delayLeft, fir->firLen);
    dsps_fir_init_f32(&(fir->firRight), fir->coeffsRight, fir->delayRight, fir->firLen);

    audio_element_handle_t el = audio_element_init(&cfg);
    AUDIO_MEM_CHECK(TAG, el, {audio_free(fir); return NULL;});
    audio_element_setdata(el, fir);
    ESP_LOGD(TAG, "fir_filter_init");
    return el;
}
