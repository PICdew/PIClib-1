/*!
 @file		pic18_pps.h
 @brief		Peripheral Pin Select lib for C18 or XC8

 @version	0.1
 @note		You can use all the available RP pins of your �C for any peripheral.
 		Use only the significant part of the names (ex: PPS_IN_INT2 would be INT2)
 @todo		

 @date		November 12th 2011
 @author	Laurence DV

 @example	ppsUnlock();
		ppsAttachIn(INT1,RP2);
		ppsAttachOut(U2TX,RP20);
		ppsLock();
 */

#ifndef _PIC18_PPS_H
#define _PIC18_PPS_H 1
#if CPU_FAMILY == PIC18Fx7Jx3

// ################## Includes ################## //
// Hardware
#include <hardware.h>

// Definition
#include <definition/stddef_megaxone.h>
#include <definition/datatype_megaxone.h>

// Dev Macro
#include <tool/splitvar_megaxone.h>
#include <tool/bitmanip_megaxone.h>
// ############################################## //


// ################## Defines ################### //
// ------ RP pins ------ //
#define RP0			0
#define RP1			1
#define RP2			2
#define RP3			3
#define RP4			4
#define RP5			5
#define RP6			6
#define RP7			7
#define RP8			8
#define RP9			9
#define RP10			10
#define RP11			11
#define RP12			12
#define RP13			13
#define RP14			14
#define RP15			15
#define RP16			16
#define RP17			17
#define RP18			18
#define RP19			19
#define RP20			20
#define RP21			21
#define RP22			22
#define RP23			23
#define RP24			24
// --------------------- //

// ------ Input PPS ------ //
#define PPS_IN_INT1		RPINR1		//External Interrupt 1 input
#define PPS_IN_INT2		RPINR2		//External Interrupt 2 input
#define PPS_IN_INT3		RPINR3		//External Interrupt 3 input
#define PPS_IN_T0CK		RPINR4		//Timer 0 Clock input
#define PPS_IN_T3CK		RPINR6		//Timer 3 Clock input
#define PPS_IN_T5CK		RPINR15		//Timer 5 Clock input
#define PPS_IN_CCP1		RPINR7		//CCP1 Capture pin
#define PPS_IN_CCP2		RPINR8		//CCP2 Capture pin
#define PPS_IN_CCP3		RPINR9		//CCP3 Capture pin
#define PPS_IN_T1G		RPINR12		//Timer 1 Gate input
#define PPS_IN_T3G		RPINR13		//Timer 3 Gate input
#define PPS_IN_T5G		RPINR14		//Timer 5 Gate input
#define PPS_IN_U2RX		RPINR16		//Eusart 2 Receive (Async)
#define PPS_IN_U2DT		RPINR16		//Eusart 2 Data input
#define PPS_IN_U2CK		RPINR17		//Eusart 2 Clock input
#define PPS_IN_SDI2		RPINR21		//SPI 2 Data input
#define PPS_IN_SCK2		RPINR22		//SPI 2 Clock input
#define PPS_IN_SS2		RPINR23		//SPI 2 Slave Select input
#define PPS_IN_FLT		RPINR24		//PWM Fault input
// ------------------------ //

// ------ Output PPS ------ //
#define PPS_OUT_DIS		0		//RP pin Disable
#define PPS_OUT_C1OUT		1		//Comparator 1 output
#define PPS_OUT_C2OUT		2		//Comparator 2 output
#define PPS_OUT_C3OUT		3		//Comparator 3 output
#define PPS_OUT_U2TX		6		//Eusart 2 Transmit (Async)
#define PPS_OUT_U2CK		6		//Eusart 2 Clock output
#define PPS_OUT_U2DT		7		//Eusart 2 Data ouput
#define PPS_OUT_SDO2		10		//SPI 2 Data output
#define PPS_OUT_SCK2		11		//SPI 2 Clock ouput
#define PPS_OUT_SSDMA		12		//SPI 2 Slave Select DMA output
#define PPS_OUT_ULPOUT		13		//Ultra Low-Power Wake-up event output
#define PPS_OUT_CCP1		14		//CCP1 Compare or PWM pin
#define PPS_OUT_P1A		14		//ECCP1 PWM pin A
#define PPS_OUT_P1B		15		//ECCP1 PWM pin B
#define PPS_OUT_P1C		16		//ECCP1 PWM pin C
#define PPS_OUT_P1D		17		//ECCP1 PWM pin D
#define PPS_OUT_CCP2		18		//CCP2 Compare or PWM pin
#define PPS_OUT_P2A		18		//ECCP2 PWM pin A
#define PPS_OUT_P2B		19		//ECCP2 PWM pin B
#define PPS_OUT_P2C		20		//ECCP2 PWM pin C
#define PPS_OUT_P2D		21		//ECCP2 PWM pin D
#define PPS_OUT_CCP3		22		//CCP3 Compare or PWM pin
#define PPS_OUT_P3A		22		//ECCP3 PWM pin A
#define PPS_OUT_P3B		23		//ECCP3 PWM pin B
#define PPS_OUT_P3C		24		//ECCP3 PWM pin C
#define PPS_OUT_P3D		25		//ECCP3 PWM pin D
// ------------------------ //
// ############################################## //


// ################# Prototypes ################# //
// === Control Functions ===== //
//IOLock control
/**
* \fn		void ppsUnlock(void)
* @brief	Unlock the PPS control reg
* @note		Will force lock before unlocking
* @arg		nothing
* @return	nothing
*/
#define ppsUnlock()		(EECON2 = 0x55); (EECON2 = 0xAA); PPSCONbits.IOLOCK = DISABLE

/**
* \fn		void ppsLock(void)
* @brief	Lock the PPS control reg
* @note		
* @arg		nothing
* @return	nothing
*/
#define ppsLock()		(EECON2 = 0x55); (EECON2 = 0xAA); PPSCONbits.IOLOCK = ENABLE

/**
* \fn		void ppsAttachIn(peripheral,pin)
* @brief	Attach an input pin to a peripheral
* @note		Arguments have no type as this is a macro.
*		Use the defined peripherals in the "Input PPS" Define section
*		IO control must be unlock prior to calling this macro
*		Use only the significant part of the define (ex: ppsAttachIn(INT2,RP0))
* @arg		peripheral			Peripheral to attach to the pin
* @arg		pin				Pin to be attached to the Peripheral
* @return	nothing
*/
#define ppsAttachIn(peripheral,pin)	((PPS_IN_##peripheral) = (pin))

/**
* \fn		void ppsAttachOut(peripheral,pin)
* @brief	Attach a peripheral to an output pin
* @note		Arguments have no type as this is a macro.
*		Use the defined peripherals in the "Output PPS" Define section
*		IO control must be unlock prior to calling this macro
*		Use only the significant part of the define (ex: ppsAttachOut(U2TX,RP0))
* @arg		peripheral			Peripheral to attach to the pin
* @arg		pin				Pin to be attached to the Peripheral
* @return	nothing
*/
#define ppsAttachOut(peripheral,pin)	((PPS_POUT_##pin) = (PPS_OUT_##peripheral))
// ========================== //
// ############################################## //


// ############### Internal Macro ############### //
#define PPS_POUT_RP0		RPOR0
#define PPS_POUT_RP1		RPOR1
#define PPS_POUT_RP2		RPOR2
#define PPS_POUT_RP3		RPOR3
#define PPS_POUT_RP4		RPOR4
#define PPS_POUT_RP5		RPOR5
#define PPS_POUT_RP6		RPOR6
#define PPS_POUT_RP7		RPOR7
#define PPS_POUT_RP8		RPOR8
#define PPS_POUT_RP9		RPOR9
#define PPS_POUT_RP10		RPOR10
#define PPS_POUT_RP11		RPOR11
#define PPS_POUT_RP12		RPOR12
#define PPS_POUT_RP13		RPOR13
#define PPS_POUT_RP14		RPOR14
#define PPS_POUT_RP15		RPOR15
#define PPS_POUT_RP16		RPOR16
#define PPS_POUT_RP17		RPOR17
#define PPS_POUT_RP18		RPOR18
#define PPS_POUT_RP19		RPOR19
#define PPS_POUT_RP20		RPOR20
#define PPS_POUT_RP21		RPOR21
#define PPS_POUT_RP22		RPOR22
#define PPS_POUT_RP23		RPOR23
#define PPS_POUT_RP24		RPOR24
// ############################################## //
#endif

#endif
