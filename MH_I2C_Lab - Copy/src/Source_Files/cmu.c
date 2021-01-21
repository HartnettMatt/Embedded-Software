/**
 * @file cmu.c
 * @author Matt Hartnett
 * @date October 18th, 2020
 * @brief Contains all the CMU driver functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Configure the clock tree.
 *
 * @details
 *	This function ensures that the clocks used by the device are initialized
 *	and enabled. It also disables the clocks that are not being used, such as
 *	LFRCO and LFXO.
 *
 * @note
 *	This function is used in app_peripheral_setup in app.c, which is called
 *	before the main.c loop. This means that it is apart of the device configuration
 *	call stack and must happen before the device begins running.
 *
 ******************************************************************************/
void cmu_open(void){

		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
		// Disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);	 // What is the enumeration required for LFRCO?

		// Disable the Low Frequency Crystal Oscillator, LFXO
		CMU_OscillatorEnable(cmuOsc_LFXO , false, false);	// What is the enumeration required for LFXO?

		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		// Route LF clock to LETIMER0 clock tree
		CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);	// What clock tree does the LETIMER0 reside on?

		// Now, you must ensure that the global Low Frequency is enabled
		CMU_ClockEnable(cmuClock_CORELE, true);	//This enumeration is found in the Lab 2 assignment
}

