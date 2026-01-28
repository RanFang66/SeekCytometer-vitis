#include "event_data.h"

#include "log.h"
#include "lwip/def.h"
#include "xil_cache.h"
#include "stdbool.h"
#include "udp_server.h"

// BRAM WRITE START AT 1
static u32 event_count = 0;
static u32 event_cache[MAX_EVENT_WORDS_NUM];
static u32 event_word_count = 0;

static u32 read_ptr = 0;
static u32 last_wrap = 0;
static u8 udp_buf[UDP_FRAME_LENGTH];
static u32 udp_len = 0;
static u32 word_cnt = 0;
static u32 send_cnt = 0;
static u32 event_words[MAX_EVENT_WORDS_NUM];

static inline u32 bram_read_word(u32 byte_addr)
{
    Xil_DCacheInvalidateRange(BRAM_BASE_ADDR + byte_addr, 4);
    return Xil_In32(BRAM_BASE_ADDR + byte_addr);
}

static u32 ring_read_word(u32 *addr)
{
    u32 data = bram_read_word(*addr);

    if (*addr >= BRAM_LAST_ADDR)
        *addr = 0;
    else
        *addr += 4;

    return data;
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



typedef enum {
    EVT_SYNC_HEAD,
    EVT_COLLECT,
} evt_state_t;

static evt_state_t evt_state = EVT_SYNC_HEAD;

bool fetch_one_event(u32 channel_num, u32 *out_words, u32 *out_word_count)
{
//    u32 curr_wrap = read_wrap_around_addr();
    u32 tail_addr = read_last_written_addr();

    /* 判断是否有新数据 */
    if (read_ptr == tail_addr)
        return false;

    while (!(read_ptr == tail_addr)) {

        u32 word = ring_read_word(&read_ptr);

        switch (evt_state) {

        case EVT_SYNC_HEAD:
            if (word == MAGIC_HEAD) {
                event_cache[0] = word;
                event_word_count = 1;
                evt_state = EVT_COLLECT;
            }
            break;

        case EVT_COLLECT:
            event_cache[event_word_count++] = word;

            /* 最大保护，防止跑飞 */
            if (event_word_count >= MAX_EVENT_WORDS_NUM) {
                evt_state = EVT_SYNC_HEAD;
                event_word_count = 0;
                break;
            }

            /* 判断是否收齐一个 event */
            if (word == MAGIC_TAIL) {

                /* 计算理论 event 长度 */
                u32 expected_words = 5 + channel_num * 3;

                if (event_word_count == expected_words) {
                    /* 完整合法 event */
                    memcpy(out_words, event_cache,
                           event_word_count * sizeof(u32));
                    *out_word_count = event_word_count;

                    evt_state = EVT_SYNC_HEAD;
                    event_word_count = 0;
//                    last_wrap = curr_wrap;
                    event_count++;
                    return true;
                } else {
                    /* 长度不对，丢弃并重新同步 */
                    evt_state = EVT_SYNC_HEAD;
                    event_word_count = 0;
                }
            }
            break;
        }
    }

//    last_wrap = curr_wrap;
    return false;
}


void send_events_udp(u32 channel_num)
{
    while (fetch_one_event(channel_num, event_words, &word_cnt)) {

        u32 event_bytes = word_cnt * 4;
        u32 net_word = 0;
        if (udp_len + event_bytes > UDP_FRAME_LENGTH) {
        	send_udp_frame(CMD_EVENT_DATA, udp_len, udp_buf);
        	send_cnt++;
            udp_len = 0;
        }

        for (u32 i = 0; i < word_cnt; i++) {
        	net_word = htonl(event_words[i]);
        	memcpy(&udp_buf[udp_len], &net_word, 4);
        	udp_len += 4;
        }

    }

    if (udp_len > 0) {
    	send_udp_frame(CMD_EVENT_DATA, udp_len, udp_buf);
    	send_cnt++;
    	udp_len = 0;
    }
}








