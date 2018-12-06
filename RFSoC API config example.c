
/***************************** Include Files ********************************/
#define ENABLE_RFDC
#define ENABLE_METAL_PRINTS
#include "xparameters.h"
#include "xrfdc.h"
#include "xrfdc_mts.h"
#include <metal/irq.h>
#include "xil_printf.h"
#include "xil_io.h"
#include "xgpio.h"
#include "xstatus.h"
#include <stdio.h>
#include "meta_log_print_1.c"


/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define __BAREMETAL__

#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID

#ifndef __BAREMETAL__
#define BUS_NAME        "platform"
#define RFDC_DEV_NAME    XPAR_XRFDC_0_DEV_NAME
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif
/************************** Function Prototypes *****************************/

int RFdcMTS_Example(XRFdc* InstancePtr);
void XRFdc_DumpCommRegs(XRFdc* InstancePtr, u32 Type, int Tile_Id);
void delay (u32 delayloop);
int RFdcCheckSettings(u16 RFdcDeviceId, u32 ExpInterpolationFactor, u32 ExpDecimationFactor, u32 ExpDecoderMode, int NumOfTiles, int NumOfBlocks);
static int CompareIntValues(u32 ExpectedVal, u32 TestVal);
int checkAdcFsmState (XRFdc* RFdcInstPtr, u32 expectedState);
int checkDdcFsmState (XRFdc* RFdcInstPtr, u32 expectedState);

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */

/****************************************************************************/
/**
*
* Main function that invokes the MTS example in this file.
*
* @param	None.
*
* @return
*		- XRFDC_SUCCESS if the example has completed successfully.
*		- XRFDC_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{

	int status;
	int Status;
	XRFdc_Config *ConfigPtr;
	XRFdc *RFdcInstPtr = &RFdcInst;

#ifndef __BAREMETAL__
	struct metal_device *device;
	struct metal_io_region *io;
	int ret = 0;
#endif

	//struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	//struct metal_init_params init_param = METAL_INIT_log_debug;

	int ADC_Tile0_01_I,ADC_Tile0_23_I,ADC_Tile1_01_I, ADC_Tile1_23_I,ADC_Tile2_01_I, ADC_Tile2_23_I,ADC_Tile3_01_I, ADC_Tile3_23_I;
	int BRAM_CTRL_data[8];
	int ADCdataTemp[8];
		/*GPIO*****************************************************************************/

	XGpio push;
	int push_check;
	XGpio_Initialize(&push,XPAR_GPIO_0_DEVICE_ID);    //AXI GPIO Initialization
	XGpio_SetDataDirection(&push,1,0xffffffff);       //AXI GPIO Set Direction, channel 1
	XGpio_SetDataDirection(&push,2,0x00000000);       //AXI GPIO Set Direction, channel 2
	/*GPIO*****************************************************************************/

///*------------------------------------------------------------------------------------------------------------------------------*/
#ifdef ENABLE_METAL_PRINTS

	struct metal_init_params init_param = {

			.log_handler	= my_metal_default_log_handler,

			.log_level		= METAL_LOG_DEBUG,

	};


	if (metal_init(&init_param)) {

		xil_printf("ERROR: Failed to run metal initialization\n");

		return XRFDC_FAILURE;

	}

#endif
///*------------------------------------------------------------------------------------------------------------------------------*/



    ConfigPtr = XRFdc_LookupConfig(RFDC_DEVICE_ID);
    if (ConfigPtr == NULL) {
		return XRFDC_FAILURE;
	}

    status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
    if (status != XRFDC_SUCCESS) {
        printf("RFdc Init Failure\n\r");
    }

/*Config internal PLL------------------------------------------------------------------------------------------------------------*/
/*Config internal PLL------------------------------------------------------------------------------------------------------------*/

	printf("\n\rChecking IP status before reset\n");
	RFdcCheckSettings(RFDC_DEVICE_ID,8,8,XRFDC_DECODER_MAX_SNR_MODE,4,4);


	XRFdc_IPStatus GetIPStatus0;
	int i;
	u32 ActiveIPStatus[4]={0,0,0,0};
	u32 All_ActiveIPStatus=0;
	while(!All_ActiveIPStatus){
		XRFdc_GetIPStatus(RFdcInstPtr, &GetIPStatus0);
		for (i = 0; i < 4; i++) {             //four ADC tiles.
          if(i<2) {                           //two DAC tiles
          printf("DAC Tile %d state: %x\n", i, GetIPStatus0.DACTileStatus[i].TileState);
          if(GetIPStatus0.DACTileStatus[i].TileState==15) ActiveIPStatus[2*i]=1;
          else ActiveIPStatus[2*i]=0;
          }

          printf("ADC Tile %d state: %x\n", i, GetIPStatus0.ADCTileStatus[i].TileState);
          if(GetIPStatus0.ADCTileStatus[i].TileState==15) ActiveIPStatus[2*i+1]=1;
          else ActiveIPStatus[2*i+1]=0;
        }
	All_ActiveIPStatus=ActiveIPStatus[0]&ActiveIPStatus[1]&ActiveIPStatus[2]&ActiveIPStatus[3];
	printf("All_ActiveIPStatus %x\n", All_ActiveIPStatus);
    delay(1000);
	}

//	/*ThresholdSetting*****************************************************************************/
//		XRFdc_Threshold_Settings Threshold_Settings;
//		Threshold_Settings.UpdateThreshold = XRFDC_UPDATE_THRESHOLD_BOTH;
//		Threshold_Settings.ThresholdMode[0] = XRFDC_TRSHD_HYSTERISIS;
//		Threshold_Settings.ThresholdUnderVal[0] = 164;
//		Threshold_Settings.ThresholdOverVal[0] = 200;
//		Threshold_Settings.ThresholdAvgVal[0] = 10;
//
//		Threshold_Settings.ThresholdMode[1] = XRFDC_TRSHD_HYSTERISIS;  //using threshold1 to freeze the calibration
//		Threshold_Settings.ThresholdUnderVal[1] = 50;//411;
//		Threshold_Settings.ThresholdOverVal[1] = 100;//420;
//		Threshold_Settings.ThresholdAvgVal[1] =50; //49152;
//		for(int Tile=0; Tile<4; Tile++){
//			for(int Block=0;Block<2;Block++){
//				XRFdc_SetThresholdSettings(RFdcInstPtr, Tile, Block, &Threshold_Settings);
//			}
//		}
//
//		XRFdc_Threshold_Settings GETThreshold_Settings;
//		XRFdc_GetThresholdSettings(RFdcInstPtr, 0, 0, &GETThreshold_Settings);
//	    xil_printf("--------------------------------------------------------------------------------\r\n");
//	    xil_printf("\r\n");
//	    printf("TILE%1d BLOCK%1d : UpdateThreshold = %x\n", 0, 0, GETThreshold_Settings.UpdateThreshold);
//
//	    printf("TILE%1d BLOCK%1d : ThresholdMode[0] = %x\n", 0, 0, GETThreshold_Settings.ThresholdMode[0]);
//	    xil_printf("TILE%1d BLOCK%1d : ThresholdUnderVal[0] = %d\r\n", 0, 0, GETThreshold_Settings.ThresholdUnderVal[0]);
//	    xil_printf("TILE%1d BLOCK%1d : ThresholdOverVal[0] = %d\r\n", 0, 0, GETThreshold_Settings.ThresholdOverVal[0]);
//	    xil_printf("TILE%1d BLOCK%1d : ThresholdAvgVal[0] = %d\r\n", 0, 0, GETThreshold_Settings.ThresholdAvgVal[0]);
//
//	    printf("TILE%1d BLOCK%1d : ThresholdMode[1] = %x\n", 0, 0, GETThreshold_Settings.ThresholdMode[1]);
//	    xil_printf("TILE%1d BLOCK%1d : ThresholdUnderVal[1] = %d\r\n", 0, 0, GETThreshold_Settings.ThresholdUnderVal[1]);
//	    xil_printf("TILE%1d BLOCK%1d : ThresholdOverVal[1] = %d\r\n", 0, 0, GETThreshold_Settings.ThresholdOverVal[1]);
//	    xil_printf("TILE%1d BLOCK%1d : ThresholdAvgVal[1] = %d\r\n", 0, 0, GETThreshold_Settings.ThresholdAvgVal[1]);
//
//	    xil_printf("\r\n");
//	    xil_printf("--------------------------------------------------------------------------------\r\n");


	/*ThresholdSetting*****************************************************************************/
	//XRFdc_StartUp(RFdcInstPtr, XRFDC_ADC_TILE, 0);

	//printf("\n\rChecking IP status After reset\n");
	//RFdcCheckSettings(RFDC_DEVICE_ID,8,8,XRFDC_DECODER_MAX_SNR_MODE,4,4);

	XRFdc_Mixer_Settings *SetADCMixer_Settings;
	SetADCMixer_Settings = malloc(1 * 4 * sizeof *SetADCMixer_Settings);
	SetADCMixer_Settings[0].Freq          = -633.92;           //ADC NCO frequency 630MHz
	SetADCMixer_Settings[0].PhaseOffset   = 0.0;
	SetADCMixer_Settings[0].EventSource   = XRFDC_EVNT_SRC_SYSREF;
	SetADCMixer_Settings[0].FineMixerMode = XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX;
	SetADCMixer_Settings[0].CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
	SetADCMixer_Settings[0].CoarseMixMode = XRFDC_COARSE_MIX_MODE_C2C_C2R;
	SetADCMixer_Settings[0].FineMixerScale = 1;

    printf("Setting ADC Mixer by using XRFdc_SetMixerSettings...\r\n");
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 1, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 1, 1, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 2, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 2, 1, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 3, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 3, 1, &SetADCMixer_Settings[0]);
//    Status = XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_DAC_TILE, 0, 0); // DDC Block0
//    Status = XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_DAC_TILE, 0, 1); // DDC Block1
//    XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0); // DDC Block0
//    Status = XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1); // DDC Block1
//	XRFdc_UpdateEvent(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0, XRFDC_EVENT_MIXER);

	xil_printf("Get mixer Setting\r\n");
	XRFdc_Mixer_Settings GetMixer_Settings[4];

    for(int Tile=0; Tile<4; Tile++){
    	for (int Block=0; Block<2; Block++){
    		Status =  XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixer_Settings[2*Tile+Block]);
								  	if (Status != XST_SUCCESS) {
							        xil_printf("Getting Fine Mixer failed\r\n");
								  		return XST_FAILURE;
								  	}

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("CURRENT Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("\r\n");
						        printf("TILE%1d BLOCK%1d : ADC : FREQ = 0x%08x\n", Tile, Block, GetMixer_Settings[2*Tile+Block].Freq);
						        printf("TILE%1d BLOCK%1d : ADC : PHASE OFFSET = %f\n", Tile, Block, GetMixer_Settings[2*Tile+Block].PhaseOffset);
						        xil_printf("TILE%1d BLOCK%1d : ADC : EVENT SOURCE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].EventSource);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FINE MIXER MODE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER FREQ = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixFreq);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER Mode = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FineMixerScale = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerScale);

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("Updating Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("\r\n");
    	}

    }

//	/*Dump common registers for debug*********************************************************************************************/
//    printf("\n\rDumpCommRegs for DAC Tile0\n\r");
//    XRFdc_DumpCommRegs(RFdcInstPtr, XRFDC_DAC_TILE, 0);
//    printf("\n\rDumpCommRegs for DAC Tile1\n\r");
//    XRFdc_DumpCommRegs(RFdcInstPtr, XRFDC_DAC_TILE, 1);
//
//    //Disconnect clock here to check offset 0x94 register
//
//    printf("\n\rDumpCommRegs for DAC Tile0\n\r");
//    XRFdc_DumpCommRegs(RFdcInstPtr, XRFDC_DAC_TILE, 0);
//    printf("\n\rDumpCommRegs for DAC Tile1\n\r");
//    XRFdc_DumpCommRegs(RFdcInstPtr, XRFDC_DAC_TILE, 1);
//    /*Dump common registers for debug*********************************************************************************************/

#ifndef __BAREMETAL__
	ret = metal_device_open(BUS_NAME, RFDC_DEV_NAME, &device);
	if (ret) {
		printf("ERROR: Failed to open device a0000000.usp_rf_data_converter.\n");
		return XRFDC_FAILURE;
	}

	/* Map RFDC device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		printf("ERROR: Failed to map RFDC regio for %s.\n",
			  device->name);
		return XRFDC_FAILURE;
	}
	RFdcInstPtr->device = device;
	RFdcInstPtr->io = io;
#endif

//	printf("\n\rChecking IP status Before MTS\n");
//	RFdcCheckSettings(RFDC_DEVICE_ID,8,8,XRFDC_DECODER_MAX_SNR_MODE,4,4);

	//delay(81900);

	xil_printf("\r\n Get mixer Setting After delay and Before MTS\r\n");
//		XRFdc_Mixer_Settings GetMixer_Settings;
//		int Tile=0;
//		int Block=0;
//		int Status;
    for(int Tile=0; Tile<4; Tile++){
    	for (int Block=0; Block<2; Block++){
    		Status =  XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixer_Settings[2*Tile+Block]);
								  	if (Status != XST_SUCCESS) {
							        xil_printf("Getting Fine Mixer failed\r\n");
								  		return XST_FAILURE;
								  	}

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("CURRENT Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("\r\n");
						        printf("TILE%1d BLOCK%1d : ADC : FREQ = 0x%08x\n", Tile, Block, GetMixer_Settings[2*Tile+Block].Freq);
						        printf("TILE%1d BLOCK%1d : ADC : PHASE OFFSET = %f\n", Tile, Block, GetMixer_Settings[2*Tile+Block].PhaseOffset);
						        xil_printf("TILE%1d BLOCK%1d : ADC : EVENT SOURCE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].EventSource);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FINE MIXER MODE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER FREQ = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixFreq);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER Mode = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FineMixerScale = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerScale);

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("Updating Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("\r\n");
    	}

    }

	printf("\n\rRFdc MTS Example Test\r\n");
	status = RFdcMTS_Example(RFdcInstPtr);

    /* ADC MTS Settings */
    XRFdc_MultiConverter_Sync_Config ADC_Sync_Config0;

    /* DAC MTS Settings */
    XRFdc_MultiConverter_Sync_Config DAC_Sync_Config0;


    XRFdc_MultiConverter_Init (&DAC_Sync_Config0, 0, 0);
    DAC_Sync_Config0.Tiles = 0x3;	/* Sync DAC tiles 0 and 1 */

    /* Initialize ADC MTS Settings */
    XRFdc_MultiConverter_Init (&ADC_Sync_Config0, 0, 0);
    ADC_Sync_Config0.Tiles = 0xF;	/* Sync ADC tiles 0, 1,2,3 */


	XRFdc_MTS_Sysref_Config(RFdcInstPtr, &DAC_Sync_Config0, &ADC_Sync_Config0, 0);

	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0); // DDC Block0
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 1, 0); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 1, 1); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 2, 0); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 2, 1); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 3, 0); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 3, 1); // DDC Block

	XRFdc_MTS_Sysref_Config(RFdcInstPtr, &DAC_Sync_Config0, &ADC_Sync_Config0, 1);


	xil_printf("\r\n Get mixer Setting After MTS\r\n");

	/*------------------------------------------------------------------------------------------------------*/

	/* change NCO frequency*/


	SetADCMixer_Settings = malloc(1 * 4 * sizeof *SetADCMixer_Settings);
	SetADCMixer_Settings[0].Freq          = -633.92;
	SetADCMixer_Settings[0].PhaseOffset   = 0.0;
	SetADCMixer_Settings[0].EventSource   = XRFDC_EVNT_SRC_SYSREF;
	SetADCMixer_Settings[0].FineMixerMode = XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX;
	SetADCMixer_Settings[0].CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
	SetADCMixer_Settings[0].CoarseMixMode = XRFDC_COARSE_MIX_MODE_C2C_C2R;
	SetADCMixer_Settings[0].FineMixerScale = 1;

    printf("Setting ADC Mixer by using XRFdc_SetMixerSettings...\r\n");
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 1, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 1, 1, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 2, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 2, 1, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 3, 0, &SetADCMixer_Settings[0]);
	XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, 3, 1, &SetADCMixer_Settings[0]);
//    Status = XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_DAC_TILE, 0, 0); // DDC Block0
//    Status = XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_DAC_TILE, 0, 1); // DDC Block1
//    XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0); // DDC Block0
//    Status = XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1); // DDC Block1
//	XRFdc_UpdateEvent(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0, XRFDC_EVENT_MIXER);

	xil_printf("Get mixer Setting\r\n");

    for(int Tile=0; Tile<4; Tile++){
    	for (int Block=0; Block<2; Block++){
    		Status =  XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixer_Settings[2*Tile+Block]);
								  	if (Status != XST_SUCCESS) {
							        xil_printf("Getting Fine Mixer failed\r\n");
								  		return XST_FAILURE;
								  	}

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("CURRENT Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("\r\n");
						        metal_log(METAL_LOG_DEBUG,"TILE%1d BLOCK%1d : ADC : FREQ = %.4f\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].Freq);
						        metal_log(METAL_LOG_DEBUG,"TILE%1d BLOCK%1d : ADC : PHASE OFFSET = %.4f\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].PhaseOffset);
						        xil_printf("TILE%1d BLOCK%1d : ADC : EVENT SOURCE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].EventSource);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FINE MIXER MODE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER FREQ = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixFreq);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER Mode = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FineMixerScale = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerScale);

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("Updating Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("\r\n");
    	}

    }

//	printf("\n\rChecking IP status Before MTS\n");
//	RFdcCheckSettings(RFDC_DEVICE_ID,8,8,XRFDC_DECODER_MAX_SNR_MODE,4,4);

	//delay(81900);

	xil_printf("\r\n Get mixer Setting After delay and Before MTS\r\n");
//		XRFdc_Mixer_Settings GetMixer_Settings;
//		int Tile=0;
//		int Block=0;
//		int Status;
    for(int Tile=0; Tile<4; Tile++){
    	for (int Block=0; Block<2; Block++){
    		Status =  XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixer_Settings[2*Tile+Block]);
								  	if (Status != XST_SUCCESS) {
							        xil_printf("Getting Fine Mixer failed\r\n");
								  		return XST_FAILURE;
								  	}

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("CURRENT Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("\r\n");
						        metal_log(METAL_LOG_DEBUG,"TILE%1d BLOCK%1d : ADC : FREQ = %.4f\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].Freq);
						        metal_log(METAL_LOG_DEBUG,"TILE%1d BLOCK%1d : ADC : PHASE OFFSET = %.4f\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].PhaseOffset);
						        xil_printf("TILE%1d BLOCK%1d : ADC : EVENT SOURCE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].EventSource);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FINE MIXER MODE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER FREQ = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixFreq);
						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER Mode = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixMode);
						        xil_printf("TILE%1d BLOCK%1d : ADC : FineMixerScale = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerScale);

						        xil_printf("\r\n");
						        xil_printf("--------------------------------------------------------------------------------\r\n");
						        xil_printf("Updating Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
						        xil_printf("\r\n");
    	}

    }



    XRFdc_MultiConverter_Init (&DAC_Sync_Config0, 0, 0);
    DAC_Sync_Config0.Tiles = 0x3;	/* Sync DAC tiles 0 and 1 */

    /* Initialize ADC MTS Settings */
    XRFdc_MultiConverter_Init (&ADC_Sync_Config0, 0, 0);
    ADC_Sync_Config0.Tiles = 0xF;	/* Sync ADC tiles 0, 1,2,3 */


	XRFdc_MTS_Sysref_Config(RFdcInstPtr, &DAC_Sync_Config0, &ADC_Sync_Config0, 0);
    delay(100);
//	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0); // DDC Block0
//	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1); // DDC Block
//	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 1, 0); // DDC Block
//	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 1, 1); // DDC Block
	//XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 2, 0); // DDC Block
	//XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 2, 1); // DDC Block
	//XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 3, 0); // DDC Block
	//XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 3, 1); // DDC Block

	XRFdc_MTS_Sysref_Config(RFdcInstPtr, &DAC_Sync_Config0, &ADC_Sync_Config0, 1);

/*freeze/un-freeze test-------------------------------------------------------------------------------------------------------*/
//
//
//	int counter_getMixer[2]={0,0};
//	u32 Reg_read_Cfg0RegVal;
//	u32 Reg_read_Cfg1RegVal;
//
//	for(int k=0; k<1; k++){     //get mix setting N times.
//
//	xil_printf("Get mixer Setting :%d\r\n", k);
//
//
//	//XGpio_DiscreteWrite(&push, 2, 0); // Freeze =1;
//	delay(100);
//	//XGpio_DiscreteWrite(&push, 2, 1); // Freeze =0;
//
//	XRFdc_GetIPStatus(RFdcInstPtr, &GetIPStatus0);
//			for (i = 0; i < 2; i++) {         // Only Enalbe two ADC tiles.
////	          if(i<2) {
////	          printf("After freeze and un-freeze calibration, DAC Tile %d state: %x\n", i, GetIPStatus0.DACTileStatus[i].TileState);
////	          }
//	        printf("After freeze and un-freeze calibration, ADC Tile %d state: %x\n", i, GetIPStatus0.ADCTileStatus[i].TileState);
//	        }
//
//    for(int Tile=0; Tile<4; Tile++){
//    	for (int Block=0; Block<2; Block++){
//
//    		Status =  XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixer_Settings[2*Tile+Block]);
//								  	if (Status != XST_SUCCESS) {
//							        xil_printf("Getting Fine Mixer failed\r\n");
//								  		return XST_FAILURE;
//								  	}
//
//
//									XRFdc_GetIPStatus(RFdcInstPtr, &GetIPStatus0);
//									printf("After freeze/un-freeze calibration and GetMixerSettings, ADC Tile %d state: %x\n", Tile, GetIPStatus0.ADCTileStatus[Tile].TileState);
//
//						        xil_printf("\r\n");
//						        Reg_read_Cfg0RegVal=XRFdc_ReadReg16(RFdcInstPtr, XRFDC_ADC_TILE_DRP_ADDR(Tile) +XRFDC_BLOCK_ADDR_OFFSET(Block),XRFDC_ADC_MXR_CFG0_OFFSET);
//						        Reg_read_Cfg1RegVal=XRFdc_ReadReg16(RFdcInstPtr, XRFDC_ADC_TILE_DRP_ADDR(Tile) +XRFDC_BLOCK_ADDR_OFFSET(Block),XRFDC_ADC_MXR_CFG1_OFFSET);
//						        xil_printf("TILE%1d BLOCK%1d :Cfg0RegVal 0x%x\r\n",Tile, Block, Reg_read_Cfg0RegVal);
//						        xil_printf("TILE%1d BLOCK%1d :Cfg1RegVal 0x%x\r\n",Tile, Block, Reg_read_Cfg1RegVal);
//						        if(Reg_read_Cfg0RegVal==0x924) counter_getMixer[0]+=1;
//						        if(Reg_read_Cfg0RegVal==0x924) counter_getMixer[1]+=1;
//						        xil_printf("--------------------------------------------------------------------------------\r\n");
//						        xil_printf("CURRENT Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
//						        xil_printf("--------------------------------------------------------------------------------\r\n");
//						        xil_printf("\r\n");
//						        metal_log(METAL_LOG_DEBUG,"TILE%1d BLOCK%1d : ADC : FREQ = %.4f\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].Freq);
//						        metal_log(METAL_LOG_DEBUG,"TILE%1d BLOCK%1d : ADC : PHASE OFFSET = %.4f\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].PhaseOffset);
//						        xil_printf("TILE%1d BLOCK%1d : ADC : EVENT SOURCE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].EventSource);
//						        xil_printf("TILE%1d BLOCK%1d : ADC : FINE MIXER MODE = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerMode);
//						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER FREQ = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixFreq);
//						        xil_printf("TILE%1d BLOCK%1d : ADC : COARSE MIXER Mode = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].CoarseMixMode);
//						        xil_printf("TILE%1d BLOCK%1d : ADC : FineMixerScale = %d\r\n", Tile, Block, GetMixer_Settings[2*Tile+Block].FineMixerScale);
//
//						        xil_printf("\r\n");
//						        xil_printf("--------------------------------------------------------------------------------\r\n");
//						        xil_printf("Updating Mixer Settings for TILE%1d BLOCK%1d : ADC\r\n", Tile, Block);
//						        xil_printf("\r\n");
//    	}
//
//    }
//
//    delay(100);
//}
//
//	  XGpio_DiscreteWrite(&push, 2, 0);
//	  xil_printf("counter_getMixer0 %d\r\n",counter_getMixer[0]);
//	  xil_printf("counter_getMixer1 %d\r\n",counter_getMixer[1]);

/*freeze/un-freeze test-------------------------------------------------------------------------------------------------------*/


    XRFdc_MultiConverter_Init (&DAC_Sync_Config0, 0, 0);
    DAC_Sync_Config0.Tiles = 0x3;	/* Sync DAC tiles 0 and 1 */

    /* Initialize ADC MTS Settings */
    XRFdc_MultiConverter_Init (&ADC_Sync_Config0, 0, 0);
    ADC_Sync_Config0.Tiles = 0xF;	/* Sync ADC tiles 0,1,2,3 */


	XRFdc_MTS_Sysref_Config(RFdcInstPtr, &DAC_Sync_Config0, &ADC_Sync_Config0, 0);
	delay(100);

	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 0); // DDC Block0
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 0, 1); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 1, 0); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 1, 1); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 2, 0); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 2, 1); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 3, 0); // DDC Block
	XRFdc_ResetNCOPhase(RFdcInstPtr, XRFDC_ADC_TILE, 3, 1); // DDC Block

	XRFdc_MTS_Sysref_Config(RFdcInstPtr, &DAC_Sync_Config0, &ADC_Sync_Config0, 1);



	/*Trig to capture the ADC data---------------------------------------------------------------------------------------------------------------------*/

		printf("Waiting for Triggering the ADC data capture by SW14[8] DIP0 on ZCU111 \r\n");
		xil_printf(" ADC_Tile0_01_I,ADC_Tile0_23_I,ADC_Tile1_01_I, ADC_Tile1_23_I,ADC_Tile2_01_I, ADC_Tile2_23_I,ADC_Tile3_01_I, ADC_Tile3_23_I\r\n");
		while(1)
		{
			push_check= XGpio_DiscreteRead(&push,1);
			if (push_check)
			{
				printf("Trigger the ADC data capture \r\n");
				break;
			}
		}
		delay(8190);
	    for(int i=1; i<2049; i++){    // the first data(i=0) is error

	    	BRAM_CTRL_data[0]=Xil_In32(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[1]=Xil_In32(XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[2]=Xil_In32(XPAR_AXI_BRAM_CTRL_2_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[3]=Xil_In32(XPAR_AXI_BRAM_CTRL_3_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[4]=Xil_In32(XPAR_AXI_BRAM_CTRL_4_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[5]=Xil_In32(XPAR_AXI_BRAM_CTRL_5_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[6]=Xil_In32(XPAR_AXI_BRAM_CTRL_6_S_AXI_BASEADDR+4*i);
			BRAM_CTRL_data[7]=Xil_In32(XPAR_AXI_BRAM_CTRL_7_S_AXI_BASEADDR+4*i);
			
	    	ADCdataTemp[0]=BRAM_CTRL_data[0]>>16;
	    	if(BRAM_CTRL_data[0] & 0x00008000)
	    	{
	    		ADC_Tile1_01_I=-((~BRAM_CTRL_data[0]&0x0000FFFF)+1);     // ADC_Tile1_01_I from BRAM_CTRL_data Low-16bit

	    	}
	    	else
	    	{ADC_Tile1_01_I=BRAM_CTRL_data[0]&0x0000FFFF;}
          /*-------------------------------------------------------------------------------------------------------*/

	    	if(ADCdataTemp[0] & 0x00008000)                           
	    	{
	    		ADC_Tile0_01_I=-((~ADCdataTemp[0]&0x0000FFFF)+1);       // ADC_Tile0_01_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile0_01_I=ADCdataTemp[0]&0x0000FFFF;}
           /*-------------------------------------------------------------------------------------------------------*/
			    	
			ADCdataTemp[2]=BRAM_CTRL_data[2]>>16;

	    	if(ADCdataTemp[2] & 0x00008000)
	    	{
	    		ADC_Tile0_23_I=-((~ADCdataTemp[2]&0x0000FFFF)+1);  // ADC_Tile0_23_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile0_23_I=ADCdataTemp[2]&0x0000FFFF;}
		   /*-------------------------------------------------------------------------------------------------------*/
		   	ADCdataTemp[3]=BRAM_CTRL_data[3]>>16;

	    	if(ADCdataTemp[3] & 0x00008000)
	    	{
	    		ADC_Tile1_23_I=-((~ADCdataTemp[3]&0x0000FFFF)+1);  // ADC_Tile1_23_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile1_23_I=ADCdataTemp[3]&0x0000FFFF;}
		   /*-------------------------------------------------------------------------------------------------------*/
		   	ADCdataTemp[4]=BRAM_CTRL_data[4]>>16;

	    	if(ADCdataTemp[4] & 0x00008000)
	    	{
	    		ADC_Tile2_01_I=-((~ADCdataTemp[4]&0x0000FFFF)+1);  // ADC_Tile2_01_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile2_01_I=ADCdataTemp[4]&0x0000FFFF;}
		   /*-------------------------------------------------------------------------------------------------------*/
		   	ADCdataTemp[5]=BRAM_CTRL_data[5]>>16;

	    	if(ADCdataTemp[5] & 0x00008000)
	    	{
	    		ADC_Tile2_23_I=-((~ADCdataTemp[5]&0x0000FFFF)+1);  // ADC_Tile2_23_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile2_23_I=ADCdataTemp[5]&0x0000FFFF;}
		   /*-------------------------------------------------------------------------------------------------------*/
		   
		   	ADCdataTemp[6]=BRAM_CTRL_data[6]>>16;

	    	if(ADCdataTemp[6] & 0x00008000)
	    	{
	    		ADC_Tile3_01_I=-((~ADCdataTemp[6]&0x0000FFFF)+1);  // ADC_Tile3_01_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile3_01_I=ADCdataTemp[6]&0x0000FFFF;}
		   /*-------------------------------------------------------------------------------------------------------*/
		   
		   	ADCdataTemp[7]=BRAM_CTRL_data[7]>>16;

	    	if(ADCdataTemp[7] & 0x00008000)
	    	{
	    		ADC_Tile3_23_I=-((~ADCdataTemp[7]&0x0000FFFF)+1);  // ADC_Tile3_23_I from BRAM_CTRL_data High-16bit

	    	}
	    	else
	    	{ADC_Tile3_23_I=ADCdataTemp[7]&0x0000FFFF;}
		   /*-------------------------------------------------------------------------------------------------------*/
		   

	    	//xil_printf("The value at address %x is %d,	 0x%x\n", XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR+4*i, ADCdata_value,ADCdata&0x0000FFFF);

			xil_printf("%d,%d,%d,%d,%d,%d,%d,%d\n",ADC_Tile0_01_I,ADC_Tile0_23_I,ADC_Tile1_01_I, ADC_Tile1_23_I,ADC_Tile2_01_I, ADC_Tile2_23_I,ADC_Tile3_01_I, ADC_Tile3_23_I);
	    	//xil_printf("%d,%d\n",ADCdata_value1,ADCdata_value0); // ADCdata_value1 is ADC00 I data; ADCdata_value0 is ADC10 I data;
	    	//xil_printf("The value at address %x is %x\n", XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR+4*i, Xil_In16(XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR+4*i));
	    }
	/*Trig to capture the ADC data****************************************************************************************************************************/
    /* HQ: shutdown and reset */
#if 0
	xil_printf("Shutdown RFDC...\r\n");
	for(int Tile=0; Tile<2; Tile++){
		XRFdc_Shutdown(RFdcInstPtr, XRFDC_ADC_TILE, Tile);
		XRFdc_Shutdown(RFdcInstPtr, XRFDC_DAC_TILE, Tile);
	}
	xil_printf("Done.\r\n");
#endif
//	for(int Tile=0; Tile<4; Tile++){
//	if (Tile<2) {XRFdc_Shutdown(RFdcInstPtr, XRFDC_ADC_TILE, Tile);}
//	XRFdc_Shutdown(RFdcInstPtr, XRFDC_DAC_TILE, Tile);
//	}
////	xil_printf("System reset after delay 2 seconds.\r\n");
////	usleep(2000000);
////	unsigned int uiVal=0;
////	uiVal=Xil_In32(0xFF5E0218);
////	xil_printf("Reg[0xFF5E0218]=0x%x\r\n", uiVal);
////	xil_printf("Trigger system reset by write register...\r\n");
////	Xil_Out32(0xFF5E0218, uiVal & 0x10);
////	return 0;
}

/****************************************************************************/
/**
*
* This function runs a MTS test on the RFSoC data converter device using the
* driver APIs.
* This function does the following tasks:
*	- Initialize the RFdc device driver instance
*	- Test MTS feature.
*
* @param	RFdcDeviceId is the XPAR_<XRFDC_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XRFDC_SUCCESS if the example has completed successfully.
*		- XRFDC_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int RFdcMTS_Example(XRFdc* RFdcInstPtr)
{
	int status_adc, status_dac, i;
	u32 factor;
    printf("=== RFdc Initialized - Running Multi-tile Sync ===\n");

    /* ADC MTS Settings */
    XRFdc_MultiConverter_Sync_Config ADC_Sync_Config;

    /* DAC MTS Settings */
    XRFdc_MultiConverter_Sync_Config DAC_Sync_Config;

    /* Run MTS for the ADC & DAC */
    printf("\n=== Run DAC Sync ===\n");

    XRFdc_MTS_RMW_DRP(RFdcInstPtr, XRFDC_ADC_TILE_DRP_ADDR(1) + XRFDC_HSCOM_ADDR,  0xB0, 0x0F, 0x01);
    XRFdc_MTS_RMW_DRP(RFdcInstPtr, XRFDC_ADC_TILE_DRP_ADDR(3) + XRFDC_HSCOM_ADDR,  0xB0, 0x0F, 0x01);

    /* Initialize DAC MTS Settings */
    XRFdc_MultiConverter_Init (&DAC_Sync_Config, 0, 0);
    DAC_Sync_Config.Tiles = 0x3;	/* Sync DAC tiles 0 and 1 */

    status_dac = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_DAC_TILE,
					&DAC_Sync_Config);
    if(status_dac == XRFDC_MTS_OK)
	printf("INFO : DAC Multi-Tile-Sync completed successfully\n");
    else printf("INFO : DAC Multi-Tile-Sync Failed\n");

    printf("\n=== Run ADC Sync ===\n");

    /* Initialize ADC MTS Settings */
    XRFdc_MultiConverter_Init (&ADC_Sync_Config, 0, 0);
    ADC_Sync_Config.Tiles = 0xF;	/* Sync ADC tiles 0, 1,2,3 */

    status_adc = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_ADC_TILE,
					&ADC_Sync_Config);
    if(status_adc == XRFDC_MTS_OK)
	printf("INFO : ADC Multi-Tile-Sync completed successfully\n");
    else printf("INFO : ADC Multi-Tile-Sync Failed\n");


    /*
     * Report Overall Latency in T1 (Sample Clocks) and
     * Offsets (in terms of PL words) added to each FIFO
     */
     printf("\n\n=== Multi-Tile Sync Report ===\n");
     for(i=0; i<4; i++) {
         if((1<<i)&DAC_Sync_Config.Tiles) {
                 XRFdc_GetInterpolationFactor(RFdcInstPtr, i, 0, &factor);
                 printf("DAC%d: Latency(T1) =%3d, Adjusted Delay"
				 "Offset(T%d) =%3d\n", i, DAC_Sync_Config.Latency[i],
						 factor, DAC_Sync_Config.Offset[i]);
         }
     }
     for(i=0; i<4; i++) {
         if((1<<i)&ADC_Sync_Config.Tiles) {
                 XRFdc_GetDecimationFactor(RFdcInstPtr, i, 0, &factor);
                 printf("ADC%d: Latency(T1) =%3d, Adjusted Delay"
				 "Offset(T%d) =%3d\n", i, ADC_Sync_Config.Latency[i],
						 factor, ADC_Sync_Config.Offset[i]);
         }
     }

    return 0;
}


void XRFdc_DumpCommRegs(XRFdc* InstancePtr, u32 Type, int Tile_Id)
{
                u32 BlockId;
                u32 Offset;
                u32 BaseAddr;
                u32 ReadReg;
                u16 NoOfTiles;
                u16 Index;
//                u16 IsBlockAvail;
//                u32 Block;
                int Tile;

                if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
                                NoOfTiles = 4;
                } else {
                                NoOfTiles = 1;
                }
                Tile = Tile_Id;
                for (Index = 0U; Index < NoOfTiles; Index++) {
                                if (Tile == XRFDC_SELECT_ALL_TILES) {
                                                Tile_Id = Index;
                                }
                                BlockId=7;
                                if (Type == XRFDC_ADC_TILE) {
                                                //metal_log(METAL_LOG_DEBUG, "\n ADC%d%d:: \r\n", Tile_Id, BlockId);
                                                printf( "\n ADC%d%d:: \r\n", Tile_Id, BlockId);
                                                BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(BlockId);
                                } else {
                                                //metal_log(METAL_LOG_DEBUG, "\n DAC%d%d:: \r\n", Tile_Id, BlockId);
                                				printf("\n DAC%d%d:: \r\n", Tile_Id, BlockId);
                                                BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(BlockId);
                                }
                                for (Offset = 0x0; Offset <= 0x148; Offset += 0x4) {
                                                ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, Offset);
                                                //metal_log(METAL_LOG_DEBUG, "\n offset = 0x%x and Value = 0x%x \t",Offset, ReadReg);
                                                printf( "\n offset = 0x%x and Value = 0x%x \t",Offset, ReadReg);
                                }

                }
}

int RFdcCheckSettings(u16 RFdcDeviceId, u32 ExpInterpolationFactor, u32 ExpDecimationFactor, u32 ExpDecoderMode, int NumOfTiles, int NumOfBlocks)
{

	int Status;

	u16 Tile;
	u16 Block;
	XRFdc_Config *ConfigPtr;
	XRFdc *RFdcInstPtr = &RFdcInst;

	u32 GetInterpolationFactor;
	u32 GetDecimationFactor;
	u32 GetDecoderMode;

  XRFdc_IPStatus GetIPStatus;
	u32 Type;
  XRFdc_BlockStatus GetBlockStatus;
  int activeDacTiles = 0;
  int activeAdcTiles = 0;

	ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

  delay(20000);  //add by WeiZhou

  activeDacTiles = getActiveDacTiles(RFdcInstPtr);
  activeAdcTiles = getActiveAdcTiles(RFdcInstPtr);

	for (Tile = 0; Tile < NumOfTiles; Tile++) {
		for (Block = 0; Block < NumOfBlocks; Block++) {
      Type = XRFDC_DAC_TILE;
      if (Tile < activeDacTiles && Block < 4) {
	      Status =  XRFdc_GetBlockStatus(RFdcInstPtr, Type, Tile, Block, &GetBlockStatus);
			  if (Status != XST_SUCCESS) {
		      xil_printf("DAC XRFdc_GetBlockStatus failed\r\n");
			  	return XST_FAILURE;
        } else {
	        xil_printf("\r\n");
	        xil_printf("--------------------------------------------------------------------------------\r\n");
	        xil_printf("GETTING BLOCK STATUS\r\n");
	        xil_printf("--------------------------------------------------------------------------------\r\n");
	        xil_printf("\r\n");
	        xil_printf("TILE%1d BLOCK%1d STATUS : DAC : Sampling Freq        : %lf\n", Tile, Block, GetBlockStatus.SamplingFreq);
		      xil_printf("TILE%1d BLOCK%1d STATUS : DAC : AnalogDataPathStatus : %d\r\n", Tile, Block, GetBlockStatus.AnalogDataPathStatus);
		      xil_printf("TILE%1d BLOCK%1d STATUS : DAC : DigitalDataPathStatus: %d\r\n", Tile, Block, GetBlockStatus.DigitalDataPathStatus);
		      xil_printf("TILE%1d BLOCK%1d STATUS : DAC : DataPathClocksStatus : %d\r\n", Tile, Block, GetBlockStatus.DataPathClocksStatus);
		      xil_printf("TILE%1d BLOCK%1d STATUS : DAC : IsFIFOFlagsEnabled   : %d\r\n", Tile, Block, GetBlockStatus.IsFIFOFlagsEnabled);
		      xil_printf("TILE%1d BLOCK%1d STATUS : DAC : IsFIFOFlagsAsserted  : %d\r\n", Tile, Block, GetBlockStatus.IsFIFOFlagsAsserted);
			  }

			  if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
			  	Status = XRFdc_GetInterpolationFactor(RFdcInstPtr, Tile, Block, &GetInterpolationFactor);

			  	if (Status != XST_SUCCESS) {
		        xil_printf("XRFdc_GetInterpolationFactor failed\r\n");
			  		return XST_FAILURE;
			  	}

			  	Status = CompareIntValues(ExpInterpolationFactor, GetInterpolationFactor);

			  	if (Status != XST_SUCCESS) {
		        xil_printf("FAIL: EXPECTED INTERPOLATE FACTOR = %d, RETURNED INTERPOLATE FACTOR = %d\r\n", ExpInterpolationFactor, GetInterpolationFactor);
			  	  return XST_FAILURE;
          } else {
		        xil_printf("PASS: EXPECTED INTERPOLATE FACTOR = %d, RETURNED INTERPOLATE FACTOR = %d\r\n", ExpInterpolationFactor, GetInterpolationFactor);
			  	}


			  	Status = XRFdc_GetDecoderMode(RFdcInstPtr, Tile, Block, &GetDecoderMode);

			  	if (Status != XST_SUCCESS) {
		        xil_printf("XRFdc_GetDecoderMode failed\r\n");
			  		return XST_FAILURE;
			  	}

			  	Status = CompareIntValues(ExpDecoderMode, GetDecoderMode);

			  	if (Status != XST_SUCCESS) {
            if (ExpDecoderMode == XRFDC_DECODER_MAX_SNR_MODE) {
              print("FAIL: EXPECTED DECODER MODE = XRFDC_DECODER_MAX_SNR_MODE, RETURNED DECODER MODE = XRFDC_DECODER_MAX_LINEARITY_MODE\r\n");
            } else if (ExpDecoderMode == XRFDC_DECODER_MAX_LINEARITY_MODE) {
              print("FAIL: EXPECTED DECODER MODE = XRFDC_DECODER_MAX_LINEARITY_MODE, RETURNED DECODER MODE = XRFDC_DECODER_MAX_SNR_MODE\r\n");
            }
			  	  return XST_FAILURE;
          } else {
            if (ExpDecoderMode == XRFDC_DECODER_MAX_SNR_MODE) {
              print("PASS: EXPECTED DECODER MODE = XRFDC_DECODER_MAX_SNR_MODE, RETURNED DECODER MODE = XRFDC_DECODER_MAX_SNR_MODE\r\n");
            } else if (ExpDecoderMode == XRFDC_DECODER_MAX_LINEARITY_MODE) {
              print("PASS: EXPECTED DECODER MODE = XRFDC_DECODER_MAX_LINEARITY_MODE, RETURNED DECODER MODE = XRFDC_DECODER_MAX_LINEARITY_MODE\r\n");
            }
			  	}
			  }
      }

      Type = XRFDC_ADC_TILE;
      if (Tile < activeAdcTiles && Block < 2) {
	      Status =  XRFdc_GetBlockStatus(RFdcInstPtr, Type, Tile, Block, &GetBlockStatus);
			  if (Status != XST_SUCCESS) {
		      xil_printf("ADC XRFdc_GetBlockStatus failed\r\n");
			  	return XST_FAILURE;
        } else {
	        xil_printf("\r\n");
	        xil_printf("--------------------------------------------------------------------------------\r\n");
	        xil_printf("GETTING BLOCK STATUS\r\n");
	        xil_printf("--------------------------------------------------------------------------------\r\n");
	        xil_printf("\r\n");
		      printf("TILE%1d BLOCK%1d STATUS : ADC : Sampling Freq        : %f\n", Tile, Block, GetBlockStatus.SamplingFreq);
		      xil_printf("TILE%1d BLOCK%1d STATUS : ADC : AnalogDataPathStatus : %d\r\n", Tile, Block, GetBlockStatus.AnalogDataPathStatus);
		      xil_printf("TILE%1d BLOCK%1d STATUS : ADC : DigitalDataPathStatus: %d\r\n", Tile, Block, GetBlockStatus.DigitalDataPathStatus);
		      xil_printf("TILE%1d BLOCK%1d STATUS : ADC : DataPathClocksStatus : %d\r\n", Tile, Block, GetBlockStatus.DataPathClocksStatus);
		      xil_printf("TILE%1d BLOCK%1d STATUS : ADC : IsFIFOFlagsEnabled   : %d\r\n", Tile, Block, GetBlockStatus.IsFIFOFlagsEnabled);
		      xil_printf("TILE%1d BLOCK%1d STATUS : ADC : IsFIFOFlagsAsserted  : %d\r\n", Tile, Block, GetBlockStatus.IsFIFOFlagsAsserted);
			  }

			  if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
			  	Status = XRFdc_GetDecimationFactor(RFdcInstPtr, Tile, Block, &GetDecimationFactor);

			  	if (Status != XST_SUCCESS) {
		        xil_printf("XRFdc_GetDecimationFactor failed\r\n");
			  		return XST_FAILURE;
			  	}

			  	Status = CompareIntValues(ExpDecimationFactor, GetDecimationFactor);

			  	if (Status != XST_SUCCESS) {
		        xil_printf("FAIL: EXPECTED DECIMATE FACTOR = %d, RETURNED DECIMATE FACTOR = %d\r\n", ExpDecimationFactor, GetDecimationFactor);
			  	  return XST_FAILURE;
          } else {
		        xil_printf("PASS: EXPECTED DECIMATE FACTOR = %d, RETURNED DECIMATE FACTOR = %d\r\n", ExpDecimationFactor, GetDecimationFactor);
			  	}
			  }
      }
		}

		Status = XRFdc_GetIPStatus(RFdcInstPtr, &GetIPStatus);
		if (Status != XST_SUCCESS) {
		  xil_printf("XRFdc_GetIPStatus failed\r\n");
			return XST_FAILURE;
    } else {
	    xil_printf("\r\n");
	    xil_printf("--------------------------------------------------------------------------------\r\n");
	    xil_printf("GETTING Tile %d IP STATUS\r\n", Tile);
	    xil_printf("--------------------------------------------------------------------------------\r\n");
	    xil_printf("\r\n");
		  xil_printf("IP STATUS : ADC : IsEnabled       : %d\r\n", GetIPStatus.ADCTileStatus[Tile].IsEnabled);
		  xil_printf("IP STATUS : ADC : TileState       : %d\r\n", GetIPStatus.ADCTileStatus[Tile].TileState);
		  xil_printf("IP STATUS : ADC : BlockStatusMask : 0x%0X\r\n", GetIPStatus.ADCTileStatus[Tile].BlockStatusMask);
		  xil_printf("IP STATUS : ADC : PLLState        : %d\r\n", GetIPStatus.ADCTileStatus[Tile].PLLState);
		  xil_printf("IP STATUS : ADC : PowerUpState    : %d\r\n", GetIPStatus.ADCTileStatus[Tile].PowerUpState);

		  xil_printf("IP STATUS : DAC : IsEnabled       : %d\r\n", GetIPStatus.DACTileStatus[Tile].IsEnabled);
		  xil_printf("IP STATUS : DAC : TileState       : %d\r\n", GetIPStatus.DACTileStatus[Tile].TileState);
		  xil_printf("IP STATUS : DAC : BlockStatusMask : 0x%0X\r\n", GetIPStatus.DACTileStatus[Tile].BlockStatusMask);
		  xil_printf("IP STATUS : DAC : PLLState        : %d\r\n", GetIPStatus.DACTileStatus[Tile].PLLState);
		  xil_printf("IP STATUS : DAC : PowerUpState    : %d\r\n", GetIPStatus.DACTileStatus[Tile].PowerUpState);
		}
	}

	return XST_SUCCESS;
}


/****************************************************************************/
/*
*
* This function compares the two Fabric Rate variables and return 0 if
* same and returns 1 if not same.
*
* @param	SetFabricRate Fabric Rate value set.
* @param	GetFabricRate Fabric Rate value get.
*
* @return
*			- 0 if both structures are same.
*			- 1 if both structures are not same.
*
* @note		None
*
*****************************************************************************/
static int CompareIntValues(u32 ExpectedVal, u32 TestVal)
{
	if (ExpectedVal == TestVal)
		return 0;
	else
		return 1;
}

static int CompareMixerSettings(XRFdc_Mixer_Settings ExpMixer_Settings, XRFdc_Mixer_Settings RetMixer_Settings)
{
	if (abs(ExpMixer_Settings.Freq - RetMixer_Settings.Freq) <= 1.0) {  // NEED TO CHECK THIS TOLERANCE WITH DESIGNERS
	  printf("PASS: EXPECTED FREQ = %f, RETURNED FREQ = %f\n", ExpMixer_Settings.Freq, RetMixer_Settings.Freq);
  } else {
	  printf("FAIL: EXPECTED FREQ = %f, RETURNED FREQ = %f\n", ExpMixer_Settings.Freq, RetMixer_Settings.Freq);
		return XST_FAILURE;
  }

	if (abs(ExpMixer_Settings.PhaseOffset - RetMixer_Settings.PhaseOffset) <= 0.5) {  // NEED TO CHECK THIS TOLERANCE WITH DESIGNERS
	  printf("PASS: EXPECTED PHASE OFFSET = %f, RETURNED PHASE OFFSET = %f\n", ExpMixer_Settings.PhaseOffset, RetMixer_Settings.PhaseOffset);
  } else {
	  printf("FAIL: EXPECTED PHASE OFFSET = %f, RETURNED PHASE OFFSET = %f\n", ExpMixer_Settings.PhaseOffset, RetMixer_Settings.PhaseOffset);
		return XST_FAILURE;
  }

	if (ExpMixer_Settings.EventSource == RetMixer_Settings.EventSource) {
	  xil_printf("PASS: EXPECTED EVENT SOURCE = %d, RETURNED EVENT SOURCE = %d\r\n", ExpMixer_Settings.EventSource, RetMixer_Settings.EventSource);
  } else {
	  xil_printf("FAIL: EXPECTED EVENT SOURCE = %d, RETURNED EVENT SOURCE = %d\r\n", ExpMixer_Settings.EventSource, RetMixer_Settings.EventSource);
		return XST_FAILURE;
  }

	if (ExpMixer_Settings.FineMixerMode == RetMixer_Settings.FineMixerMode) {
	  xil_printf("PASS: EXPECTED FINE MIXER MODE = %d, RETURNED FINE MIXER MODE = %d\r\n", ExpMixer_Settings.FineMixerMode, RetMixer_Settings.FineMixerMode);
  } else {
	  xil_printf("FAIL: EXPECTED FINE MIXER MODE = %d, RETURNED FINE MIXER MODE = %d\r\n", ExpMixer_Settings.FineMixerMode, RetMixer_Settings.FineMixerMode);
		return XST_FAILURE;
  }

	if (ExpMixer_Settings.CoarseMixFreq == RetMixer_Settings.CoarseMixFreq) {
	  xil_printf("PASS: EXPECTED COARSE MIXER FREQ = %d, RETURNED COARSE MIXER FREQ = %d\r\n", ExpMixer_Settings.CoarseMixFreq, RetMixer_Settings.CoarseMixFreq);
  } else {
	  xil_printf("FAIL: EXPECTED COARSE MIXER FREQ = %d, RETURNED COARSE MIXER FREQ = %d\r\n", ExpMixer_Settings.CoarseMixFreq, RetMixer_Settings.CoarseMixFreq);
		return XST_FAILURE;
  }

	return XST_SUCCESS;
}


int checkDacFsmState (XRFdc* RFdcInstPtr, u32 expectedState) {
	int result = 1;
	XRFdc_IPStatus ipStatus;

    // Calling this function gets the status of the IP
    XRFdc_GetIPStatus(RFdcInstPtr, &ipStatus);

    for (int i=0; i<=3; i++) {
    	if (ipStatus.DACTileStatus[i].IsEnabled == 1) {
			xil_printf("DACTile %d Current state of DAC is: 0x%08x\n\r", i, ipStatus.DACTileStatus[i].TileState);
    		if (ipStatus.DACTileStatus[i].TileState != expectedState) {
    			result = 0;
    			xil_printf("DACTile %d state is not DONE. Current state of DAC is: 0x%08x\n\r", i, ipStatus.DACTileStatus[i].TileState);
    		}
    	}
    }
    return result;
}


int getActiveDacTiles (XRFdc* RFdcInstPtr) {
	int active = 0;
	XRFdc_IPStatus ipStatus;

    // Calling this function gets the status of the IP
    XRFdc_GetIPStatus(RFdcInstPtr, &ipStatus);
    for (int i=0; i<=3; i++) {
    	if (ipStatus.DACTileStatus[i].IsEnabled == 1) {
    		active = active + 1;
    	}
    }
    return active;
}

int getActiveAdcTiles (XRFdc* RFdcInstPtr) {
	int active = 0;
	XRFdc_IPStatus ipStatus;

    // Calling this function gets the status of the IP
    XRFdc_GetIPStatus(RFdcInstPtr, &ipStatus);
    for (int i=0; i<=3; i++) {
    	if (ipStatus.ADCTileStatus[i].IsEnabled == 1) {
    		active = active + 1;
    	}
    }
    return active;
}

int checkAdcFsmState (XRFdc* RFdcInstPtr, u32 expectedState) {
	int result = 1;
	XRFdc_IPStatus ipStatus;

    // Calling this function gets the status of the IP
    XRFdc_GetIPStatus(RFdcInstPtr, &ipStatus);

    for (int i=0; i<=3; i++) {
    	if (ipStatus.ADCTileStatus[i].IsEnabled == 1) {
			xil_printf("ADCTile %d Current state of ADC is: 0x%08x\n\r", i, ipStatus.ADCTileStatus[i].TileState);
    		if (ipStatus.ADCTileStatus[i].TileState != expectedState) {
    			xil_printf("ADCTile %d state is not DONE. Current state of ADC is: 0x%08x\n\r", i, ipStatus.ADCTileStatus[i].TileState);
    			result = 0;
    		}
    	}
    }
    return result;
}

void delay (u32 delayloop) {
	for (int i=0; i<=delayloop; i++){
		for (int j=0; j<=delayloop; j++){;}
	}

}

