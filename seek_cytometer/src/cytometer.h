#ifndef __CYTOMETER__HEADER__
#define __CYTOMETER__HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/cc.h>

enum CYTOMETER_STATE {
    STATE_IDLE,
    STATE_ACQUIRING,
    STATE_SORTING,
    STATE_ERROR,
};

struct GateData {
	int gate_type;
	int ch_x;
	int ch_y;
	int type_x;
	int type_y;
	int x_min;
	int x_max;
	int y_min;
	int y_max;
};


struct DETECTOR_SETTING {
	u8_t 	id;
    u8_t    enabled;
	u8_t 	enable_height;
	u8_t	enable_width;
	u8_t 	enable_area;
	u8_t 	enable_thresh;

	int 	threshold_val;
};

struct SORTER_SETTING {
	u8_t    trigger_type;
	u32_t   time_delay;
	u32_t   time_cooling;
	u32_t   time_drive_width;
};


#define AD_CHANNEL_NUM				        (8)
#define THRESHOLD_TYPE_OR 			        (0)
#define THRESHOLD_TYPE_AND		    	    (1)

void update_detector_settings(u8_t id, u8_t enable_height, u8_t enable_width, u8_t enable_area, u8_t enable_thresh, int threshold_val);
void update_enabled_channels(u8_t enabeld_channels);

void reset_detector(int id);
void init_detectors();
void init_cytometer();
void cytometer_start_analyze();
void cytometer_stop_analyze();
void cytometer_start_sort();
void cytometer_stop_sort();

void cytometer_set_drive_settings(u8_t type, u32_t delay, u32_t width, u32_t cooling_time, u32_t coe);
void cytometer_set_gate(struct GateData gate);
void cytometer_set_speed_measure(u8_t preCh, u8_t postCh, int thresh, int dist, int max_diff_time);

u8_t waveform_is_enabled();
u8_t waveform_enabled_channel_num();
u8_t waveform_sample_interval();

u8_t waveform_enabled_channels();

int analyze_enabled_channels_num();
u8_t analyze_enabled_channels();


void update_waveform_setting(u8_t waveform_en, u8_t channels, u8_t interval);

int get_current_state();

#ifdef __cplusplus
}
#endif

#endif /* __CYTOMETER__HEADER__ */
