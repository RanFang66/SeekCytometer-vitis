#ifndef __AXI_DMA_H__
#define __AXI_DMA_H__

#include "xil_types.h"
#include "xstatus.h"
#include "ad_sample.h"


#define MAX_DMA_LEN		   		0x200000      /* DMA max length in byte */
#define DMA_ADC_SAMPLE_NUM  	(1024*64)		/* The sample times in one DMA transfer process*/

XStatus setup_axidma();

void axi_dma_start();

int update_waveform_data(u8 dma_rx_byte_buffer[], u8 enabled_channels, u8 interval);

#endif
