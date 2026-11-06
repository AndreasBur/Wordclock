/*
 * What Pixels::Render() puts on the wire, checked byte for byte against a captured
 * transmission. The RMT calls are stubbed here rather than in stubs.cpp because this test
 * has to keep the frame the driver handed over - the shared stub throws it away.
 */
#include <driver/rmt_tx.h>

#include "Arduino.h"
#include "Pixels.h"
#include "check.h"

#include <cstdio>
#include <cstring>

static byte captured[512];
static size_t captured_size = 0;

esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t*, rmt_channel_handle_t* h) { *h = reinterpret_cast<rmt_channel_handle_t>(1); return ESP_OK; }
esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t*, rmt_encoder_handle_t* h) { *h = reinterpret_cast<rmt_encoder_handle_t>(1); return ESP_OK; }
esp_err_t rmt_enable(rmt_channel_handle_t) { return ESP_OK; }
esp_err_t rmt_del_channel(rmt_channel_handle_t) { return ESP_OK; }
esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t, int) { return ESP_OK; }
esp_err_t rmt_transmit(rmt_channel_handle_t, rmt_encoder_handle_t, const void* d, size_t n, const rmt_transmit_config_t*)
{
    captured_size = n;
    memcpy(captured, d, n);
    return ESP_OK;
}

int main()
{
    Pixels& p = Pixels::GetInstance();
    p.Init(10);

    p.ClearPixels();
    p.SetPixel(0u, 10u, 20u, 30u);      // Red=10, Green=20, Blue=30
    p.SetPixel(12u, 1u, 2u, 3u);        // row 1, column 1 -> byte offset 36
    p.Render();

    Check(captured_size == 330u, "frame is 110 * 3 bytes");
    Check(captured[0] == 20u && captured[1] == 10u && captured[2] == 30u, "pixel 0 goes out as green, red, blue");
    Check(captured[36] == 2u && captured[37] == 1u && captured[38] == 3u, "pixel 12 goes out as green, red, blue at offset 36");
    Check(!p.IsDirty(), "render clears the dirty flag");

    captured_size = 0;
    p.Render();
    Check(captured_size == 0u, "an unchanged frame is not retransmitted");

    p.DisablePixels();
    p.Render();
    bool all_dark = true;
    for(size_t i = 0; i < captured_size; i++) { if(captured[i] != 0u) all_dark = false; }
    Check(all_dark, "disablePixels blanks the frame");

    p.EnablePixels();
    p.Render();
    Check(captured[0] == 20u && captured[1] == 10u && captured[2] == 30u, "enablePixels restores it without a redraw");
    return Report();
}
