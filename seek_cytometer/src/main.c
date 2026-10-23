/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "log.h"
#include "xil_types.h"
#include "udp_server.h"
#include "axi_dma.h"
#include "cytometer.h"
#include "ad_sample.h"
#include "signal_analyzer.h"
extern u8 send_udp_frame_flag;
u8 debug_flag = 0;

// ============================================================================
// [临时调试开关] 信号基线识别与校正 上机验证
// ----------------------------------------------------------------------------
// 复位默认 baseline_en = 0（基线校正关闭），上机验证需显式置 1。
// 阶段4 的 UDP 命令映射尚未实现，故用此编译期开关临时开启。
// 测试完成后：将 DEBUG_BASELINE_ENABLE 改为 0（或删除整块），恢复默认行为。
//
// 参数：
//   write_baseline_settings(baseline_en, alpha_shift, slope_th)
//     baseline_en = 1   开启基线校正（相对阈值 + 度量级减）
//     alpha_shift = 10  慢跟踪 EMA α = 2^-10（τ≈1024 拍）
//     slope_th    = 0   斜率二级门控禁用（第一阶段默认，待标定后再启用）
//   write_baseline_params(acquire_len, guard_len, max_pulse_wid)  // 可选
//     复位默认：slv_reg3=0x00080400 (acquire_len=1024 / guard_len=8)，slv_reg4=255 (max_pulse_wid=255)
// ============================================================================
#define DEBUG_BASELINE_ENABLE  1


int main()
{
    init_platform();

    init_cytometer();

#if DEBUG_BASELINE_ENABLE
    // [临时调试] 开启基线校正（复位默认关闭），测试完成后删除本块或改 DEBUG_BASELINE_ENABLE=0
    write_baseline_settings(1, 10, 0);
    // write_baseline_params(1024, 8, 255);  // 如需覆盖运行时参数再取消注释
#endif

    LOG_INFO("-----------Start Seek Cytometer----------");

    while(1)
    {
    	lwip_loop_func();
//    	if (debug_flag == 1) {
//    		cytometer_start_analyze();
//			update_detector_settings(1, 1, 1, 1, 1, 3000);
//			update_enabled_channels(0x01);
//    	    debug_flag = 0;
//    	} else if (debug_flag == 2) {
//    		cytometer_stop_analyze();
//
//    		debug_flag = 0;
//    	}
    }
    cleanup_platform();

    return 0;
}
