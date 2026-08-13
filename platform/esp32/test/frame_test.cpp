/* What Pixels::render() puts on the wire, checked byte for byte against a captured
   transmission. The RMT calls are stubbed here rather than in stubs.cpp because this test
   has to keep the frame the driver handed over - the shared stub throws it away. */
#include <driver/rmt_tx.h>

#include "Arduino.h"
#include "Pixels.h"

#include <cstdio>
#include <cstring>

static byte Captured[512];
static size_t CapturedSize = 0;

esp_err_t rmt_new_tx_channel(const rmt_tx_channel_config_t*, rmt_channel_handle_t* h) { *h = (rmt_channel_handle_t)1; return ESP_OK; }
esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t*, rmt_encoder_handle_t* h) { *h = (rmt_encoder_handle_t)1; return ESP_OK; }
esp_err_t rmt_enable(rmt_channel_handle_t) { return ESP_OK; }
esp_err_t rmt_del_channel(rmt_channel_handle_t) { return ESP_OK; }
esp_err_t rmt_tx_wait_all_done(rmt_channel_handle_t, int) { return ESP_OK; }
esp_err_t rmt_transmit(rmt_channel_handle_t, rmt_encoder_handle_t, const void* d, size_t n, const rmt_transmit_config_t*)
{
    CapturedSize = n;
    memcpy(Captured, d, n);
    return ESP_OK;
}

static int Failures = 0;
static void check(bool Ok, const char* What) {
    printf("%-52s %s\n", What, Ok ? "ok" : "FAIL");
    if(!Ok) Failures++;
}

int main()
{
    Pixels& P = Pixels::getInstance();
    P.init(10);

    P.clearPixels();
    P.setPixel(0u, 10u, 20u, 30u);      // Red=10, Green=20, Blue=30
    P.setPixel(12u, 1u, 2u, 3u);        // row 1, column 1 -> byte offset 36
    P.render();

    check(CapturedSize == 330u, "frame is 110 * 3 bytes");
    check(Captured[0] == 20u && Captured[1] == 10u && Captured[2] == 30u, "pixel 0 goes out as green, red, blue");
    check(Captured[36] == 2u && Captured[37] == 1u && Captured[38] == 3u, "pixel 12 goes out as green, red, blue at offset 36");
    check(!P.isDirty(), "render clears the dirty flag");

    CapturedSize = 0;
    P.render();
    check(CapturedSize == 0u, "an unchanged frame is not retransmitted");

    P.disablePixels();
    P.render();
    bool AllDark = true;
    for(size_t i = 0; i < CapturedSize; i++) { if(Captured[i] != 0u) AllDark = false; }
    check(AllDark, "disablePixels blanks the frame");

    P.enablePixels();
    P.render();
    check(Captured[0] == 20u && Captured[1] == 10u && Captured[2] == 30u, "enablePixels restores it without a redraw");

    printf("\n%s\n", Failures == 0 ? "all checks passed" : "FAILURES");
    return Failures == 0 ? 0 : 1;
}
