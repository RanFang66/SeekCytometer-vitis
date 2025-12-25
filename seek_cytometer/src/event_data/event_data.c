#include "event_data.h"

#include "log.h"
#include "lwip/def.h"
#include "xil_cache.h"

// BRAM WRITE START AT 1

static u32 last_wr_addr = 0;
static u32 last_wrap = 0;
static u32 event_counter = 0;


static u32 event_data_buffer[(BRAM_ADDR_MAX - 1) / 4];
u8 data_buffer_for_udp[BRAM_ADDR_MAX - 1];

void reset_event_data()
{
	last_wr_addr = 0;
}


u32 read_last_written_addr()
{
	u32 bram_state = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BRAM_STATE);
	return (bram_state & 0x00FFFFFF);
}

u32 read_wrap_around_addr()
{
	u32 bram_state = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BRAM_STATE);
	return ((bram_state & 0x01000000) >> 24);
}

u32 read_bram_block(u32 start_addr, u32 last_addr, u32 *buffer) {
    // Disable cache to ensure get latest data
    Xil_DCacheDisable();

    u32 read_addr;
//    for (u32 i = 0; i < word_count; i++) {
////    	u32 index = (start_addr + i > BRAM_ADDR_MAX) ? (start_addr + i - BRAM_ADDR_MAX) : (start_addr + i);
//
////    	u32 index = start_addr+i;
////    	if (index > BRAM_ADDR_MAX) {
////    		index = index - BRAM_ADDR_MAX + 1;
////    	}
////    	u32 addr = BRAM_BASE_ADDR + index * 4;
//        buffer[i] = Xil_In32(BRAM_BASE_ADDR + read_addr);
//        if (read_addr == BRAM_ADDR_MAX * 4) {
//        	read_addr = 4;
//        } else {
//        	read_addr += 4;
//        }
//    }
    u32 count = 0;
    if (last_addr > start_addr) {
    	for (read_addr = start_addr; read_addr < last_addr; read_addr += 4) {
    		buffer[count] = Xil_In32(BRAM_BASE_ADDR + read_addr);
    		count++;
    	}
    } else {
    	for (read_addr = start_addr; read_addr <= BRAM_ADDR_MAX; read_addr += 4) {
    		buffer[count] = Xil_In32(BRAM_BASE_ADDR + read_addr);
    		count++;
    	}
    	for (read_addr = 0; read_addr < last_addr; read_addr += 4) {
    		buffer[count] = Xil_In32(BRAM_BASE_ADDR + read_addr);
    		count++;
    	}
    }

    // Re-enable the cache
    Xil_DCacheEnable();
    return count-1;
}


int parse_event_data(u32 *data_buffer, u32 word_count, EventData *event) {
    // Check minimal data length
    if (word_count < 2) {
        LOG_ERROR("Insufficient data for event (min 2 words, got %d)", word_count);
        return -1;
    }

    // Parse header
    u32 header = data_buffer[0];
    event->channel_mask = (header >> 24) & 0xFF;  // MSB is channel mask
    event->event_id = header & 0x00FFFFFF;        // Low 3 bytes is event id

    // Calculate the channels number
    u32 enabled_channels = 0;
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (event->channel_mask & (1 << i)) {
            enabled_channels++;
        }
    }

    // Calculate expected words number
    u32 expected_words = 1 + (enabled_channels * 3) + 1; // Header + 3 words per channel  + Magic word

    // Check data length
    if (word_count < expected_words) {
        LOG_ERROR("Data truncated. Expected %d words, got %d", expected_words, word_count);
        return -2;
    }

    // Check magic word
    u32 magic_index = expected_words - 1;
    if (data_buffer[magic_index] != MAGIC_WORD) {
        LOG_ERROR("Error: Magic word mismatch. Expected 0x%08X, got 0x%08X",
                   MAGIC_WORD, data_buffer[magic_index]);
        return -3;
    }

    // Parse channel data
    u32 data_index = 1; // Start after header
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        if (event->channel_mask & (1 << ch)) {
            // Peak value
            event->peak[ch] = (int32_t)data_buffer[data_index++];

            // Width Value
            event->width[ch] = data_buffer[data_index++];

            // Area Value
            event->area[ch] = (int32_t)data_buffer[data_index++];
        } else {
            // Disabled channels
            event->peak[ch] = 0;
            event->width[ch] = 0;
            event->area[ch] = 0;
        }
    }

    return expected_words;
}

void convert_to_network_byte_order(const uint32_t* data_in, uint8_t* frame_out, size_t num_words) {
    for (size_t i = 0; i < num_words; i++) {
        uint32_t network_order = htonl(data_in[i]);
        memcpy(&frame_out[i * 4], &network_order, sizeof(uint32_t));
    }
}


u32 process_events() {
	// 1. Read current state
	u32 current_wr_addr = read_last_written_addr();


	// 2. Check if there are new data write
	if (current_wr_addr != last_wr_addr) {
		u32 data_count = 0;
//		u32 start_addr = 0;

		// 3. Calculate data number
//		data_count = (current_wr_addr > last_wr_addr) ? (current_wr_addr -last_wr_addr) : (BRAM_ADDR_MAX - last_wr_addr + current_wr_addr);
//		if (last_wr_addr == 0) {
//			start_addr = 1;
//		} else {
//			start_addr = last_wr_addr + 4;
//		}
		// 4 Read data
		data_count = read_bram_block(last_wr_addr, current_wr_addr, event_data_buffer);

		// 5 Convert to UDP data buffer
		convert_to_network_byte_order(event_data_buffer, data_buffer_for_udp, data_count);

		last_wr_addr = current_wr_addr;
		return data_count * 4;
	}else {
		return 0;
	}
}


const u8 *get_event_data_buffer_udp()
{
	return data_buffer_for_udp;
}


