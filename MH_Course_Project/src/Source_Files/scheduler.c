/**
 * @file scheduler.c
 * @author Matt Hartnett
 * @date November 1st, 2020
 * @brief The scheduler handles events and ensures
 * all events are serviced properly.
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Libraries

//** Silicon Lab include files

//** User/developer include files
#include "scheduler.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Driver to open a scheduler for event handlers
 *
 * @details
 *	Initializes the scheduler to have no events scheduled, by utilizing a static
 *	variable that keeps track of which events need to be serviced.
 *
 * @note
 *	This function is atomic to prevent issues with interrupts changing the event
 *	scheduled bit multiple times.
 *
 ******************************************************************************/
void scheduler_open(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	event_scheduled = 0;

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *	Function to add an event to the scheduler.
 *
 * @details
 *	This function adds the event to the scheduler through an or-equals function to
 *	the static variable with the event integer. This ensures that other events are
 *	not effected by the adding of a new event.
 *
 * @note
 *	This function is atomic to prevent issues with interrupts changing the event
 *	scheduled bit multiple times.
 *
 * @param[in] event
 *   An integer that represents an event that needs to be serviced.
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	event_scheduled |= event;

	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *	Function to remove an event to the scheduler.
 *
 * @details
 *	This function removes the event to the scheduler through an and-not-equals function
 *	to the static variable with the event integer. This ensures that other events are
 *	not effected by the removing of a new event.
 *
 * @note
 *	This function is atomic to prevent issues with interrupts changing the event
 *	scheduled bit multiple times.
 *
 * @param[in] event
 *   An integer that represents an event that needs to be serviced.
 *
 ******************************************************************************/
void remove_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	event_scheduled &= ~event;

	CORE_EXIT_CRITICAL();

}

/***************************************************************************//**
 * @brief
 *	Returns the which events are scheduled.
 *
 * @details
 *	Returns the value of the static variable that keeps track of the scheduled
 *	events
 *
 * @note
 *	No need to be atomic, as it only returns and does not change the events that
 *	are scheduled.
 *
 ******************************************************************************/
uint32_t get_scheduled_events(void){
	return event_scheduled;
}
