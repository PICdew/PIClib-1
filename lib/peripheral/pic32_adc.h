/*!
 @file		pic32_adc.h
 @brief		ADC Control lib for pic32

 @version	0.1
 @note		
 @todo		

 @date		February 16th 2011
 @author	Laurence DV
*/


#ifndef _PIC32_ADC_H
#define _PIC32_ADC_H 1

// ################## Includes ################## //
// Hardware
#include <hardware.h>

// Lib
#include <peripheral/pic32_clock.h>
#include <peripheral/pic32_interrupt.h>

// Definition
#include <definition/stddef_megaxone.h>
#include <definition/datatype_megaxone.h>

// Dev Macro
#include <tool/splitvar_megaxone.h>
#include <tool/bitmanip_megaxone.h>
// ############################################## //


// ################## Defines ################### //
// == Flag State == //
#define ADC_CONV_DONE			1
#define ADC_CONV_BUSY			0
// ================ //

// == Timing Constraint == //
#ifndef ADC_SAMP_FREQ_MAX
	#define ADC_SAMP_FREQ_MAX	5000000			//Maximum Sampling frequency	(NOT sample rate!!!) (absolute maximum of 7.575 MHz)
#endif
#ifndef ADC_TAD_FREQ_MAX
	#define ADC_TAD_FREQ_MAX	15000000		//Maximum frequency of TAD	(absolute maximum of 15.384 MHz)
#endif
#define	ADC_CONV_TIME			12			//Conversion time in TAD
#define ADC_BOOT_TIME			2000			//Stabilisation time at boot in ns
// ======================= //

// == Calibration ======== //
#define ADC_CAL_CONV_NB			10
// ======================= //

// == Mode Selection ===== //
#define ADC_MODE_SINGLE			0x0
#define ADC_MODE_CONTINUOUS		0x1
#define ADC_MODE_ALTERNATE		0x2
// == Format Selection === //
#define ADC_FORMAT_U16			0
#define ADC_FORMAT_S16			0x1
#define ADC_FORMAT_UF16			0x2
#define ADC_FORMAT_SF16			0x3
#define ADC_FORMAT_U32			0x4
#define ADC_FORMAT_S32			0x5
#define ADC_FORMAT_UF32			0x6
#define ADC_FORMAT_SF32			0x7
// == Trigger Selection == //
#define ADC_TRIG_SAMP			0
#define ADC_TRIG_INT0			0x1
#define ADC_TRIG_TIMER3			0x2
#define ADC_TRIG_CTMU			0x3
#define ADC_TRIG_AUTO			0x7
// == Clock Selection ==== //
#define ADC_CLK_PBCLK			0
#define ADC_CLK_FRC			0x1
// ======================= //

// == ADC Port == //
#define	ADC_1				0
// ============== //
// ############################################## //


// ################# Data Type ################## //
//ADC Module State
typedef enum
{
	ADCidle,
	ADCconfig,
	ADCbusy,
	ADCcalibration,
	ADCerror,
}tADCState;

//ADC Normal mode input selection
typedef enum
{
	an0 = 0x0,
	an1 = 0x1,
	an2 = 0x2,
	an3 = 0x3,
	an4 = 0x4,
	an5 = 0x5,
	an6 = 0x6,
	an7 = 0x7,
	an8 = 0x8,
	an9 = 0x9,
	an10 = 0xA,
	an11 = 0xB,
	an12 = 0xC,
	an13 = 0xD,
	an14 = 0xE,
	an15 = 0xF,
	vrefl = 0x0,
	ctmuTemp = 0xD,
	ivref = 0xE,
	open = 0xF
}tADCInput;

//ADC Scan mode input selection
typedef enum
{
	muxAn0 = 0x1,
	muxAn1 = 0x2,
	muxAn2 = 0x4,
	muxAn3 = 0x8,
	muxAn4 = 0x10,
	muxAn5 = 0x20,
	muxAn6 = 0x40,
	muxAn7 = 0x80,
	muxAn8 = 0x100,
	muxAn9 = 0x200,
	muxAn10 = 0x400,
	muxAn11 = 0x800,
	muxAn12 = 0x1000,
	muxAn13 = 0x2000,
	muxAn14 = 0x4000,
	muxAn15 = 0x8000,
	muxVrefl = 0x1,
	muxCtmuTemp = 0x2000,
	muxIvref = 0x4000,
	muxVss = 0x8000
}tADCMuxInput;

//ADC General Control
typedef struct
{
	U8 averaging:1;
	U8 newResult:1;
	U8 :7;
	tADCState state;
	tADCState oldState;
	S16 offsetVal;
	U16 averagingSampleNb;
	U16 averagingSampleDoneNb;

	U32 * averagingBuffer;
	U8 * donePtr;
	U16 * resultPtr;

	U16 resultBuffer[ADC_FIFO_LVL];
}tADCcontrol;

// == Register Pointer == //
//ADxCON1
typedef union
{
	U32 all;
	struct
	{
		U32 DONE:1;			//ADC conversion done Flag
		U32 SAMP:1;			//ADC manual start
		U32 ASAM:1;			//ADC auto sample Enable
		U32 :1;
		U32 CLRASAM:1;			//Stop Conversion
		U32 SSRC:3;			//Conversion trigger Selection
		U32 FORM:3;			//Data output format Selection
		U32 :2;
		U32 SIDL:1;			//Stop in idle mode
		U32 :1;
		U32 ON:1;			//ADC module Enable
		U32 :16;
	};
}tADxCON1;

//ADxCON2
typedef union
{
	U32 all;
	struct
	{
		U32 ALTS:1;			//Alternate input sample mode Enable
		U32 BUFM:1;			//ADC result buffer Selection
		U32 SMPI:4;			//ADC Interrupt sequence Selection
		U32 :1;
		U32 BUFS:1;			//Buffer fill flag (only valid when BUFM = 1)
		U32 :2;
		U32 CSCNA:1;			//Input scan mode Enable
		U32 :1;
		U32 OFFCAL:1;			//Input offset calibration Enable
		U32 VCFG:3;			//Voltage ref Selection
		U32 :16;
	};
}tADxCON2;

//ADxCON3
typedef union
{
	U32 all;
	struct
	{
		U32 ADCS:8;			//ADC Conversion Clock Time
		U32 SAMC:5;			//Auto-Sample Time
		U32 :2;
		U32 ADRC:1;			//ADC Conversion Clock Source
		U32 :16;
	};
}tADxCON3;

//ADxCHS
typedef union
{
	U32 all;
	struct
	{
		U32 :16;
		U32 CH0SA:4;			//Positive input Selection for MUX A
		U32 :3;
		U32 CH0NA:1;			//Negative input Selection for MUX A
		U32 CH0SB:4;			//Positive input Selection for MUX B
		U32 :3;
		U32 CH0NB:1;			//Negative input Selection for MUX B
	};
}tADxCHS;

//ADxCSSL
typedef union
{
	U32 all;
	struct
	{
		U32 CSSL:16;			//ADC Scan mode Selection
		U32 :16;
	};
	struct
	{
		U32 CSSL0:1;
		U32 CSSL1:1;
		U32 CSSL2:1;
		U32 CSSL3:1;
		U32 CSSL4:1;
		U32 CSSL5:1;
		U32 CSSL6:1;
		U32 CSSL7:1;
		U32 CSSL8:1;
		U32 CSSL9:1;
		U32 CSSL10:1;
		U32 CSSL11:1;
		U32 CSSL12:1;
		U32 CSSL13:1;
		U32 CSSL14:1;
		U32 CSSL15:1;
		U32 :16;
	};
}tADxCSSL;

//Working Pointer Struct
typedef struct
{
	volatile tADxCON1	ADxCON1;
	volatile tADxCON2	ADxCON2;
	volatile tADxCON3	ADxCON3;
	volatile tADxCHS	ADxCHS;
	volatile tADxCSSL	ADxCSSL;
	union
	{
		volatile U32	ADCxBUF[16];
		struct
		{
			volatile U32		ADCxBUF0;
			volatile U32		ADCxBUF1;
			volatile U32		ADCxBUF2;
			volatile U32		ADCxBUF3;
			volatile U32		ADCxBUF4;
			volatile U32		ADCxBUF5;
			volatile U32		ADCxBUF6;
			volatile U32		ADCxBUF7;
			volatile U32		ADCxBUF8;
			volatile U32		ADCxBUF9;
			volatile U32		ADCxBUFA;
			volatile U32		ADCxBUFB;
			volatile U32		ADCxBUFC;
			volatile U32		ADCxBUFD;
			volatile U32		ADCxBUFE;
			volatile U32		ADCxBUFF;
		};
	};
}tADCreg;
// ====================== //
// ############################################## //


// ################# Prototypes ################# //
// ========== ISR =========== //
/**
* \fn		void adcISR(U8 adcPort)
* @brief	Interrupt Handler for any ADC port
* @note		Place this in the correct ISR in the main
* @arg		U8 adcPort				Hardware ADC ID
* @return	nothing
*/
void adcISR(U8 adcPort);
// ========================== //


// === Control Functions ==== //
/**
* \fn		U8 adcSelectPort(U8 adcPort)
* @brief	Correctly point all reg pointers for a designated ADC port
* @note		Will return STD_EC_NOTFOUND if an invalid port is given
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcSelectPort(U8 adcPort);

/**
* \fn		U8 adcInit(U8 adcPort)
* @brief	Initialize the ADC module for single mode
* @note		
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcInit(U8 adcPort);

/**
* \fn		void adcSetInput(tADCScanInput inputMatrix)
* @brief	Enable Analog Input as such
* @note		Will configure in the correct register depending of the CPU_FAMILY
*		Don't forget to set the TRIS register as input also!
* @arg		tADCScanInput inputMatrix		Bitwise selection of input enabled
* @return	nothing
*/
void adcSetInput(tADCMuxInput inputMatrix);

/**
* \fn		U8 adcSetSampleRate(U8 adcPort, U32 desiredSampleRate)
* @brief	Function to set the sample rate of the selected ADC
* @note		This function will return error if the timing constraint are not met
*		Check the family reference section 17 for details on equations
*		Return STD_EC_TOOLARGE if the desired sample rate is too large for the PBCLK
*		Return STD_EC_INVALID if the ADC is using FRC as the clock source
* @arg		U8 adcPort				Hardware ADC ID
* @arg		U32 SampleRate				Sample Rate to configure (in sample per second)
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcSetSampleRate(U8 adcPort, U32 sampleRate);

/**
* \fn		U32 adcGetSampleRate(U8 adcPort)
* @brief	Return the actual Sample Rate of the selected ADC
* @note
* @arg		U8 adcPort				Hardware ADC ID
* @return	U32 SampleRate				Actual Sample Rate
*/
U32 adcGetSampleRate(U8 adcPort);

/**
* \fn		U8 adcCalibrate(U8 adcPort)
* @brief	Calibrate and offset the selected ADC
* @note		Will return STD_EC_NOTFOUND if an invalid port is given
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcCalibrate(U8 adcPort);

/**
* \fn		U8 adcSetScan(U8 adcPort, U32 scanInput)
* @brief	Set the enabled input matrix for the scan mode of the selected ADC port
* @note		Use tADCScanInput for the correct input selection
* @arg		U8 adcPort				Hardware ADC ID
* @arg		tADCScanInput scanInput			Input Matrix to enable
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcSetScanInput(U8 adcPort, tADCMuxInput scanInput);

/**
* \fn		U32 adcGetScan(U8 adcPort)
* @brief	Return the enabled input matrix for the scan mode of the selected ADC port
* @note		Use tADCScanInput for the correct return value
* @arg		U8 adcPort				Hardware ADC ID
* @return	tADCScanInput inputMatrix		Input Matrix enabled
*/
tADCMuxInput adcGetScanInput(U8 adcPort);

/**
* \fn		U32 adcGetScan(U8 adcPort)
* @brief	Return the number of enabled input for the scan mode of the selected ADC port
* @note		WARNING ! If using it with multiple ADC don't forget to re-select it after
*		calling this function
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 inputNb				Number of input enabled
*/
U8 adcGetScanInputNb(U8 adcPort);

/**
* \fn		void adcEnableAveraging(U8 adcPort, U16 sampleNb, U8 inputNb)
* @brief	Enable the auto-averaging function of the ADC
* @note		Will convert all the enabled input and wait for the specified number of sample
*		and then average the result and save it in the resultPtr
* @arg		U8 adcPort				Hardware ADC ID
* @arg		U16 sampleNb				Number of sample to average out
* @arg		U8 inputNb				Number of input actually enabled
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
void adcEnableAveraging(U8 adcPort, U16 sampleNb, U8 inputNb);
// ========================== //

// == Conversion Functions == //
/**
* \fn		U32 adcConvert(U8 adcPort)
* @brief	Will start a specified number of conversion on a single channel
* @note		This function can jam everything...
*		Super ugly and crappy function, waiting for RTOS to be better
* @arg		U8 adcPort				Hardware ADC ID
* @arg		tADCInput adcInput			Analog input to convert
* @arg		U8 conversionNb				Number of conversion to do and round up (maximum 16)
* @arg		U16 * resultPtr				Pointer to store the result
* @arg		U8 * donePtr				Pointer to flag the completion of the conversion (=ADC_CONV_DONE when done)
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U32 adcConvert(U8 adcPort, tADCInput adcInput, U8 conversionNb, U16 * resultPtr, U8 * donePtr);

/**
* \fn		U8 adcStartScan(U8 adcPort, U16 * resultPtr, U8 * donePtr)
* @brief	Start the scan mode on the previously selected input
* @note		WARNING ! This function can jam everything...
*		The $resultPtr must have enough space to store the entire result (use adcGetScanInputNb)
*		Super ugly and crappy function, waiting for RTOS to be better
* @arg		U8 adcPort				Hardware ADC ID
* @arg		U16 * resultPtr				Pointer to store the result
* @arg		U8 * donePtr				Pointer to flag the completion of the conversion (=ADC_CONV_DONE when done)
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcStartScan(U8 adcPort, U16 * resultPtr, U8 * donePtr);

/**
* \fn		U8 adcStopScan(U8 adcPort)
* @brief	Stop the scan mode on the selected ADC
* @note		Will stop after the actual conversion
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcStopScan(U8 adcPort);
// ========================== //
// ############################################## //


// ############### Internal Define ############## //
#define	ADC_SAMC_MAX		31
#define ADC_SAMP_RATE_MAX	1000000
#define ADC_FIFO_LVL		16
// ############################################## //

#endif
