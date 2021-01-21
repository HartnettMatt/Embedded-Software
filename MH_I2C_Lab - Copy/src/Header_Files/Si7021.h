//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_
/* System include statements */


/* Silicon Labs include statements */
//#include "em_si7021.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "scheduler.h"
#include "sleep_routines.h"
#include "i2c.h"

//***********************************************************************************
// defined files
//***********************************************************************************
//***********************************************************************************
// global variables
//***********************************************************************************
#define	I2C_0
#ifdef I2C_0
	#define		I2C_SCL_LOC  I2C_ROUTELOC0_SCLLOC_LOC15
	#define		I2C_SDA_LOC  I2C_ROUTELOC0_SDALOC_LOC15
	#define		I2Cn		 I2C0
#else
	#define		I2C_SCL_LOC  I2C_ROUTELOC0_SCLLOC_LOC19
	#define		I2C_SDA_LOC  I2C_ROUTELOC0_SDALOC_LOC19
	#define		I2Cn		 I2C1
#endif



#define		I2C_RefFreq 	0
#define		I2C_Freq		I2C_FREQ_FAST_MAX
#define		I2C_clhr		i2cClockHLRAsymetric
#define		temp_noHold	0xF3
#define		slave_address	0x40
//***********************************************************************************
// function prototypes
//***********************************************************************************
void si7021_i2c_open(uint32_t si7021_read_cb);
void si7021_i2c_read(uint32_t si7021_read_cb);
float si7021_temp(void);

#endif
