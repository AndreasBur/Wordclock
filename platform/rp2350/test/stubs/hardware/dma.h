#ifndef _HARDWARE_DMA_H_
#define _HARDWARE_DMA_H_
#include <stdint.h>
#include <stddef.h>
#include "pio.h"
typedef enum { DMA_SIZE_8 = 0, DMA_SIZE_16 = 1, DMA_SIZE_32 = 2 } dma_channel_transfer_size;
typedef struct { uint32_t ctrl; } dma_channel_config;
int dma_claim_unused_channel(bool);
dma_channel_config dma_channel_get_default_config(uint);
void channel_config_set_transfer_data_size(dma_channel_config*, dma_channel_transfer_size);
void channel_config_set_read_increment(dma_channel_config*, bool);
void channel_config_set_write_increment(dma_channel_config*, bool);
void channel_config_set_dreq(dma_channel_config*, uint);
void dma_channel_configure(uint, const dma_channel_config*, volatile void*, const void*, uint, bool);
bool dma_channel_is_busy(uint);
#endif
