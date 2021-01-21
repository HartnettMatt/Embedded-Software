/**
 * @file letimer.c
 * @author Matt Hartnett
 * @date December 4th, 2020
 * @brief Contains all the LETIMER driver functions
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Libraries

//** Silicon Lab include files

//** User/developer include files
#include "letimer.h"


//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t scheduled_comp0_cb;
static uint32_t scheduled_comp1_cb;
static uint32_t scheduled_uf_cb;

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Driver to open an set an LETIMER peripheral in PWM mode
 *
 * @details
 * 	 This routine is a low level driver.  The application code calls this function
 * 	 to open one of the LETIMER peripherals for PWM operation to directly drive
 * 	 GPIO output pins of the device and/or create interrupts that can be used as
 * 	 a system "heart beat" or by a scheduler to determine whether any system
 * 	 functions need to be serviced.
 *
 * @note
 *   This function is normally called once to initialize the peripheral and the
 *   function letimer_start() is called to turn-on or turn-off the LETIMER PWM
 *   operation.
 *
 * @param[in] letimer
 *   Pointer to the base peripheral address of the LETIMER peripheral being opened
 *
 * @param[in] app_letimer_struct
 *   Is the STRUCT that the calling routine will use to set the parameters for PWM
 *   operation
 *
 ******************************************************************************/
void letimer_pwm_open(LETIMER_TypeDef *letimer, APP_LETIMER_PWM_TypeDef *app_letimer_struct){
	LETIMER_Init_TypeDef letimer_pwm_values;

	unsigned int period_cnt;
	unsigned int period_active_cnt;

	/*  Initializing LETIMER for PWM mode */
	/*  Enable the routed clock to the LETIMER0 peripheral */
	if(letimer == LETIMER0){
		CMU_ClockEnable(cmuClock_LETIMER0, true);
	}
	letimer_start(letimer,false);
	letimer->IFC = LETIMER_IFC_COMP0 | LETIMER_IFC_COMP1 | LETIMER_IFC_UF;

	if(app_letimer_struct->comp0_irq_enable == true){
		letimer->IEN |= LETIMER_IEN_COMP0;
	}
	if(app_letimer_struct->comp1_irq_enable == true){
		letimer->IEN |= LETIMER_IEN_COMP1;
	}
	if(app_letimer_struct->uf_irq_enable == true){
		letimer->IEN |= LETIMER_IEN_UF;
	}

	scheduled_comp0_cb = app_letimer_struct->comp0_cb;
	scheduled_comp1_cb = app_letimer_struct->comp1_cb;
	scheduled_uf_cb = app_letimer_struct->uf_cb;
	/* Use EFM_ASSERT statements to verify whether the LETIMER clock tree is properly
	 * configured and enabled
	 * You must select a register that utilizes the clock enabled to be tested
	 * With the LETIMER regiters being in the low frequency clock tree, you must
	 * use a while SYNCBUSY loop to verify that the write of the register has propagated
	 * into the low frequency domain before reading it. */
	letimer->CMD = LETIMER_CMD_START;
	while(letimer->SYNCBUSY);
	EFM_ASSERT(letimer->STATUS & LETIMER_STATUS_RUNNING);
	letimer->CMD = LETIMER_CMD_STOP;
	while(letimer->SYNCBUSY);

	// Must reset the LETIMER counter register since enabling the LETIMER to verify that
	// the clock tree has been correctly configured to the LETIMER may have resulted in
	// the counter counting down from 0 and underflowing which by default will load
	// the value of 0xffff.  To load the desired COMP0 value quickly into this
	// register after complete initialization, it must start at 0 so that the underflow
	// will happen quickly upon enabling the LETIMER loading the desired top count from
	// the COMP0 register.

	// Reset the Counter to a know value such as 0
	letimer->CNT = 0;	// What is the register enumeration to use to specify the LETIMER Counter Register?

	// Initialize letimer for PWM operation
	letimer_pwm_values.bufTop = false;		// Comp1 will not be used to load comp0, but used to create an on-time/duty cycle
	letimer_pwm_values.comp0Top = true;		// load comp0 into cnt register when count register underflows enabling continuous looping
	letimer_pwm_values.debugRun = app_letimer_struct->debugRun;
	letimer_pwm_values.enable = app_letimer_struct->enable;
	letimer_pwm_values.out0Pol = 0;			// While PWM is not active out, idle is DEASSERTED, 0
	letimer_pwm_values.out1Pol = 0;			// While PWM is not active out, idle is DEASSERTED, 0
	letimer_pwm_values.repMode = letimerRepeatFree;	// Setup letimer for free running for continuous looping
	letimer_pwm_values.ufoa0 = letimerUFOAPwm;		// Using the HAL documentation, set to PWM mode
	letimer_pwm_values.ufoa1 = letimerUFOAPwm;		// Using the HAL documentation, set to PWM mode

	LETIMER_Init(letimer, &letimer_pwm_values);		// Initialize letimer
	while(letimer->SYNCBUSY);
	/* Calculate the value of COMP0 and COMP1 and load these control registers
	 * with the calculated values
	 */
	period_cnt = app_letimer_struct->period * LETIMER_HZ;
	period_active_cnt = app_letimer_struct->active_period * LETIMER_HZ;
	letimer->COMP0 = period_cnt;
	letimer->COMP1 = period_active_cnt;
	/* Set the REP0 mode bits for PWM operation directly since this driver is PWM specific.
	 * Datasheets are very specific and must be read very carefully to implement correct functionality.
	 * Sometimes, the critical bit of information is a single sentence out of a 30-page datasheet
	 * chapter.  Look careful in the following section of the Pearl Gecko Reference Manual,
	 * 20.3.4 Underflow Output Action, to learn how to correctly set the REP0 and REP1 bits
	 *
	 * Use the values from app_letimer_struct input argument for ROUTELOC0 and ROUTEPEN enable
	 */
	letimer->REP0 = 1;
	letimer->REP1 = 1;
	letimer->ROUTELOC0 = app_letimer_struct->out_pin_route0 | app_letimer_struct->out_pin_route1;
	letimer->ROUTEPEN = (app_letimer_struct->out_pin_0_en * _LETIMER_ROUTEPEN_OUT0PEN_MASK) |
			(app_letimer_struct->out_pin_1_en * _LETIMER_ROUTEPEN_OUT1PEN_MASK);


	/* Enable interrupts */
	if(app_letimer_struct->comp0_irq_enable || app_letimer_struct->comp1_irq_enable || app_letimer_struct->uf_irq_enable){
		NVIC_EnableIRQ(LETIMER0_IRQn);
	}

	/* Check if LETIEMR has been enabled, and enable energy mode blocking*/
	if(LETIMER_STATUS_RUNNING == 1){
		sleep_block_mode(LETIMER_EM);
	}


}

/***************************************************************************//**
 * @brief
 *	Driver to begin or stop the LETIMER peripheral and manage sleep mode.
 *
 * @details
 *	This function is mainly used to begin the LETIMER and ensure that the
 *	proper energy modes are blocked/unblocked based on whether or not the
 *	LETIMER is being stopped or started.
 *
 * @note
 *	This function has a delay built in to ensure that the LETIMER is correctly
 *	started before ending the function.
 *
 * @param[in] letimer
 *	Pointer to the base peripheral address of the LETIMER peripheral being opened
 *
 * @param[in] enable
 *	Boolean that controls if the LETIMER is being stopped or started.
 *
 ******************************************************************************/
void letimer_start(LETIMER_TypeDef *letimer, bool enable){
	if(LETIMER_STATUS_RUNNING == 0 && enable){
		sleep_block_mode(LETIMER_EM);
	}
	if(LETIMER_STATUS_RUNNING == 1 && !enable){
		sleep_unblock_mode(LETIMER_EM);
	}
	LETIMER_Enable(letimer, enable);
	if(LETIMER_STATUS_RUNNING != enable){
		while(letimer->SYNCBUSY);
	}

}

/***************************************************************************//**
 * @brief
 *	The interrupt handler for the LETIMER0.
 *
 * @details
 * 	This function first removes the interrupt bit from the IFC register, and then handles
 * 	the cause of the interrupt. For comp0 and comp1 interrupts, the event is just added to
 * 	the scheduler, but for the underflow interrupt, it adds the event to the scheduler
 * 	and advances the energy mode forward so that the processor loops through the different
 * 	energy modes.
 *
 * @note
 *	This function is mainly called off of the underflow interrupt, which occurs once
 *	every LETIMER period.
 *
 ******************************************************************************/
void LETIMER0_IRQHandler(void){
	uint32_t int_flag;
	int_flag = LETIMER0->IF & LETIMER0->IEN;
	LETIMER0->IFC = int_flag;
	if(int_flag & LETIMER_IF_COMP0){
		add_scheduled_event(scheduled_comp0_cb);
		EFM_ASSERT(!(LETIMER0->IF & LETIMER_IF_COMP0));
	}
	if(int_flag & LETIMER_IF_COMP1){
		add_scheduled_event(scheduled_comp1_cb);
		EFM_ASSERT(!(LETIMER0->IF & LETIMER_IF_COMP1));
	}
	if(int_flag & LETIMER_IF_UF){
		add_scheduled_event(scheduled_uf_cb);
//		uint32_t current;
//		current = current_block_energy_mode();
//		sleep_unblock_mode(current);
//		if(current < 4){
//			sleep_block_mode(current + 1);
//		} else {
//			sleep_block_mode(EM0);
//		}
		EFM_ASSERT(!(LETIMER0->IF & LETIMER_IF_UF));
	}
}
