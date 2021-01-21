/**************************************************************************
* @file sleep_routines.c
***************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
***************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
* @author Matt Hartnett
* @date November 1st, 2020
* @brief Contains all the sleep routine functions
 *
**************************************************************************/


#include "sleep_routines.h"

static int lowest_energy_mode[MAX_ENERGY_MODES];

/***************************************************************************//**
 * @brief
 *	Driver to open the sleep routines.
 *
 * @details
 *	Uses an array of the energy modes so that sleep modes can be marked as
 *	blocked or allowed based on their value in the array (0 = allowed, >0 =
 *	blocked). This function initializes all energy modes to be allowed.
 *
 ******************************************************************************/
void sleep_open(void){
	for(uint32_t i = 0; i < MAX_ENERGY_MODES; i++){
		lowest_energy_mode[i] = 0;
	}
}

/***************************************************************************//**
 * @brief
 *	Blocks a sleep mode from being entered
 *
 * @details
 *	Adds one to the energy mode's array value. This ensures that if other operations
 *	need the sleep mode blocked, they still have the ability to declare when the
 *	sleep mode is allowed/blocked.
 *
 * @note
 *	This function is atomic to prevent interrupts from potentially creating errors.
 *
 * @param[in] EM
 *  The energy mode in question that needs to be blocked. Values defined in the
 *  sleep_routines.h file.
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	lowest_energy_mode[EM]++;

	CORE_EXIT_CRITICAL();
	EFM_ASSERT(lowest_energy_mode[EM] < 5);
}

/***************************************************************************//**
 * @brief
 *	Removes a sleep mode block
 *
 * @details
 *	Removes one from the energy mode's array value. This ensures that if other operations
 *	need the sleep mode blocked, they still have the ability to declare when the
 *	sleep mode is allowed/blocked.
 *
 * @note
 *	This function is atomic to prevent interrupts from potentially creating errors.
 *	The array value should never fall below zero, as that represents an imbalance
 *	in blocking/allowing sleep modes.
 *
 * @param[in] EM
 *  The energy mode in question that needs to be blocked. Values defined in the
 *  sleep_routines.h file.
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	if(lowest_energy_mode[EM] > 0){
		lowest_energy_mode[EM]--;
	}

	CORE_EXIT_CRITICAL();

	EFM_ASSERT(lowest_energy_mode[EM] >= 0);
}

/***************************************************************************//**
 * @brief
 *	Decides which energy mode the processor should enter.
 *
 * @details
 *	This uses the static array to find which sleep mode is allowed, and does it
 *	in such a way as to ensure that the processor is in the lowest energy mode
 *	that it can be based on the operations occurring.
 *
 * @note
 *	This function is atomic to prevent interrupts from causing errors by changing
 *	the lowest energy mode partway through the function.
 *
 ******************************************************************************/
void enter_sleep(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	if(lowest_energy_mode[EM0] > 0){
	} else if (lowest_energy_mode[EM1] > 0){
	} else if (lowest_energy_mode[EM2] > 0){
		EMU_EnterEM1();
	} else if (lowest_energy_mode[EM3] > 0){
		EMU_EnterEM2(1);
	} else {
		EMU_EnterEM3(1);
	}

	CORE_EXIT_CRITICAL();
	return;
}

/***************************************************************************//**
 * @brief
 *	Returns the lowest energy state the processor can be operating in.
 *
 * @details
 * 	Iterates through the static array to find which energy mode is allowed based
 * 	on the values in the array. When finding where in the array the first nonzero
 * 	value is, returns the value of the iterator.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
	for(uint32_t i; i < MAX_ENERGY_MODES; i++){
		if(lowest_energy_mode[i] != 0){
			return i;
		}
	}
	return (MAX_ENERGY_MODES -1);
}
