//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	I2C_HG
#define	I2C_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_i2c.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "scheduler.h"
#include "sleep_routines.h"

//***********************************************************************************
// defined variables
//***********************************************************************************
typedef struct {
	bool					enable;
	bool					master;
	uint32_t				refFreq;
	uint32_t				freq;
	I2C_ClockHLR_TypeDef	clhr;
	bool					scl_en;
	bool					sda_en;
	uint32_t				scl_loc;
	uint32_t				sda_loc;
	uint32_t				event_def;
} I2C_OPEN_STRUCT ;

enum i2c_defined_states {
	handshake,
	measure_cmd,
	confirm_cmd,
	RX_MS_byte,
	RX_LS_byte,
	end_comm
};

typedef struct{
	uint32_t				state;
	uint32_t				slave_address;
	uint32_t				command; //For the Si7021 (should be 0xF3 for read temp)
	uint32_t				*data;
//	uint32_t				byte_cnt;
	bool					read;
	I2C_TypeDef *			I2Cn;
	uint32_t				callback;
} I2C_STATE_MACHINE;

#define	I2C_EM_BLOCK 	2

//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void I2C0_IRQHandler(void);
void i2c_start(uint32_t slave_add, uint32_t cmd,  uint32_t *read_data, I2C_TypeDef * i2c, uint32_t si7021_read_cb);

#endif
