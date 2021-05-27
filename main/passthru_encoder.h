// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _PASSTHRU_ENCODER_H_
#define _PASSTHRU_ENCODER_H_

#include "esp_err.h"
#include "audio_element.h"

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
} passthru_encoder_cfg_t;

#define passthru_encoder_TASK_STACK          (3 * 1024)
#define passthru_encoder_TASK_CORE           (0)
#define passthru_encoder_TASK_PRIO           (5)
#define passthru_encoder_RINGBUFFER_SIZE     (8 * 1024)

#define DEFAULT_passthru_encoder_CONFIG() {\
    .out_rb_size        = passthru_encoder_RINGBUFFER_SIZE,\
    .task_stack         = passthru_encoder_TASK_STACK,\
    .task_core          = passthru_encoder_TASK_CORE,\
    .task_prio          = passthru_encoder_TASK_PRIO,\
    .stack_in_ext       = true,\
}

/**
 * @brief      Create a handle to an Audio Element to encode incoming data using WAV format
 *
 * @param      config  The configuration
 *
 * @return     The audio element handle
 */
audio_element_handle_t passthru_encoder_init(passthru_encoder_cfg_t *config);


#ifdef __cplusplus
}
#endif

#endif
