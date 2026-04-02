

/***************************** Include Files *******************************/
#include "signal_analyzer.h"
#include "xil_io.h"
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

void write_speed_measure_settings(u8_t pre_ch, u8_t post_ch, int thresh, int dist, int max_diff_time)
{
	u32 speed_ch = 0;
	// Because channel start from 0 at FPGA, so minus 1.
	if (pre_ch > 0) pre_ch--;
	if (post_ch > 0) post_ch--;
	speed_ch = ((u32)post_ch << 8) | ((u32)pre_ch);
	u32 thresh_addr = SIGNAL_ANALYZER_THRESH_CH1 + pre_ch * 4;

	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_CH, speed_ch);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, thresh_addr, thresh);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_DIST, dist);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_SPEED_MAX_DIFF, max_diff_time);


}




void write_drive_settings(struct DriveParas paras)
{
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DRIVE_TYPE, paras.drive_type);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DRIVE_DELAY, paras.drive_delay);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DRIVE_WIDTH, paras.drive_width);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_COOLING_TIME, paras.cooling_time);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_DELAY_COE, paras.speed_coee);
	SIGNAL_ANALYZER_mWriteReg(SIGNAL_ANALYZER_BASE_ADDR, SIGNAL_ANALYZER_MIN_EVENT_INTERVAL, paras.min_event_interval);
}

