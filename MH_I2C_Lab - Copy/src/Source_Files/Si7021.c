/**
 * @file Si7021.c
 * @author Matt Hartnett
 * @date October 18th, 2020
 * @brief Contains all the Si7021 driver functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "Si7021.h"


//***********************************************************************************
// Private functions
//***********************************************************************************
static uint32_t	data;
//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	This function opens the I2C peripheral with the correct values need to read
 *	a value from the SI7021.
 *
 * @details
 * 	This function calls the i2c_open() function, and uses the definitions in si7021.h
 * 	to ensure that the I2C peripheral is configured to correctly read a value from the
 * 	si7021.
 *
 * @param[in] si7021_read_cb
 *	The scheduler event value, which is required to clear the scheduler when
 * 	the I2C state machine has terminated.
 *
 ******************************************************************************/

void si7021_i2c_open(uint32_t si7021_read_cb) {
	I2C_OPEN_STRUCT i2c_si7021_struct;
	i2c_si7021_struct.enable = true;
	i2c_si7021_struct.master = true;
	i2c_si7021_struct.refFreq = I2C_RefFreq;
	i2c_si7021_struct.freq = I2C_Freq;
	i2c_si7021_struct.clhr = I2C_clhr;
	i2c_si7021_struct.scl_en = true;
	i2c_si7021_struct.sda_en = true;
	i2c_si7021_struct.scl_loc = I2C_SCL_LOC;
	i2c_si7021_struct.sda_loc = I2C_SDA_LOC;
	i2c_si7021_struct.event_def = si7021_read_cb;
	i2c_open(I2Cn, &i2c_si7021_struct);
}

/***************************************************************************//**
 * @brief
 *	This function initiates a read of the si7021 over the I2C bus.
 *
 * @details
 * 	This function calls the i2c_start() function, and uses the definitions in si7021.h
 * 	to ensure that the I2C peripheral is configured to correctly read a value from the
 * 	si7021.
 *
 * @param[in] si7021_read_cb
 *	The scheduler event value, which is required to clear the scheduler when
 * 	the I2C state machine has terminated.
 *
 ******************************************************************************/
void si7021_i2c_read(uint32_t si7021_read_cb){
	i2c_start(slave_address, temp_noHold, &data, I2Cn, si7021_read_cb);
}

/***************************************************************************//**
 * @brief
 *	This function converts the temperature data code from the si7021 to the temperature
 *	in Fahrenheit.
 *
 * @details
 * 	This function uses the equation found in the si7021 documentation to translate the
 * 	code from the si7021 peripheral into a temperature on a more useful scale.
 *
 ******************************************************************************/
float si7021_temp(void){
	float celsius = (175.72*data)/65536-46.85;
	return 32 + (9./5)*celsius;
}
