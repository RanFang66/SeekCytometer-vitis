#include "timers.h"
#include "xiltimer.h"
#include "xil_types.h"
#include "log.h"


static XTimer timer_inst;
static u64 time_ms = 0;
static u64 time_s = 0;
static int timer_interval_ms = 100;
static u8  timer_interrupt_priority = 0;




volatile u8 send_udp_frame_flag = 0;
volatile u8 send_event_data_flag = 0;



static void timer_interval_handler(void *callback_ref, u32 status_event)
{
	time_ms += 100;
	if (time_ms % 200 == 0) {
		send_event_data_flag = 1;
	}

	if (time_ms == 1000) {
		time_s++;
		time_ms = 0;
		if (time_s % 2 == 0) {
			send_udp_frame_flag = 1;
		}
	}
}


XStatus init_timer(unsigned long interval, u8 priority, XTimer_TickHandler handler, void * callback_ref)
{
    XStatus status;

    status = XilTickTimer_Init(&timer_inst);

    if (status != XST_SUCCESS) {
        LOG_ERROR("Tick timer initialization failed!");
        return status;
    }

    XTimer_SetInterval(interval);
    XTimer_SetHandler(handler, callback_ref, priority);


    // timer_inst->XTimer_TickIntrHandler(timer_inst, priority);

    LOG_INFO("Initialize tick timer ok.");
    return XST_SUCCESS;
}

XStatus setup_timers()
{
	return init_timer(timer_interval_ms, timer_interrupt_priority, timer_interval_handler, NULL);
}




u64 get_platform_time_s()
{
	return time_s;
}

u64 get_platform_time_ms()
{
	return time_ms;
}
