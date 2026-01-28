#ifndef __EVENT_DATA_H__
#define __EVENT_DATA_H__

#include "xil_types.h"
#include "xil_io.h"
#include "xparameters.h"
#include "signal_analyzer.h"

#define BRAM_BASE_ADDR    		XPAR_BRAM_0_BASEADDR
#define BRAM_CTRL_BASE_ADDR		XPAR_BRAM_0_CTRL_BASEADDR



#define BRAM_SIZE_BYTES  0x00008000
#define BRAM_LAST_ADDR   (BRAM_SIZE_BYTES - 4)
#define BRAM_WORDS       (BRAM_SIZE_BYTES / 4)

#define MAX_EVENT_WORDS_NUM		(64)
#define MAGIC_HEAD				0x55AA55AA
#define MAGIC_TAIL 				0xAA55AA55

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


void send_events_udp(u32 channel_num);

#endif

