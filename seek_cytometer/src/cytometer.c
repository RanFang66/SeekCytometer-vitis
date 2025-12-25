#include "cytometer.h"
#include "xil_printf.h"
#include "log.h"
#include "signal_analyzer.h"

/* Current Cytometer State */
enum CYTOMETER_STATE current_state;


int     waveform_channel_enabled[AD_CHANNEL_NUM];
struct  DETECTOR_SETTING detector_settings[AD_CHANNEL_NUM];
int     threshold_type = THRESHOLD_TYPE_OR;
u8_t    m_enabled_channels = 0;

u8_t    analyze_enabled = 0;
u8_t    sort_enabled = 0;


u8_t    waveform_enabled = 0;
u8_t    waveform_channels = 0;
u8_t    waveform_interval = 1;




void init_detectors()
{
	for (int i = 0; i < AD_CHANNEL_NUM; i++) {
		detector_settings[i].id = i+1;
        detector_settings[i].enabled = 0;
		detector_settings[i].enable_height = 0;
		detector_settings[i].enable_width = 0;
		detector_settings[i].enable_area = 0;
		detector_settings[i].enable_thresh = 0;
		detector_settings[i].threshold_val = 3000;
	}
}

void reset_detector(int id)
{
    detector_settings[id-1].enabled = 0;
    detector_settings[id-1].enable_height = 0;
    detector_settings[id-1].enable_width = 0;
    detector_settings[id-1].enable_area = 0;
    detector_settings[id-1].enable_thresh = 0;
    detector_settings[id-1].threshold_val = 3000;
}

void update_detector_settings(u8_t id, u8_t enable_height, u8_t enable_width, u8_t enable_area, u8_t enable_thresh, int threshold_val)
{
	struct DETECTOR_SETTING *detector = &(detector_settings[id-1]);
    detector->enabled = 1;
	detector->enable_height = enable_height;
	detector->enable_width = enable_width;
	detector->enable_area = enable_area;
	detector->enable_thresh = enable_thresh;
	detector->threshold_val = threshold_val;

	write_detetor_settings(id-1, threshold_val);
}

void update_enabled_channels(u8_t enabled_channels)
{
	m_enabled_channels = enabled_channels;
	write_enabled_channels(enabled_channels);
}

u8_t analyze_enabled_channels()
{
	return m_enabled_channels;
}

int analyze_enabled_channels_num()
{
	u8_t cnt = 0;
	for (int i = 0; i < 8; i++) {
		if (m_enabled_channels &((u8_t)0x01 << i)) {
			cnt++;
		}
	}
	return cnt;
}

void update_waveform_setting(u8_t waveform_en, u8_t channels, u8_t interval)
{
    waveform_enabled = waveform_en;
    waveform_channels = channels;
    waveform_interval = interval;
}


void update_therhold_type(int type)
{
    if (type != THRESHOLD_TYPE_OR || type != THRESHOLD_TYPE_AND)
        return;

    threshold_type = type;
}


void init_cytometer()
{
    current_state = STATE_IDLE;
	threshold_type = THRESHOLD_TYPE_OR;
	for (int i = 0; i < AD_CHANNEL_NUM; i++) {
        waveform_channel_enabled[i] = 0;
    }
    init_detectors();

	LOG_INFO("Initialize cytometer parameters ok.");
}

int get_current_state()
{
    return current_state;
}

void cytometer_start_analyze()
{
    if (current_state == STATE_IDLE) {
    	analyze_enabled = 1;
        current_state = STATE_ACQUIRING;
        start_analyze();
    }
}

void cytometer_stop_analyze()
{
    if (current_state == STATE_ACQUIRING) {
    	analyze_enabled = 0;
        current_state = STATE_IDLE;
        stop_analyze();
    }
}

void cytometer_start_sort()
{
    if (current_state == STATE_IDLE || current_state == STATE_ACQUIRING) {
    	sort_enabled = 1;
        current_state = STATE_SORTING;
        start_sort();
    }
}
void cytometer_stop_sort()
{
    if (current_state == STATE_SORTING) {
    	sort_enabled = 0;
        current_state = STATE_IDLE;
        stop_sort();
    }
}

void cytometer_set_drive_settings(u8_t type, u32_t delay, u32_t width, u32_t cooling_time)
{
	write_drive_settings(type, delay, width, cooling_time);
}


void cytometer_set_gate(struct GateData gate)
{
	write_gate_settings(gate);
}


void cytometer_set_speed_measure(u8_t preCh, u8_t postCh, int thresh, int dist, int max_diff_time)
{
	write_speed_measure_settings(preCh, postCh, thresh, dist, max_diff_time);
}

u8_t waveform_enabled_channels()
{
	return waveform_channels;
}

u8_t waveform_enabled_channel_num()
{
	u8_t cnt = 0;
	for (int i = 0; i < 8; i++) {
		if (waveform_channels &((u8_t)0x01 << i)) {
			cnt++;
		}
	}
	return cnt;
}

u8_t waveform_ch_is_en(u8_t ch)
{
	return (waveform_channels & ((u8)1 << ch));
}

u8_t waveform_is_enabled()
{
	return waveform_enabled;
}

u8_t waveform_sample_interval()
{
	return waveform_interval;
}

