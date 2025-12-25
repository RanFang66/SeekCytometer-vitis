#include "axi_dma.h"
#include "log.h"
#include "dma_bd.h"
#include "ad_sample/ad_sample.h"
#include "interrupts.h"
#include "xparameters.h"
#include "xaxidma.h"

#include "cytometer.h"

#define AXIDMA_DEVICE_ID 			XPAR_AXIDMA_0_DEVICE_ID
#define AXIDMA_BASE_ADDR			XPAR_AXIDMA_0_BASEADDR

#define AXIDMA_S2MM_INTR_ID 	    XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR




static XAxiDma axidma_inst;
static u8 axidma_interrupt_priority = 0x08;		// Highest priority
static u8 axidma_interrupt_trigger = 3; 		// Rise edge sensitive
static u16 bd_count = 4;
static u16 dma_intr_count = 0;

/*
 * BD buffer
 */
u32 bd_chain_buffer[BD_ALIGNMENT*16] __attribute__ ((aligned(64)));
int32_t dma_rx_buffer[MAX_DMA_LEN/sizeof(int32_t)]  __attribute__ ((aligned(64)));


volatile u8 dma_s2mm_flag = 0;				// Receive s2mm data interrupt flag


XStatus init_axidma()
{
	XAxiDma_Config *dma_config;
	XAxiDma *axidma = &axidma_inst;
	XStatus status;

	dma_config = XAxiDma_LookupConfig(AXIDMA_DEVICE_ID);
	if (!dma_config) {
		LOG_ERROR("Cannot find the configuration of AXIDMA with id %d.", AXIDMA_DEVICE_ID);
		return XST_FAILURE;
	}

	status = XAxiDma_CfgInitialize(axidma, dma_config);
	if (status != XST_SUCCESS) {
		LOG_ERROR("Initialize the AXI DMA failed!");
		return status;
	}

	/* Disable MM2S interrupt and enable S2MM interrupt */
	XAxiDma_IntrEnable(axidma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(axidma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	return XST_SUCCESS;
}

void dma_interrupt_handler(void *callback_ref)
{
	XAxiDma *axidma_inst = (XAxiDma *)callback_ref;
	if (!axidma_inst) {
		LOG_ERROR("AXI DMA instance is NULL!");
		return;
	}

	int s2mm_status = XAxiDma_IntrGetIrq(axidma_inst, XAXIDMA_DEVICE_TO_DMA);


	if ((s2mm_status & XAXIDMA_IRQ_ERROR_MASK)) {
	    XAxiDma_IntrAckIrq(axidma_inst, XAXIDMA_IRQ_ERROR_MASK, XAXIDMA_DEVICE_TO_DMA);
	    LOG_ERROR("DMA S2MM error interrupt occurred!");
	    return;
	}

	if (s2mm_status & XAXIDMA_IRQ_IOC_MASK) {
		// Clear interrupt
		XAxiDma_IntrAckIrq(axidma_inst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
//		dma_intr_count++;
//		if (dma_intr_count % 32 == 0) {
//			LOG_INFO("AXI DMA received interrupt for %d times.", dma_intr_count);
//		}

		// Set DMA s2mm falg
		dma_s2mm_flag = 1;
	}
}


XStatus setup_axidma()
{
	XStatus status;
	status = init_axidma();
	if (status != XST_SUCCESS) {
		LOG_ERROR("Platform initialize AXI DMA failed!");
		return status;
	}

	status = connect_interrupt(AXIDMA_S2MM_INTR_ID, dma_interrupt_handler, &axidma_inst, axidma_interrupt_priority, axidma_interrupt_trigger);
	if (status != XST_SUCCESS) {
		LOG_ERROR("Platform connect AXI DMA interrupt failed!");
		return status;
	}
	CreateBdChain(bd_chain_buffer, bd_count, ADC_BYTE * ADC_CH_COUNT * DMA_ADC_SAMPLE_NUM, (unsigned char *)dma_rx_buffer, RXPATH);
	LOG_INFO("Platform setup AXI DMA finished!");
	return XST_SUCCESS;
}



void axi_dma_start()
{
	/* Clear BD Status */
	Bd_StatusClr(bd_chain_buffer, bd_count);
	/* Start sample */
	start_ad_data_upload(DMA_ADC_SAMPLE_NUM);
	/* start DMA translation from ADC to DDR3 */
	Bd_Start(bd_chain_buffer, bd_count, &axidma_inst, RXPATH);
}

int update_waveform_data(u8_t waveform_buffer[], u8 enabled_channels, u8_t interval)
{
	Xil_DCacheInvalidateRange((INTPTR) dma_rx_buffer, ADC_BYTE*ADC_CH_COUNT*DMA_ADC_SAMPLE_NUM);
	if (interval < 1) {
		interval = 1;
	}

	int data_len = 0;
	for (int i = 0; i < DMA_ADC_SAMPLE_NUM; i += interval) {
		for (int ch = 0; ch < ADC_CH_COUNT; ch++) {
			if (enabled_channels &((u8)1 << ch)) {
				waveform_buffer[data_len++] = (u8)(dma_rx_buffer[ADC_CH_COUNT*i + ch] >> 24);
				waveform_buffer[data_len++] = (u8)(dma_rx_buffer[ADC_CH_COUNT*i + ch] >> 16);
				waveform_buffer[data_len++] = (u8)(dma_rx_buffer[ADC_CH_COUNT*i + ch] >> 8);
				waveform_buffer[data_len++] = (u8)dma_rx_buffer[ADC_CH_COUNT*i + ch];
			}
		}
	}

	return data_len;

//	for (int ch = 0; ch < ADC_CH_COUNT; ch++) {
//		if (enabled_channels & ((u8)1 << ch)) {
//			for(int i = 0 ; i < DMA_ADC_SAMPLE_NUM ; i++) {
//				dma_rx_byte_buffer[ch][i*4] = (u8)(dma_rx_buffer[8*i + ch] >> 24);
//				dma_rx_byte_buffer[ch][i*4+1] = (u8)(dma_rx_buffer[8*i + ch] >> 16);
//				dma_rx_byte_buffer[ch][i*4+2] = (u8)(dma_rx_buffer[8*i + ch] >> 8);
//				dma_rx_byte_buffer[ch][i*4+3] = (u8)dma_rx_buffer[8*i + ch];
//			}
//		}
//	}
}


