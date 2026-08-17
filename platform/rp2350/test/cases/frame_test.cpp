/* What Pixels::render() puts on the wire, checked against a captured transfer. The DMA
   call is stubbed here rather than in pio_stubs.cpp because this test has to keep the frame
   the driver handed over - the shared stub throws it away.

   One word per LED here where the ESP32 has three bytes, which is the shape the state
   machine reads. The check is the same question asked of a different container: green,
   then red, then blue, from the top of the word down. */
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pio.h>

#include "Arduino.h"
#include "Pixels.h"
#include "check.h"

#include <cstdio>
#include <cstring>

static uint32_t Captured[256];
static size_t CapturedCount = 0;

/* The blocks themselves, which the driver takes the FIFO address out of. Defined here as
   well as in pio_stubs.cpp because this test links that file's replacement rather than the
   file. */
pio_hw pio0_hw{};
pio_hw pio1_hw{};

int pio_add_program(PIO, const pio_program_t*) { return 0; }
pio_sm_config pio_get_default_sm_config(void) { return {}; }
void sm_config_set_wrap(pio_sm_config*, uint, uint) { }
void sm_config_set_sideset(pio_sm_config*, uint, bool, bool) { }
void sm_config_set_sideset_pins(pio_sm_config*, uint) { }
void sm_config_set_out_shift(pio_sm_config*, bool, bool, uint) { }
void sm_config_set_fifo_join(pio_sm_config*, pio_fifo_join) { }
void sm_config_set_clkdiv(pio_sm_config*, float) { }
void pio_gpio_init(PIO, uint) { }
void pio_sm_set_consecutive_pindirs(PIO, uint, uint, uint, bool) { }
void pio_sm_init(PIO, uint, uint, const pio_sm_config*) { }
void pio_sm_set_enabled(PIO, uint, bool) { }
uint pio_get_dreq(PIO, uint, bool) { return 0u; }
uint32_t clock_get_hz(clock_handle_t) { return 150u * 1000u * 1000u; }

int dma_claim_unused_channel(bool) { return 0; }
dma_channel_config dma_channel_get_default_config(uint) { return {}; }
void channel_config_set_transfer_data_size(dma_channel_config*, dma_channel_transfer_size) { }
void channel_config_set_read_increment(dma_channel_config*, bool) { }
void channel_config_set_write_increment(dma_channel_config*, bool) { }
void channel_config_set_dreq(dma_channel_config*, uint) { }
bool dma_channel_is_busy(uint) { return false; }

void dma_channel_configure(uint, const dma_channel_config*, volatile void*, const void* Source, uint Count, bool)
{
    CapturedCount = Count;
    memcpy(Captured, Source, Count * sizeof(uint32_t));
}

/* The three bytes as the strip receives them, taken out of the word the way the state
   machine shifts them: from bit 31 down. Written out here rather than compared against the
   shifts the driver used, so that a change to those has to be justified against the wire
   order and not against itself. */
static byte green(uint32_t Word) { return static_cast<byte>((Word >> 24u) & 0xFFu); }
static byte red(uint32_t Word)   { return static_cast<byte>((Word >> 16u) & 0xFFu); }
static byte blue(uint32_t Word)  { return static_cast<byte>((Word >> 8u) & 0xFFu); }


int main()
{
    Pixels& P = Pixels::getInstance();
    P.init(10);

    P.clearPixels();
    P.setPixel(0u, 10u, 20u, 30u);      // Red=10, Green=20, Blue=30
    P.setPixel(12u, 1u, 2u, 3u);        // row 1, column 1 -> word 12
    P.render();

    check(CapturedCount == 110u, "frame is one word per LED");
    check(green(Captured[0]) == 20u && red(Captured[0]) == 10u && blue(Captured[0]) == 30u,
          "pixel 0 goes out as green, red, blue from the top of the word");
    check(green(Captured[12]) == 2u && red(Captured[12]) == 1u && blue(Captured[12]) == 3u,
          "pixel 12 goes out as green, red, blue in word 12");
    /* The eight low bits are never shifted out - autopull refills after 24 - so nothing
       may depend on them, and nothing here checks them. */
    check(!P.isDirty(), "render clears the dirty flag");

    CapturedCount = 0;
    P.render();
    check(CapturedCount == 0u, "an unchanged frame is not retransmitted");

    P.disablePixels();
    P.render();
    bool AllDark = true;
    for(size_t i = 0; i < CapturedCount; i++) { if((Captured[i] & 0xFFFFFF00u) != 0u) AllDark = false; }
    check(AllDark, "disablePixels blanks the frame");

    P.enablePixels();
    P.render();
    check(green(Captured[0]) == 20u && red(Captured[0]) == 10u && blue(Captured[0]) == 30u,
          "enablePixels restores it without a redraw");
    return report();
}
