/*!
 @file		pic32_adc.c
 @brief		ADC Control lib for pic32

 @version	0.1
 @note		This lib use auto-sample time function, there is no facilities to handle the manual sample mode
 		This lib use interrupt, and assume they are correctly enabled in the main
 @todo		- Transform this for the rtos
 *		- Change adcResultPtr to be able to point different size of element

 @date		February 16th 2012
 @author	Laurence DV
*/

// ################## Includes ################## //
#include "pic32_adc.h"
// ############################################## //


// ################## Variables ################# //
extern U32 heapAvailable;
// -- Internal Safe-guard -- //
extern U32 globalDump;
U8 __adcSafeDonePtr = ADC_CONV_DONE;					//Safe place to point adcDonePtr if a NULL pointer is passed
// ------------------------- //

tADxCON1 * pADxCON1 = NULL;
tADxCON2 * pADxCON2 = NULL;
tADxCON3 * pADxCON3 = NULL;
tADxCHS * pADxCHS = NULL;
tADxCSSL * pADxCSSL = NULL;
U32 * pADxBUF = NULL;

tADCcontrol adcControl[ADC_MAX_PORT];					//Control of the ADC
//S16 adcOffsetValue[ADC_MAX_PORT];					//Calibration offset value result
//tADCState adcState[ADC_MAX_PORT];					//ADC module actual state
//U16 * adcResultPtr[ADC_MAX_PORT] = {&globalDump};			//Pointer to store the conversion result
//U8 * adcDonePtr[ADC_MAX_PORT] = {&globalDump};			//Pointer to flag when the conversions are done
//U32 adcConversionID[ADC_MAX_PORT];					//ID of the last completed conversion
// ############################################## //


// ################ ADC Functions ############### //
// ========== ISR =========== //
/**
* \fn		void adcISR(U8 adcPort)
* @brief	Interrupt Handler for any ADC port
* @note		Place this in the correct ISR in the main
* @arg		U8 adcPort				Hardware ADC ID
* @return	nothing
*/
void adcISR(U8 adcPort)
{
	U8 wu0;
	tADCcontrol * workPtr = &adcControl[adcPort];

	adcSelectPort(adcPort);
	
	switch (workPtr->state)
	{
		case ADCbusy:
		{
			// -- Averaging --------- //
			if (workPtr->averaging == ENABLE)
			{
				for (wu0 = 0; wu0 < pADxCON2->SMPI; wu0++)
					workPtr->averagingBuffer[wu0] += (pADxBUF[wu0 << 2]) - workPtr->offsetVal;

				// -- Count the sample -- //
				workPtr->averagingSampleDoneNb++;
				if (workPtr->averagingSampleDoneNb >= workPtr->averagingSampleNb)
				{
					for (wu0 = 0; wu0 < pADxCON2->SMPI; wu0++)
					{
						workPtr->resultPtr[wu0] = (workPtr->averagingBuffer[wu0]) / workPtr->averagingSampleNb;
						workPtr->averagingBuffer[wu0] = 0;	//Clear the buffer
					}
					workPtr->averagingSampleDoneNb = 0;	//Reset the sample counter

					*(workPtr->donePtr) = ADC_CONV_DONE;	//Flag the completion
				}
				// ---------------------- //
			}
			// -- Normal operation -- //
			else
			{
				// -- Save the result in the destination -- //
				for (wu0 = 0; wu0 <= pADxCON2->SMPI; wu0++)
					workPtr->resultPtr[wu0] = (pADxBUF[wu0 << 2]) - workPtr->offsetVal;
				// ---------------------------------------- //

				*(workPtr->donePtr) = ADC_CONV_DONE;		//Flag the completion
			}
			// ---------------------- //

			// -- If single shot reset idle -- //
			if (!pADxCON2->CSCNA)
				workPtr->state = ADCidle;
			else
				pADxCON1->ASAM = ENABLE;
			// ------------------------------- //

			break;
		}
		case ADCcalibration:
		{
			U32 tempCal = 0;

			// -- Save the calibration result -- //
			for (wu0 = 0; wu0 < ADC_CAL_CONV_NB; wu0++)
				tempCal += (pADxBUF[wu0 << 2]);

			workPtr->offsetVal = tempCal/ADC_CAL_CONV_NB;
			pADxCON2->OFFCAL = DISABLE;
			// --------------------------------- //

			workPtr->state = ADCidle;
			
			break;
		}
		default:
		{
			// -- Discard the result -- //
			for (wu0 = 0; wu0 <= pADxCON2->SMPI; wu0++)
				globalDump = (pADxBUF[wu0 << 2]);
			// ------------------------ //
			
			break;
		};
	}
}
// ========================== //


// === Control Functions ==== //
/**
* \fn		U8 adcSelectPort(U8 adcPort)
* @brief	Correctly point all reg pointers for a designated ADC port
* @note		Will return STD_EC_NOTFOUND if an invalid port is given
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcSelectPort(U8 adcPort)
{
	switch (adcPort)
	{
		case ADC_1:	pADxCON1 = (tADxCON1*)&AD1CON1; pADxCON2 = (tADxCON2*)&AD1CON2; pADxCON3 = (tADxCON3*)&AD1CON3; pADxCHS = (tADxCHS*)&AD1CHS; pADxCSSL = (tADxCSSL*)&AD1CSSL; pADxBUF = (U32*)&ADC1BUF0;	break;
		default :	return STD_EC_NOTFOUND;
	}
	return STD_EC_SUCCESS;
}

/**
* \fn		U8 adcInit(U8 adcPort)
* @brief	Initialize the ADC module for single mode
* @note		
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcInit(U8 adcPort)
{
	U8 errorCode = adcSelectPort(adcPort);
	tADCcontrol * workPtr = &adcControl[adcPort];

	if (errorCode == STD_EC_SUCCESS)
	{
		// -- Stop the ADC -- //
		pADxCON1->ON = 0;
		// ------------------ //

		pADxCON1->SSRC = ADC_TRIG_AUTO;		//Internal counter for trigger
		pADxCON2->BUFM = 0;			//16bit buffer
		pADxCON2->VCFG = 0;			//Set the references to AVdd and AVss
		pADxCON2->ALTS = 0;			//Use only MUX A
		pADxCON3->ADRC = ADC_CLK_PBCLK;		//Use PBClock as the clock source
		pADxCON1->FORM = ADC_FORMAT_U16;	//Format the result as a unsigned 16bit

		// -- Init control -- //
		workPtr->offsetVal = 0;
		workPtr->state = ADCconfig;
		workPtr->donePtr = &__adcSafeDonePtr;
		// ------------------ //

		// -- Start the ADC -- //
		pADxCON1->ON = 1;
		Nop();					//TODO wait ADC_BOOT_TIME
		// ------------------- //
	}

	return errorCode;
}

/**
* \fn		void adcSetInput(tADCScanInput inputMatrix)
* @brief	Enable Analog Input as such
* @note		Will configure in the correct register depending on the CPU_FAMILY
*		Don't forget to set the TRIS register as input also!
* @arg		tADCScanInput inputMatrix		Bitwise selection of input enabled
* @return	nothing
*/
void adcSetInput(tADCMuxInput inputMatrix)
{
#if CPU_FAMILY == PIC32MX1xx || CPU_FAMILY == PIC32MX2xx
#if defined (__32MX110F016B__) || (__32MX120F032B__) || (__32MX130F064B__) || (__32MX150F128B__) || (__32MX210F016B__) || (__32MX220F032B__) || (__32MX230F064B__) || (__32MX250F128B__)
	
	split32 splittedMatrix;

	splittedMatrix.all = inputMatrix;

	ANSELA = inputMatrix & (BIT1|BIT0);				//AN0,AN1
	ANSELBCLR = 0xFFFF;						//Clear all input on Port B
	ANSELBSET = ((inputMatrix >> 2) & (BIT3|BIT2|BIT1|BIT0));	//AN2, AN3, AN4, AN5
	ANSELBbits.ANSB12 = splittedMatrix.b12;				//AN12
	ANSELBbits.ANSB13 = splittedMatrix.b11;				//AN11
	ANSELBbits.ANSB14 = splittedMatrix.b10;				//AN10
	ANSELBbits.ANSB15 = splittedMatrix.b9;				//AN9

#elif defined (__32MX110F016C__) || (__32MX120F032C__) || (__32MX130F064C__) || (__32MX150F128C__) || (__32MX210F016C__) || (__32MX220F032C__) || (__32MX230F064C__) || (__32MX250F128C__)

#elif defined (__32MX110F016D__) || (__32MX120F032D__) || (__32MX130F064D__) || (__32MX150F128D__) || (__32MX210F016D__) || (__32MX220F032D__) || (__32MX230F064D__) || (__32MX250F128D__)

#endif	
#elif CPU_FAMILY == PIC32MX3xx || CPU_FAMILY == PIC32MX4xx || CPU_FAMILY == PIC32MX5xxH || CPU_FAMILY == PIC32MX5xxL || CPU_FAMILY == PIC32MX6xx || CPU_FAMILY == PIC32MX7xx
	AD1PCFG = inputMatrix;
#endif
}

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
U8 adcSetSampleRate(U8 adcPort, U32 sampleRate)
{
	U8 adcs = 0;
	U8 samc = 0;
	U8 adcON;
	U32 tempPBclock = clockGetPBCLK();
	tADCcontrol * workPtr = &adcControl[adcPort];

	U8 errorCode = adcSelectPort(adcPort);
	if (errorCode == STD_EC_SUCCESS)
	{
		// -- Hadle boundary -- //
		if (sampleRate > 1000000)
			return STD_EC_TOOLARGE;
		// -------------------- //

		// -- Stop the ADC -- //
		workPtr->state = ADCconfig;
		adcON = pADxCON1->ON;
		pADxCON1->ON = 0;
		// ------------------ //

		// -- Check if using PBCLK -- //
		if (pADxCON3->ADRC == ADC_CLK_PBCLK)
		{
			// -- Determine the minimum adcs -- //
			for (; (tempPBclock / ((adcs+1) << 1 )) > ADC_TAD_FREQ_MAX; adcs++);
			// -------------------------------- //

			// -- Check if it is possible -- //
			if ((tempPBclock / (((adcs+1) << 1 ) * (ADC_CONV_TIME+1))) >= sampleRate)	//Minimum value of SAMC is 1
			{

					// -- Test for a valid samc with the lowest adcs possible -- //
					for (; adcs <= 255; adcs++)
					{
						samc = (tempPBclock / (sampleRate * ((adcs+1) << 1 ))) - ADC_CONV_TIME;

						if ((samc < ADC_SAMC_MAX) && ((tempPBclock / ((adcs+1) << 1 )) < ADC_TAD_FREQ_MAX))		//If it is suitable exit loop
							break;
					}
					// --------------------------------------------------------- //

					// -- Save the values -- //
					pADxCON3->ADCS = adcs;
					pADxCON3->SAMC = samc;
					// --------------------- //

					// -- Verify the actual sample rate -- //
					while (adcGetSampleRate(adcPort) > ADC_SAMP_RATE_MAX)
						pADxCON3->SAMC++;
					// ----------------------------------- //
			}
			else
				errorCode = STD_EC_TOOLARGE;						//Sample rate too large for the PBCLK
			// ----------------------------- //
		}
		else
			errorCode = STD_EC_INVALID;							//Using FRC
		// -------------------------- //
	
		// -- Restore the ADC State -- //
		workPtr->state = ADCidle;
		pADxCON1->ON = adcON;
		if (adcON)
		{
			Nop();				//TODO wait ADC_BOOT_TIME
		}
		// --------------------------- //
	}
	
	return errorCode;
}

/**
* \fn		U32 adcGetSampleRate(U8 adcPort)
* @brief	Return the actual Sample Rate of the selected ADC
* @note		
* @arg		U8 adcPort				Hardware ADC ID
* @return	U32 SampleRate				Actual Sample Rate
*/
U32 adcGetSampleRate(U8 adcPort)
{
	U32 sampleRate = 0;

	if (adcSelectPort(adcPort) == STD_EC_SUCCESS)
	{
		// -- Compute the actual Sample Rate -- //
		sampleRate = clockGetPBCLK()/((((pADxCON3->ADCS)+1) <<1) * (ADC_CONV_TIME + pADxCON3->SAMC));
		// ------------------------------------ //
	}

	return sampleRate;
}

/**
* \fn		U8 adcCalibrate(U8 adcPort)
* @brief	Calibrate and offset the selected ADC
* @note		Will return STD_EC_NOTFOUND if an invalid port is given
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcCalibrate(U8 adcPort)
{
	U8 errorCode = adcSelectPort(adcPort);

	if (errorCode == STD_EC_SUCCESS)
	{
		// -- Init for calibration -- //
		pADxCON2->OFFCAL = ENABLE;			//Set for calib
		pADxCON2->SMPI = ADC_CAL_CONV_NB-1;		//Do the specified amount of conversion
		pADxCON1->ASAM = 1;				//Auto mode
		pADxCON1->CLRASAM = ENABLE;			//Stop after the interrupt
		pADxCON1->SAMP = ENABLE;			//Start the conversion

		adcControl[adcPort].state = ADCcalibration;
		// -------------------------- //
	}
	return errorCode;
}

/**
* \fn		U8 adcSetScan(U8 adcPort, U32 scanInput)
* @brief	Set the enabled input matrix for the scan mode of the selected ADC port
* @note		Use tADCScanInput for the correct input selection
* @arg		U8 adcPort				Hardware ADC ID
* @arg		tADCScanInput scanInput			Input Matrix to enable
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcSetScanInput(U8 adcPort, tADCMuxInput scanInput)
{
	U8 errorCode = adcSelectPort(adcPort);
	if (errorCode == STD_EC_SUCCESS)
		pADxCSSL->CSSL = scanInput;		//Set the selected input
	return errorCode;
}

/**
* \fn		U32 adcGetScan(U8 adcPort)
* @brief	Return the enabled input matrix for the scan mode of the selected ADC port
* @note		Use tADCScanInput for the correct return value
* @arg		U8 adcPort				Hardware ADC ID
* @return	tADCScanInput inputMatrix		Input Matrix enabled
*/
tADCMuxInput adcGetScanInput(U8 adcPort)
{
	if (adcSelectPort(adcPort) == STD_EC_SUCCESS)
		return pADxCSSL->CSSL;
	return 0;
}

/**
* \fn		U32 adcGetScan(U8 adcPort)
* @brief	Return the number of enabled input for the scan mode of the selected ADC port
* @note		WARNING ! If using it with multiple ADC don't forget to re-select it after
*		calling this function
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 inputNb				Number of input enabled
*/
U8 adcGetScanInputeNb(U8 adcPort)
{
	U8 wu0;
	U8 inputNb = 0;

	if (adcSelectPort(adcPort) == STD_EC_SUCCESS)
	{
		for (wu0 = 0;wu0 < 16;wu0++)
		{
			if (pADxCSSL->CSSL & (BIT0 << wu0))
				inputNb++;
		}
	}

	return inputNb;
}

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
void adcEnableAveraging(U8 adcPort, U16 sampleNb, U8 inputNb)
{
	tADCcontrol * workPtr = &adcControl[adcPort];

	if (adcSelectPort(adcPort) == STD_EC_SUCCESS)
	{
		// -- Allocate the averaging buffer -- //
		workPtr->averagingBuffer = calloc(inputNb,sizeof(U32));
		if (workPtr->averagingBuffer != NULL)
		// ----------------------------------- //
		{
			// Count the allocated ram
			heapAvailable -= sizeof(U32)*inputNb;

			workPtr->averagingSampleNb = sampleNb;
			workPtr->averagingSampleDoneNb = 0;
			workPtr->averaging = ENABLE;
		}
	}
}
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
U32 adcConvert(U8 adcPort, tADCInput adcInput, U8 conversionNb, U16 * resultPtr, U8 * donePtr)
{
	U8 errorCode;
	tADCcontrol * workPtr = &adcControl[adcPort];

	errorCode = adcSelectPort(adcPort);
	if (errorCode == STD_EC_SUCCESS)
	{
		// -- Handle boundary -- //
		if (conversionNb > 16)
			conversionNb = 16;
		// --------------------- //

		// Wait for the ADC to be idle
		while (workPtr->state != ADCidle);

		// -- Select the correct channel -- //
		pADxCHS->CH0NA = 0;			//Select VrefL as the negative input
		pADxCHS->CH0SA = adcInput;
		// -------------------------------- //

		// -- Save the variables -- //
		if (donePtr == NULL)
			return STD_EC_FAIL;			//Point to a safe place
		else
			workPtr->donePtr = donePtr;		//Save the done pointer

		if (resultPtr == NULL)
			return STD_EC_FAIL;			//Point to a safe place
		else
			workPtr->resultPtr = resultPtr;		//Save the result pointer

		workPtr->state = ADCbusy;
		// ------------------------ //

		// -- Start the conversion -- //
		*(workPtr->donePtr) = ADC_CONV_BUSY;		//Set the flag as busy
		pADxCON1->ASAM = 1;				//Auto mode
		pADxCON1->CLRASAM = 1;				//Stop after SMPI nb of conversion (clear ASAM automaticaly)
		pADxCON2->SMPI = conversionNb-1;		//Set the number of conversion to do

		pADxCON1->SAMP = 1;				//Start the sampling/conversion
		// -------------------------- //
	}

	return errorCode;
}

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
U8 adcStartScan(U8 adcPort, U16 * resultPtr, U8 * donePtr)
{
	U8 errorCode;
	tADCcontrol * workPtr = &adcControl[adcPort];

	errorCode = adcSelectPort(adcPort);
	if (errorCode == STD_EC_SUCCESS)
	{
		// Wait for the ADC to be idle
		while (workPtr->state != ADCidle);

		// -- Save the variables -- //
		if (donePtr == NULL)
			return STD_EC_FAIL;			//Point to a safe place
		else
			workPtr->donePtr = donePtr;		//Save the done pointer

		if (resultPtr == NULL)
			return STD_EC_FAIL;			//Point to a safe place
		else
			workPtr->resultPtr = resultPtr;		//Save the result pointer

		workPtr->state = ADCbusy;
		// ------------------------ //

		// -- Count the number of conversion -- //
		pADxCON2->SMPI = adcGetScanInputeNb(adcPort) -1;//Set the number of conversion to do
		// ------------------------------------ //

		// -- Start the scan -- //
		*(workPtr->donePtr) = ADC_CONV_BUSY;		//Set the flag as busy
		pADxCON1->CLRASAM = ENABLE;			//Wait after the interrupt
		pADxCON2->CSCNA = ENABLE;			//Start the scan mode
		pADxCON1->ASAM = ENABLE;			//Auto mode
		//pADxCON1->SAMP = 1;				//Start the sampling/conversion
		// -------------------- //
	}
	return errorCode;
}

/**
* \fn		U8 adcStopScan(U8 adcPort)
* @brief	Stop the scan mode on the selected ADC
* @note		Will stop after the actual conversion
* @arg		U8 adcPort				Hardware ADC ID
* @return	U8 errorCode				STD Error Code (STD_EC_SUCCESS if successful)
*/
U8 adcStopScan(U8 adcPort)
{
	U8 errorCode;
	
	errorCode = adcSelectPort(adcPort);
	if (errorCode == STD_EC_SUCCESS)
	{
		pADxCON2->CSCNA = 0;				//Stop the scan
		pADxCON1->CLRASAM = 1;				//Stop the conversion at the next interrupt
	}
	return errorCode;
}
// ============================= //
// ############################################## //

