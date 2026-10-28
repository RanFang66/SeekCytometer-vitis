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

/* 最近一次下发的基线参数。基线寄存器在 PL 里, 重新下载 bitstream 会复位成
 * 默认值(基线关闭), 而 PS 的 current_state 仍然有效 —— 所以 START 时要补下发
 * 一次, 否则用户会发现"调好的基线自己关了"。
 * acquire_force 有意不缓存: 它是一次性脉冲, 每次 START 重发会把预热计数反复清零。 */
static u8_t  m_baseline_en       = 0;
static u8_t  m_baseline_alpha    = 16;
static u16_t m_baseline_acq_len  = 1024;
static u8_t  m_baseline_guard    = 8;
static u16_t m_baseline_max_wid  = 4095;
static u8_t  m_baseline_sent     = 0;

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
    reset_gates(GT_NO_GATE);

	LOG_INFO("Initialize cytometer parameters ok.");
}

int get_current_state()
{
    return current_state;
}

/* 把 PS 侧缓存的检测器/使能配置重新写一遍到 PL。
 * 用于 PL 被重新下载 bitstream（寄存器全部复位）后恢复, 也用于每次 START。 */
static void resync_analyzer_settings(void)
{
	write_enabled_channels(m_enabled_channels);
	for (int i = 0; i < AD_CHANNEL_NUM; i++) {
		if (detector_settings[i].enabled) {
			write_detetor_settings(i, detector_settings[i].threshold_val);
		}
	}
	/* 基线参数: PL 复位后一并补下发(不发 acquire_force, 避免每次 START 重置预热)。 */
	if (m_baseline_sent) {
		write_baseline_settings(m_baseline_en, m_baseline_alpha, 0);
		write_baseline_params(m_baseline_acq_len, m_baseline_guard, m_baseline_max_wid);
	}
}

void cytometer_start_analyze()
{
	/* 幂等: 不因为 current_state 不是 IDLE 就静默丢弃。
	 * PL 可能被重新下载过 bitstream 而把 slv_reg0 复位成 0, 此时 PS 的
	 * current_state 仍停留在 ACQUIRING/SORTING; 旧实现会丢弃 START,
	 * 导致 analyze_en 永远回不到 1（上位机必须点两次 Stop/Start 才行）。 */
	if (current_state != STATE_SORTING) {
		current_state = STATE_ACQUIRING;
	}
	analyze_enabled = 1;
	resync_analyzer_settings();
	start_analyze();
}

void cytometer_stop_analyze()
{
	/* 同样做成幂等: 之前只有 ACQUIRING 才停, 一旦状态和 PL 不同步就再也停不下来。 */
	if (current_state == STATE_SORTING) {
		LOG_WARN("Stop analyze ignored: sorting is active, stop sorting first.");
		return;
	}
	analyze_enabled = 0;
	current_state = STATE_IDLE;
	stop_analyze();
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
        /* 停分选不等于停采集: 采集还在跑时状态回到 ACQUIRING, 而不是 IDLE。
         * 保持 IDLE 会让后续"停采集"看起来生效, 但状态机已经不同步。 */
        current_state = analyze_enabled ? STATE_ACQUIRING : STATE_IDLE;
        stop_sort();
    }
}

void cytometer_set_drive_settings(struct DriveParas paras)
{
	write_drive_settings(paras);
}


void cytometer_set_gate(struct GateData gate)
{
	write_gate_settings(gate);
}


void cytometer_set_speed_measure(u8_t speed_en, u8_t preCh, u8_t postCh, int max_diff_time, int dist)
{
	write_speed_measure_settings(speed_en, preCh, postCh, max_diff_time, dist);
}

void cytometer_set_delay_refer(u8_t en, u8_t ch_id)
{
	write_delay_refer_settings(en, ch_id);
}

void cytometer_set_baseline(u8_t baseline_en, u8_t alpha_shift, u8_t slope_th,
                            u8_t acquire_force, u16_t acquire_len, u8_t guard_len, u16_t max_pulse_wid)
{
	write_baseline_settings(baseline_en, alpha_shift, slope_th);
	write_baseline_params(acquire_len, guard_len, max_pulse_wid);

	m_baseline_en      = baseline_en;
	m_baseline_alpha   = alpha_shift;
	m_baseline_acq_len = acquire_len;
	m_baseline_guard   = guard_len;
	m_baseline_max_wid = max_pulse_wid;
	m_baseline_sent    = 1;

	if (acquire_force) {
		baseline_reacquire();
	}
}

void cytometer_read_baseline_mon(u32_t *values_8)
{
	for (int i = 0; i < AD_CHANNEL_NUM; i++) {
		values_8[i] = (u32_t)read_baseline_mon((u8)i);
	}
}

u32_t cytometer_read_baseline_debug(void)
{
	return (u32_t)read_baseline_debug();
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