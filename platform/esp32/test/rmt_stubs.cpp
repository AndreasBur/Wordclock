/* The RMT calls, for the tests and the host process that do not inspect the frame.
   frame_test.cpp defines its own instead, so it can keep what was transmitted. */
#include <driver/rmt_tx.h>

esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t*, rmt_channel_handle_t* h) { *h = (rmt_channel_handle_t)1; return ESP_OK; }
esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t*, rmt_encoder_handle_t* h) { *h = (rmt_encoder_handle_t)1; return ESP_OK; }
esp_err_t rmt_enable(rmt_channel_handle_t) { return ESP_OK; }
esp_err_t rmt_del_channel(rmt_channel_handle_t) { return ESP_OK; }
esp_err_t rmt_transmit(rmt_channel_handle_t, rmt_encoder_handle_t, const void*, size_t, const rmt_transmit_config_t*) { return ESP_OK; }
esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t, int) { return ESP_OK; }
