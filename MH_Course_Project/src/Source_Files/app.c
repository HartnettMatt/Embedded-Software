/**
 * @file app.c
 * @author Matt Hartnett
 * @date December 4th, 2020
 * @brief Contains application layer functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);
static char str[64];
static char c_str[] = "#TEMP C!";
static char f_str[] = "#TEMP F!";
static bool celsius = false;
//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Initialize peripherals
 *
 * @details
 *	This function is what ensures that the peripherals are setup correctly. The routine it follows
 *	is as such: configure clock tree,configure peripheral GPIO, initialize peripherals, route and verify.
 *	If interrupts were utilized in this function, they would need to be initialized here at the end of
 *	the current routine.
 *
 * @note
 *	This function has to run before the main.c loop, as it configures the devices used in the loop.
 *
 ******************************************************************************/

void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	scheduler_open();
	sleep_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
	si7021_i2c_open(SI7021_READ_CB);
	ble_open(BLE_TX_CB, BLE_RX_CB);
	add_scheduled_event(BOOT_UP_CB);
	sleep_block_mode(SYSTEM_BLOCK_EM);
}

/***************************************************************************//**
 * @brief
 *	Start the LETIMER with proper values
 *
 * @details
 *	This function's primary purpose is build an APP_LETIMER_PWM_ struct with the proper values,
 *	which are gotten from the parameters passed into the function. This struct is then passed into
 *	the letimer_pwm_open function, which will initialize the pwm LETIMER with the correct period,
 *	active period, enables, and routing. The last step is to begin counting with the letimer_start
 *	function.
 *
 * @note
 *	This function is called in the app_peripheral_setup, and is used before the main loop to configure
 *	the clock tree.
 *
 * @param[in] period
 *	Floating point value that represents the PWM period in seconds.
 *
 * @param[in] act_period
 *	Floating point value that represents the PWM active period in seconds.
 *
 * @param[in] out0_route
 *	The routing register that connects the output of LETIMER0 to the first destination.
 *
 * @param[in] out1_route
 *	The routing register that connects the output of LETIMER0 to the first destination.
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){

	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef letimer_pwm_struct;
	letimer_pwm_struct.period = period;
	letimer_pwm_struct.active_period = act_period;
	letimer_pwm_struct.out_pin_route0 = out0_route;
	letimer_pwm_struct.out_pin_route1 = out1_route;
	letimer_pwm_struct.debugRun = false;
	letimer_pwm_struct.out_pin_0_en = false;
	letimer_pwm_struct.out_pin_1_en = false;
	letimer_pwm_struct.enable = false;
	letimer_pwm_struct.comp0_irq_enable = false;
	letimer_pwm_struct.comp1_irq_enable = false;
	letimer_pwm_struct.uf_irq_enable = true;
	letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
	letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
	letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;
	letimer_pwm_open(LETIMER0, &letimer_pwm_struct);

	// letimer_start will inform the LETIMER0 peripheral to begin counting.
	letimer_start(LETIMER0, true);
}

/***************************************************************************//**
 * @brief
 *	The event handler for the LETIMER0 UF event
 *
 * @details
 *	This function removes the underflow event bit from the scheduler, and has a
 *	check to ensure that the event was successfully handled. This callback function
 *	is primarily set by the interrupt handlers, but can also be called by the completion
 *	of processing a state.
 *
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb (void){
	si7021_i2c_read(SI7021_READ_CB);
	remove_scheduled_event(LETIMER0_UF_CB);
}

/***************************************************************************//**
 * @brief
 *	The event handler for the LETIMER0 comp0 event
 *
 * @details
 *	This function removes the comp0 event bit from the scheduler, and has a
 *	check to ensure that the event was successfully handled. This callback function
 *	is primarily set by the interrupt handlers, but can also be called by the completion
 *	of processing a state.
 *
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb (void){
	remove_scheduled_event(LETIMER0_COMP0_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *	The event handler for the LETIMER0 comp1 event
 *
 * @details
 *	This function removes the comp1 event bit from the scheduler, and has a
 *	check to ensure that the event was successfully handled. This callback function
 *	is primarily set by the interrupt handlers, but can also be called by the completion
 *	of processing a state.
 *
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb (void){
	remove_scheduled_event(LETIMER0_COMP1_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *	The event handler for the Si7021 temp_complete event
 *
 * @details
 *	This function removes the temp_complete event bit from the scheduler, and based
 *	on the temperature, turns LED0 on or off. This callback function is primarily
 *	set by the interrupt handlers, but can also be called by the completion of
 *	processing a state.
 *
 ******************************************************************************/
void si7021_temp_done_evt(void){
	float temp;
	temp = si7021_temp();
	if(celsius){
		temp = (temp-32)*(5.0/9.0);
		if(temp > 30.0){
			GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		} else {
			GPIO_PinOutClear(LED0_PORT, LED0_PIN);
		}
		sprintf(str, "temp = %3.1f C\n", temp);
	} else {
		if(temp > 80.0){
			GPIO_PinOutSet(LED0_PORT, LED0_PIN);
		} else {
			GPIO_PinOutClear(LED0_PORT, LED0_PIN);
		}
		sprintf(str, "temp = %3.1f F\n", temp);
	}
	ble_write(str);
	remove_scheduled_event(SI7021_READ_CB);
}

/***************************************************************************//**
 * @brief
 *	The event handler for the boot up event
 *
 * @details
 *	This function removes the boot up event bit from the scheduler, which occurs
 *	after the app peripheral setup in app.c. This function can be used as to test
 *	the BLE module, it then tests the circular buffer, and sends several strings
 *	to be transmitted.
 *
 ******************************************************************************/
void scheduled_boot_up_cb (void){
	remove_scheduled_event(BOOT_UP_CB);

#ifdef BLE_TEST_ENABLED
	bool ble_test_ret = ble_test("MattsBLE");
	EFM_ASSERT(ble_test_ret);
	timer_delay(2000);
#endif
	circular_buff_test();
	ble_write("\nHello World\n");
	ble_write("ADC Lab\n");
	ble_write("Matt Hartnett\n");
	letimer_start(LETIMER0, true);
}


/***************************************************************************//**
 * @brief
 *	The event handler for the BLE TX event
 *
 * @details
 *	This function removes the BLE TX event bit from the scheduler, which is
 *	used to signify that the transmission over the LEUART has been successfully
 *	completed, and then it pops the next string off of the circular buffer.
 *
 ******************************************************************************/
void scheduled_ble_tx_cb (void){
	remove_scheduled_event(BLE_TX_CB);
	ble_circ_pop(false);
}


/***************************************************************************//**
 * @brief
 *	The event handler for the BLE RX event
 *
 * @details
 *	This function removes the BLE RX event bit from the scheduler, which is
 *	used to signify that a complete command (with a START and a SIG frame) has
 *	been received successfully. If the command matches with the celsius/fahrenheit
 *	command, then it begins to display in the format specified.
 *
 ******************************************************************************/
void scheduled_ble_rx_cb (void){
	remove_scheduled_event(BLE_RX_CB);
	strcpy(str, rx_str());
	if(strcmp(str, c_str) == 0){
		celsius = true;
	} else if (strcmp(str, f_str) == 0){
		celsius = false;
	}
}

