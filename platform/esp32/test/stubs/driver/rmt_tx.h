#ifndef _RMT_TX_H_
#define _RMT_TX_H_
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
typedef int gpio_num_t;
typedef enum { RMT_CLK_SRC_DEFAULT = 0 } rmt_clock_source_t;
typedef struct rmt_channel_t* rmt_channel_handle_t;
typedef struct rmt_encoder_t* rmt_encoder_handle_t;
typedef struct { gpio_num_t gpio_num; rmt_clock_source_t clk_src; uint32_t resolution_hz;
                 size_t mem_block_symbols; size_t trans_queue_depth; } rmt_tx_channel_config_t;
typedef struct { uint16_t duration0:15; uint16_t level0:1; uint16_t duration1:15; uint16_t level1:1; } rmt_symbol_word_t;
typedef struct { rmt_symbol_word_t bit0; rmt_symbol_word_t bit1; struct { uint32_t msb_first:1; } flags; } rmt_bytes_encoder_config_t;
typedef struct { int loop_count; } rmt_transmit_config_t;
esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t*, rmt_channel_handle_t*);
esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t*, rmt_encoder_handle_t*);
esp_err_t rmt_enable(rmt_channel_handle_t);
esp_err_t rmt_del_channel(rmt_channel_handle_t);
esp_err_t rmt_transmit(rmt_channel_handle_t, rmt_encoder_handle_t, const void*, size_t, const rmt_transmit_config_t*);
esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t, int);
#endif
