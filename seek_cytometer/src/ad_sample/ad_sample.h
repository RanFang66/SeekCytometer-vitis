
#ifndef AD_SAMPLE_H
#define AD_SAMPLE_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xil_io.h"

#define AD_SAMPLE_BASE_ADDR		XPAR_AD_SAMPLE_0_S00_AXI_BASEADDR


#define AD_SAMPLE_S00_AXI_SLV_REG0_OFFSET 0
#define AD_SAMPLE_S00_AXI_SLV_REG1_OFFSET 4
#define AD_SAMPLE_S00_AXI_SLV_REG2_OFFSET 8
#define AD_SAMPLE_S00_AXI_SLV_REG3_OFFSET 12
#define AD_SAMPLE_S00_AXI_SLV_REG4_OFFSET 16
#define AD_SAMPLE_S00_AXI_SLV_REG5_OFFSET 20
#define AD_SAMPLE_S00_AXI_SLV_REG6_OFFSET 24
#define AD_SAMPLE_S00_AXI_SLV_REG7_OFFSET 28
#define AD_SAMPLE_S00_AXI_SLV_REG8_OFFSET 32
#define AD_SAMPLE_S00_AXI_SLV_REG9_OFFSET 36
#define AD_SAMPLE_S00_AXI_SLV_REG10_OFFSET 40
#define AD_SAMPLE_S00_AXI_SLV_REG11_OFFSET 44
#define AD_SAMPLE_S00_AXI_SLV_REG12_OFFSET 48
#define AD_SAMPLE_S00_AXI_SLV_REG13_OFFSET 52
#define AD_SAMPLE_S00_AXI_SLV_REG14_OFFSET 56
#define AD_SAMPLE_S00_AXI_SLV_REG15_OFFSET 60


#define AD_SAMPLE_CONTROL_REG 		AD_SAMPLE_S00_AXI_SLV_REG0_OFFSET
#define AD_SAMPLE_DATA_LENGTH_REG	AD_SAMPLE_S00_AXI_SLV_REG1_OFFSET

#define AD_SAMPLE_START_UPLOAD		(0x00000001)
#define AD_SAMPLE_STOP_UPLOAD		(0x00000000)


/*
 *ADC defines
 */
#define ADC_BYTE           	(4)              /* ADC data byte number */
#define ADC_BITS           	(18)
#define ADC_CH_COUNT       	(8)
#define ADC_FREQUENCY		(1000000)


/**************************** Type Definitions *****************************/
/**
 *
 * Write a value to a AD_SAMPLE register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the AD_SAMPLEdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void AD_SAMPLE_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define AD_SAMPLE_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a AD_SAMPLE register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the AD_SAMPLE device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 AD_SAMPLE_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define AD_SAMPLE_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))



/************************** Function Prototypes ****************************/
void start_ad_data_upload(u32 length);


void stop_ad_data_upload();


#endif // AD_SAMPLE_H
