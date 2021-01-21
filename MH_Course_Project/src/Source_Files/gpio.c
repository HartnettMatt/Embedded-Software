/**
 * @file gpio.c
 * @author Matt Hartnett
 * @date December 4th, 2020
 * @brief Contains all the GPIO driver functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Initializes the GPIO and configures the pins.
 *
 * @details
 *	This function enables the GPIO clock in the CMU, and it sets the drive strength and
 *	pinMode for LED0, LED1, the Si7021, and the LEUART.
 *
 * @note
 *	As a initialization function, this is called in the app_peripheral_setup and runs
 *	before the main loop.
 *
 ******************************************************************************/
void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

	GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);

	//	Configure sensor enable pins
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, gpioModePushPull, true);

	//	Configure Si7021 pins
	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, gpioModeWiredAnd, true);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, gpioModeWiredAnd, true);

	// 	Configure LEUART pins
	GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, gpioModePushPull, true);
	GPIO_DriveStrengthSet(LEUART_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
	GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, gpioModeInput, true);

}
