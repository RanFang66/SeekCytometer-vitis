#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__


#include "xil_types.h"
#include "xil_exception.h"

#include "XStatus.h"


XStatus init_gic();

XStatus connect_interrupt(u32 intc_id, Xil_ExceptionHandler handler, void *callback_ref, u8 priority, u8 trigger);

#endif
