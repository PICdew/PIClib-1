/*!
 @file		pic32_realtime.c
 @brief		Real Time keeping for pic32 based on hardware Timer 1

 @version	0.1.2
 @note		The sysTick Rate is also the upTime Update Rate, keep it below a second to use the realTime system
			accurately.
 @todo		- Make the RTCC System functionnal again

 @date		March 2th 2012
 @author	Laurence DV
*/

// ################## Includes ################## //
#include "pic32_realtime.h"
// ############################################## //


// ################## Defines ################### //

// ############################################## //


// ################## Variables ################# //

//SysTick system
U32 sysTick = 0;										//Number of sysTick passed
U32 sysTickValue = 0;									//Value of a sysTick (in us)

//UpTime system
U32 upTimeLast = 0;										//Last update of the upTime (in systick)
tRealTime upTime;										//Up Time in real time

//RTCC system
tRealTime rtccTime;										//Real time
#if RTCC_SYSTEM	== RTCC_SOFTWARE
U16 rtccUpdateTick;										//number of sysTick between rtccUpdate
U32 rtccTickNext;										//Next update of the rtcc (in sysTick)
U8 rtccCurrentMonthDayNb = 30;							//Number of day in the Current Month
#endif

// ############################################## //


// ############# Real-time Functions ############ //
// === Interrupt Handler ===== //
/**
* \fn		void realTimeISR(void)
* @brief	Interrupt Service Routine for the realTime system
* @note		Place it in the Timer 1 vector
* @arg		nothing
* @return	nothing
*/
/*
 void __ISR(RT_TIMER_INT_VECTOR, IPL7SOFT)realTimeISR(void)
{
	extern U32 sysTick;

	sysTick++;

	//upTimeUpdate();		//Should be done Async now

	intFastClearFlag(RT_TIMER_INT_VECTOR);
}
*/

// =========================== //


// === Timing Functions ====== //
/**
* \fn		U8 realTimeInit(U32 tickPeriod)
* @brief	Initialised the Real-Time system to a specified sysTick period
* @note		Intensive Computation, used only at init time.
*			Keep the value between 100 and 64 535 000 to be accurate and useful
*			Use the Hardware Timer 1
*			Use a dividable number by 1000 to assure precision of the software rtcc
* @arg		U32 tickPeriod		Desired period of a sysTick (in �s)
* @return	U8 errorCode		STD Error Code (return STD_EC_SUCCESS if successful)
*/
U8 realTimeInit(U32 tickPeriod)
{
	U8 wu0;

	// -- Init and set the Timer 1 -- //
	timerInit(RT_TIMER_ID,TMR_CS_PBCLK|TMR_FRZ_STOP);	//Timer 1 is based off PBCLK (and freezed in debug mode)
	timerSetOverflow(RT_TIMER_ID,tickPeriod);			//Set the overflow time to the desired sysTick period
	intFastSetPriority(RT_TIMER_INT_ID,7);				//Set the Timer 1 interrupt to max priority
	intFastSetSubPriority(RT_TIMER_INT_ID,3);
	intFastEnable(RT_TIMER_INT_ID);						//Enable Timer 1 interrupt
	// ------------------------------ //


	// -- Init the soft rtcc if needed -- //
	#if RTCC_SYSTEM == RTCC_SOFTWARE
	sysTickValue = tickPeriod;
	rtccInit();
	#endif
	// ---------------------------------- //


	// -- Reset the Up Time -- //
	for (wu0 = 0; wu0 < 9 ; wu0++)
		upTime.all[wu0] = 0;
	// ----------------------- //

	//Start the timer
	timerStart(RT_TIMER_ID);

	return STD_EC_SUCCESS;
}


/**
* \fn		void upTimeUpdate(void)
* @brief	Update the system Up Time
* @note		Need to be called in the realTimeISR
* @arg		nothing
* @return	nothing
*/
void upTimeUpdate(void)
{
	//Add the not counted Tick (in ms) to the global millis count
	upTime.millis += (sysTick - upTimeLast)*sysTickValue;

	// -- Resplit the millis value in real time -- //
	while (upTime.millis > 999)
	{
		upTime.millis -= 1000;
		upTime.sec++;

		//Update the Minutes
		while (upTime.sec > 59)
		{
			upTime.sec -= 60;
			upTime.min++;

			//Update the Hours
			while (upTime.min > 59)
			{
				upTime.min -= 60;
				upTime.hour++;

				//Update the Days
				while (upTime.hour > 23)
				{
					upTime.hour -= 24;
					upTime.day++;
				}
			}
		}
	}
	// ------------------------------------------- //
}

/**
* \fn		tRealTime* upTimeGet(void)
* @brief	Return the actual Up Time value
* @note		
* @arg		nothing
* @return	tRealTime* pUpTime	Pointer to the upTime global variable
*/
tRealTime* upTimeGet(void)
{
	return &upTime;
}
// =========================== //

// ====== RTCC Function ====== //

/**
* \fn		void rtccInit(void)
* @brief	Init the software rtcc for software and external mode
* @note		realTimeInit will define the correct sysTick value for 1 sec
*			and rtccUpdate will be called in the realTimeISR
* @arg		nothing
* @return	nothing
*/

void rtccInit(void)
{
	U8 wu0;

	// -- Reset the realTime variable -- //
	for (wu0 = 0; wu0<9 ;wu0++)
		rtccTime.all[wu0] = 0;
	// --------------------------------- //
}



void rtccUpdate(void)
{
	#if RTCC_SYSTEM	== RTCC_SOFTWARE
	//Count the new second
	upTime.sec++;

	//Update the Minutes
	if (upTime.sec > 59)
	{
		upTime.sec -= 60;
		upTime.min++;

		//Update the Hours
		if (upTime.min > 59)
		{
			upTime.min -= 60;
			upTime.hour++;

			//Update the Days
			if (upTime.hour > 23)
			{
				upTime.hour -= 24;
				upTime.day++;

				//Update the month
				if (upTime.day > (rtccCurrentMonthDayNb-1))
				{
					upTime.day -= rtccCurrentMonthDayNb;

					//Update the current month day number

					upTime.month++;

					if (upTime.month > 11)
					{
						upTime.month -= 12;
						upTime.year++;
					}
				}
			}
		}
	}
	#elif RTCC_SYSTEM == RTCC_HARDWARE

	#elif RTCC_SYSTEM == RTCC_EXTERNAL

	#endif
}


// =========================== //
// ############################################## //
