#include "interrupts.h"
#include "xparameters.h"
#include "xscugic.h"
#include "log.h"

#define INTC_BASE_ADDR				XPAR_SCUGIC_0_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR			XPAR_SCUGIC_0_DIST_BASEADDR
#define INTC_GIC_DEVICE_ID			XPAR_SCUGIC_0_DEVICE_ID


static XScuGic gic_inst;



XStatus init_gic()
{
	XScuGic_Config *intc_config;
	XStatus status;

	// intc_config = XScuGic_LookupConfigBaseAddr(INTC_DIST_BASE_ADDR);  // This interface use dist base addr to find configuration
	intc_config = XScuGic_LookupConfig(INTC_GIC_DEVICE_ID);

	if (!intc_config) {
		LOG_ERROR("Cannot find the configuration of GIC.");
		return XST_FAILURE;
	}

	status = XScuGic_CfgInitialize(&gic_inst, intc_config, intc_config->CpuBaseAddress);
	if (status != XST_SUCCESS) {
		LOG_ERROR("GIC configuration initialization failed!");
		return status;
	}
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &gic_inst);
	Xil_ExceptionEnable();

	LOG_INFO("Initialize GIC finished.");
	return XST_SUCCESS;
}


XStatus connect_interrupt(u32 intc_id, Xil_ExceptionHandler handler, void *callback_ref, u8 priority, u8 trigger)
{
    XStatus status;
    XScuGic *gic = &gic_inst;
    if (!handler) {
        LOG_ERROR("Invalid parameters to connect_interrupt.");
        return XST_INVALID_PARAM;
    }

    status = XScuGic_Connect(gic, intc_id, handler, callback_ref);
    if (status != XST_SUCCESS) {
        LOG_ERROR("Connect interrupt failed!");
        return status;
    }

    XScuGic_SetPriorityTriggerType(gic, intc_id, priority, trigger);


    XScuGic_Enable(gic, intc_id);
    LOG_INFO("Connect interrupt of interrupt-id %u OK.", intc_id);
    return XST_SUCCESS;
}
