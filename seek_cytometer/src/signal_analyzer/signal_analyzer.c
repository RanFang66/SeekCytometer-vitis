

/***************************** Include Files *******************************/
#include "signal_analyzer.h"
#include "xil_io.h"
#include "xil_cache.h"
/************************** Function Definitions ***************************/

void start_analyze()
{
	u32 control_reg = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL);
	control_reg |= ANALYZE_ENABLE_MASK;
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL, control_reg);
}


void stop_analyze()
{
	u32 control_reg = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL);
	control_reg &= ~ANALYZE_ENABLE_MASK;
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL, control_reg);
}

void start_sort()
{
	u32 control_reg = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL);
	control_reg |= ANALYZE_ENABLE_MASK;		// Must enable analyze to start sort
	control_reg |= SORT_ENABLE_MASK;

	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL, control_reg);
}

void stop_sort()
{
	u32 control_reg = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL);
	control_reg &= ~SORT_ENABLE_MASK;
	control_reg &= ~ANALYZE_ENABLE_MASK;

	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL, control_reg);
}

void write_detetor_settings(int index, int thresh)
{
//	u32 enabled_channels = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CHANNELS);
//	if (enabled) {
//		enabled_channels |= ((u32)0x01 < index);
//	} else {
//		enabled_channels &= ~((u32)0x01 < index);
//	}
//	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CHANNELS, enabled_channels);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_THRESH_CH1 + index * 4, thresh);
}

void write_enabled_channels(u8 enabled_channels)
{
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CHANNELS, enabled_channels);
}


void write_gate_settings(struct GateData data)
{
	u32 gate_property = 0;
	if (data.ch_x > 0) data.ch_x--;	// In fpga channel start at 0 not 1
	if (data.ch_y > 0) data.ch_y--;
	gate_property = (data.ch_x & GATE_CH_X_MASK)
					| ((data.type_x << 8) & GATE_TYPE_X_MASK)
					| ((data.ch_y << 16) & GATE_CH_Y_MASK)
					| ((data.type_y << 24) & GATE_TYPE_Y_MASK);

	u32 gate_type = 0;
	gate_type = (data.gate_type & GATE_TYPE_MASK) | ((data.gate_points_num << 4) & GATE_POINTS_NUM_MASK);


	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_GATE_SETTING, gate_property);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_GATE_TYPE, gate_type);

	for (int i = 0; i < data.gate_points_num; i++) {
		SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALZYER_GATE_POINT_X(i), data.point_x[i]);
		SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_GATE_POINT_Y(i), data.point_y[i]);
	}

}

int write_gates_at(int i ,struct GateData *data, uint8_t tail)
{
	int j = 0;
	uint32_t *p, check;
	if (!data || i < 0 || i >= MAX_GATE_NUM || data->gate_points_num > 12) {
		return -1;
	}
	if (data->ch_x > 0 && data->ch_y > 0) {
		data->ch_x--;
		data->ch_y--;
	} else {
		return -1;
	}
	PL_GateConfig_t * gate_config = GATE_CONFIG(i);
	gate_config->header = tail ? HEADER_TAIL_CODE : HEADER_BODY_CODE;
	gate_config->checkbits = 0;

	gate_config->gate_config = (data->gate_type & 0xFF);
//	switch(data->gate_type) {
//	case 0:	// Interval
//		gate_config->gate_config = GT_INTERVAL;
//		break;
//	case 1:	// Rectangle
//		gate_config->gate_config = GT_RECTANGLE;
//		break;
//	case 2:	// Polygon
//		gate_config->gate_config = GT_POLYGON;
//		break;
//	case 3:	// Ellipse
//		gate_config->gate_config = GT_ELLIPSE;
//		break;
//	case 4:	// Quadrant
//		gate_config->gate_config = GT_NO_GATE;
//		break;
//	default: //
//		gate_config->gate_config = GT_NO_GATE;
//		break;
//	}
	// bit[15:8] = number of 32-bit memory units occupied by the points; bit[31:16]
	// was for a parameter (e.g. quadrant type) but is now reused as the point count.
	gate_config->gate_config |= (data->gate_points_num << 9) | (data->gate_points_num << 16);

	gate_config->channel_config = (data->ch_x & GATE_CH_X_MASK)
								| ((data->type_x << 8) & GATE_TYPE_X_MASK)
								| ((data->ch_y << 16) & GATE_CH_Y_MASK)
								| ((data->type_y << 24) & GATE_TYPE_Y_MASK);
	for (j = 0; j < MAX_GATE_POINTS_NUM; j++) {
		gate_config->points[j].pointx = data->point_x[j];
		gate_config->points[j].pointy = data->point_y[j];
	}
	check = 0;
	for (j = 0, p = (uint32_t *)gate_config; j < 4 + data->gate_points_num * 2; j++, p++) {
		check ^= *p;
	}
	gate_config->checkbits = check;
	Xil_DCacheFlushRange((INTPTR)gate_config, sizeof(PL_GateConfig_t));
	return 0;
}

void reset_gates(PL_GateType_e gate)
{
	PL_GateConfig_t * gate_config = GATE_CONFIG(0);
	gate_config->header = HEADER_TAIL_CODE;
	gate_config->checkbits = HEADER_TAIL_CODE ^ gate;
	gate_config->gate_config = gate;
	gate_config->channel_config = 0;
	for (int i = 0; i < MAX_GATE_POINTS_NUM; i++) {
		gate_config->points[i].pointx = 0;
		gate_config->points[i].pointy = 0;
	}
	Xil_DCacheFlushRange((INTPTR)gate_config, sizeof(PL_GateConfig_t));
}


void write_speed_measure_settings(u8_t speed_en, u8_t pre_ch, u8_t post_ch, int max_diff_time, int dist)
{
	// Set or clear speed measure enable bit in control register
	u32 control_reg = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL);
	if (speed_en) {
		control_reg |= SPEED_MEASURE_EN_MASK;
	} else {
		control_reg &= ~SPEED_MEASURE_EN_MASK;
	}
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL, control_reg);

	// Because channel start from 0 at FPGA, so minus 1.
	if (pre_ch > 0) pre_ch--;
	if (post_ch > 0) post_ch--;
	u32 speed_ch = ((u32)post_ch << 8) | ((u32)pre_ch);

	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_CH, speed_ch);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_MAX_DIFF, max_diff_time);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_DIST, dist);
}




void write_delay_refer_settings(u8_t en, u8_t ch_id)
{
	// Bit3 of slv_reg0 (control reg): delay refer enable
	u32 control_reg = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL);
	if (en) {
		control_reg |= DELAY_REFER_EN_MASK;
	} else {
		control_reg &= ~DELAY_REFER_EN_MASK;
	}
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_CONTROL, control_reg);

	// Bits 18:16 of slv_reg52 (SPEED_CH reg): delay refer channel id (FPGA channel starts from 0)
	if (ch_id > 0) ch_id--;
	u32 speed_ch = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_CH);
	speed_ch &= ~DELAY_REFER_CH_MASK;
	speed_ch |= (((u32)ch_id << 16) & DELAY_REFER_CH_MASK);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_CH, speed_ch);
}


void write_drive_settings(struct DriveParas paras)
{
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DRIVE_TYPE, paras.drive_type);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DRIVE_DELAY, paras.drive_delay);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DRIVE_WIDTH, paras.drive_width);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_COOLING_TIME, paras.cooling_time);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DELAY_COE, paras.speed_coee);
}


void write_baseline_settings(u8 baseline_en, u8 alpha_shift, u8 slope_th)
{
	u32 ctrl = 0;
	if (baseline_en) {
		ctrl |= BASELINE_EN_MASK;
	}
	ctrl |= ((u32)(alpha_shift & 0x1F) << 2) & ALPHA_SHIFT_MASK;
	ctrl |= ((u32)(slope_th & 0x3F) << 8) & SLOPE_TH_MASK;
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_CTRL, ctrl);
}


void write_baseline_params(u16 acquire_len, u8 guard_len, u16 max_pulse_wid)
{
	u32 param = ((u32)(guard_len & 0xFF) << 16)
	          | ((u32)(acquire_len & 0xFFFF));
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_PARAM, param);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_PARAM2, (u32)(max_pulse_wid & 0xFFFF));
}


void baseline_reacquire(void)
{
	u32 ctrl = SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_CTRL);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_CTRL, ctrl | ACQUIRE_FORCE_MASK);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_CTRL, ctrl & ~ACQUIRE_FORCE_MASK);
}


u32 read_baseline_mon(u8 ch_id)
{
	// Select channel (FPGA channel index, 0-based)
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_MON_CH, (u32)(ch_id & BASELINE_MON_CH_MASK));
	// Flush the posted mon_ch write before reading: on Zynq a read to a different
	// PL address is not ordered after a write, so without this the monitor read can
	// return the previous channel's baseline (the "occasional jump" symptom).
	(void)SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_MON_CH);
	// slv_reg43: [18]=baseline_valid, [17:0]=baseline (integer ADC code)
	return SIGNAL_ANALYZER_mReadReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_BASELINE_MON);
}

