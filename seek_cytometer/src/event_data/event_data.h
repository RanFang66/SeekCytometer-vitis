#ifndef __EVENT_DATA_H__
#define __EVENT_DATA_H__

#include "xil_types.h"
#include "xil_io.h"
#include "xparameters.h"
#include "signal_analyzer.h"

#define BRAM_BASE_ADDR    		XPAR_BRAM_0_BASEADDR
#define BRAM_CTRL_BASE_ADDR		XPAR_BRAM_0_CTRL_BASEADDR

#define BRAM_ADDR_MAX 			0x00004000
#define MAGIC_WORD 				0xAA55AA55
#define NUM_CHANNELS			8


typedef struct {
    uint32_t event_id;        		// Event ID
    uint32_t channel_mask;    		// Channel Mask
    int32_t  peak[NUM_CHANNELS];   	// Peak Data
    uint32_t width[NUM_CHANNELS];  	// Width Data
    int32_t  area[NUM_CHANNELS];   	// Area Data
} EventData;

u32 process_events();

const u8 *get_event_data_buffer_udp();

void reset_event_data();

#endif

