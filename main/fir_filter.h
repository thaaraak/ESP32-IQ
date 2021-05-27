// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _fir_filter_H_
#define _fir_filter_H_

#include "esp_err.h"
#include "audio_element.h"
#include "esp_dsp.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      WAV Encoder configurations
 */
typedef struct {

	int                     out_rb_size;    /*!< Size of output ringbuffer */
    int                     task_stack;     /*!< Task stack size */
    int                     task_core;      /*!< Task running in core (0 or 1) */
    int                     task_prio;      /*!< Task priority (based on freeRTOS priority) */
    bool                    stack_in_ext;   /*!< Try to allocate stack in external memory */

    int						firLen;
    float*					coeffsLeft;
    float*					coeffsRight;

} fir_filter_cfg_t;

#define fir_filter_TASK_STACK          (3 * 1024)
#define fir_filter_TASK_CORE           (0)
#define fir_filter_TASK_PRIO           (5)
#define fir_filter_RINGBUFFER_SIZE     (8 * 1024)

#define DEFAULT_fir_filter_CONFIG() {\
    .out_rb_size        = fir_filter_RINGBUFFER_SIZE,\
    .task_stack         = fir_filter_TASK_STACK,\
    .task_core          = fir_filter_TASK_CORE,\
    .task_prio          = fir_filter_TASK_PRIO,\
    .stack_in_ext       = true,\
}

/**
 * @brief      Create a handle to an Audio Element to encode incoming data using WAV format
 *
 * @param      config  The configuration
 *
 * @return     The audio element handle
 */
audio_element_handle_t fir_filter_init(fir_filter_cfg_t *config);


#ifdef __cplusplus
}
#endif

#endif
