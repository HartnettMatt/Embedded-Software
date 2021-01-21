/**
 * @file i2c.c
 * @author Matt Hartnett
 * @date December 4th, 2020
 * @brief Contains all the LETIMER driver functions
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************

//** User/developer include files
#include "i2c.h"


//***********************************************************************************
// Private variables
//***********************************************************************************

static I2C_STATE_MACHINE i2c_sm;
static uint32_t	event;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void i2c_ack(void);
static void i2c_nack(void);
static void i2c_rxdatav(void);
static void i2c_mstop(void);
static void i2c_bus_reset(I2C_TypeDef *i2c);
/***************************************************************************//**
 * @brief
 *	This function resets the I2C bus for either the I2C0 or I2C1 peripheral
 *
 * @details
 * 	This routine is meant to ensure that the I2C bus is cleared, along with
 * 	the I2C interrupt flag register and the CMD register.
 *
 * @note
 * 	This function is only meant to be used within the I2C.c routines, primarily
 * 	when the peripheral is being opened in i2c_open()
 *
 *@param[in] i2c
 *   Pointer to the base peripheral address of the i2c peripheral being reset
 *
 ******************************************************************************/

static void i2c_bus_reset(I2C_TypeDef *i2c){
	uint32_t state;
	state = i2c->IEN;
	i2c->IEN = 0;
	i2c->IFC = i2c->IF;
	i2c->CMD |= I2C_CMD_CLEARTX;
	i2c->CMD |= I2C_CMD_START | I2C_CMD_STOP;
	while(!(i2c->IF & I2C_IF_MSTOP));
	i2c->IFC = i2c->IF;
	i2c->IEN = state;
	i2c->CMD |= I2C_CMD_ABORT;
}

/***************************************************************************//**
 * @brief
 * 	This function services the I2C ACK interrupt, and behaves based on the state
 * 	of the I2C state machine.
 *
 * @details
 * 	If the ACK interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an ACK is received while in a state that does not expect an ACK,
 * 	this function will get caught in an EFM_ASSERT.
 *
 * @note
 *	This function sends bus commands and advances the state machine based on the current
 *	state.
 ******************************************************************************/
static void i2c_ack(){
	switch(i2c_sm.state){
		case handshake:{
			i2c_sm.state = measure_cmd;
			i2c_sm.I2Cn->TXDATA = i2c_sm.command;
		break;
		}
		case measure_cmd:{
			i2c_sm.state = confirm_cmd;
			i2c_sm.I2Cn->CMD = I2C_CMD_START;
			i2c_sm.I2Cn->TXDATA = (i2c_sm.slave_address << 1) | true;
		break;
		}
		case confirm_cmd:{
			i2c_sm.state = RX_MS_byte;
		break;
		}
		case RX_MS_byte:{
		break;
		}
		case RX_LS_byte: {
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case end_comm:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		default:{
			EFM_ASSERT(false);
		break;
		}
	}
}
/***************************************************************************//**
 * @brief
 * 	This function services the I2C NACK interrupt, and behaves based on the state
 * 	of the I2C state machine.
 *
 * @details
 * 	If the NACK interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an NACK is received while in a state that does not expect an NACK,
 * 	this function will get caught in an EFM_ASSERT.
 *
 * @note
 *	This function sends bus commands and advances the state machine based on the current
 *	state.
 ******************************************************************************/
static void i2c_nack(){
	switch(i2c_sm.state){
		case handshake:{
			i2c_sm.I2Cn->CMD = I2C_CMD_START;
			i2c_sm.I2Cn->TXDATA = (i2c_sm.slave_address << 1) | false;
		break;
		}
		case measure_cmd:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case confirm_cmd:{
			i2c_sm.I2Cn->CMD = I2C_CMD_START;
			i2c_sm.I2Cn->TXDATA = (i2c_sm.slave_address << 1) | true;
		break;
		}
		case RX_MS_byte:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case RX_LS_byte: {
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case end_comm:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		default:{
			EFM_ASSERT(false);

		break;
		}
	}
}

/***************************************************************************//**
 * @brief
 * 	This function services the I2C RXDATAV interrupt, and behaves based on the state
 * 	of the I2C state machine.
 *
 * @details
 * 	If the RXDATAV interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an RXDATAV is received while in a state that does not expect an RXDATAV,
 * 	this function will get caught in an EFM_ASSERT.
 *
 * @note
 *	This function sends bus commands and advances the state machine based on the current
 *	state.
 ******************************************************************************/
static void i2c_rxdatav(){
	switch(i2c_sm.state){
		case handshake:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case measure_cmd:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case confirm_cmd:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case RX_MS_byte:{
			*i2c_sm.data |= i2c_sm.I2Cn->RXDATA << 8;
			i2c_sm.state = RX_LS_byte;
			i2c_sm.I2Cn->CMD = I2C_CMD_ACK;
		break;
		}
		case RX_LS_byte: {
			*i2c_sm.data |= i2c_sm.I2Cn->RXDATA;
			i2c_sm.state = end_comm;
			i2c_sm.I2Cn->CMD = I2C_CMD_NACK;
			i2c_sm.I2Cn->CMD = I2C_CMD_STOP;
		break;
		}
		case end_comm:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		default:{
			EFM_ASSERT(false);

		break;
		}
	}
}

/***************************************************************************//**
 * @brief
 * 	This function services the I2C MSTOP interrupt, and behaves based on the state
 * 	of the I2C state machine.
 *
 * @details
 * 	If the MSTOP interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an MSTOP is received while in a state that does not expect an MSTOP,
 * 	this function will get caught in an EFM_ASSERT.
 *
 * @note
 *	This function sends bus commands and advances the state machine based on the current
 *	state.
 ******************************************************************************/
static void i2c_mstop(){
	switch(i2c_sm.state){
		case handshake:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case measure_cmd:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case confirm_cmd:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case RX_MS_byte:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case RX_LS_byte: {
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case end_comm:{
			sleep_unblock_mode(I2C_EM_BLOCK);
			add_scheduled_event(event);
			i2c_sm.state = handshake;
		break;
		}
		default:{
			EFM_ASSERT(false);

		break;
		}
	}
}


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Driver to open an set an I2C peripheral
 *
 * @details
 * 	 This routine is a low level driver.  The application code calls this function
 * 	 to open one of the I2C peripherals, and the values used in the initialization
 * 	 of the I2C peripheral are provided by the application layer.
 *
 * @note
 *   This function is normally called once to initialize the peripheral and the
 *   function i2c_start() is called to turn-on or turn-off the i2c operation.
 *
 * @param[in] i2c
 *   Pointer to the base peripheral address of the i2c peripheral being opened
 *
 * @param[in] i2c_setup
 *   This is the STRUCT that the calling routine will use to set the parameters for
 *   operation
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup){
	if(i2c == I2C0){
		CMU_ClockEnable(cmuClock_I2C0, true);
		NVIC_EnableIRQ(I2C0_IRQn);
	} else if(i2c == I2C1){
		CMU_ClockEnable(cmuClock_I2C1, true);
		NVIC_EnableIRQ(I2C1_IRQn);
	} else {
		CMU_ClockEnable(cmuClock_I2C0, true);
	}

	if ((i2c->IF & 0x01) == 0){
		i2c->IFS = 0x01;
		EFM_ASSERT(i2c->IF & 0x01);
		i2c->IFC = 0x01;
	} else {
		i2c->IFC = 0x01;
		EFM_ASSERT(!(i2c->IF & 0x01));
	}

	I2C_Init_TypeDef i2c_init_struct;
	i2c_init_struct.master = i2c_setup->master;
	i2c_init_struct.enable = true;
	i2c_init_struct.clhr = i2c_setup->clhr;
	i2c_init_struct.freq = i2c_setup->freq;
	i2c_init_struct.refFreq = i2c_setup->refFreq;
	event = i2c_setup->event_def;
	I2C_Init(i2c, &i2c_init_struct);
	i2c->ROUTELOC0 = i2c_setup->scl_loc | i2c_setup->sda_loc;
	i2c->ROUTEPEN = (i2c_setup->scl_en * _I2C_ROUTEPEN_SCLPEN_MASK) |
			(i2c_setup->sda_en * _I2C_ROUTEPEN_SDAPEN_MASK);

	i2c_bus_reset(i2c);

	i2c->IEN = 0;
	i2c->IEN |= I2C_IF_ACK;
	i2c->IEN |= I2C_IF_NACK;
	i2c->IEN |= I2C_IF_RXDATAV;
	i2c->IEN |= I2C_IF_MSTOP;

}

/***************************************************************************//**
 * @brief
 *	This function begins the operation of the I2C state machine.
 *
 * @details
 *	Begins by ensuring that the I2C state machine is not busy with another operation,
 *	and upon confirmation of availability, blocks the appropriate energy mode,
 *	and then begins the state machine with the values passed through the function
 *	call.
 *
 * @note
 *	Requires that the I2C bus in not in use when called or this function will be
 *	caught in an EFM_ASSERT.
 *
 * @param[in] slave_add
 *   Pointer to the base peripheral address of the i2c slave device.
 *
 * @param[in] cmd
 *   The command being passed into the I2C slave.
 *
 * @param[in] read_data
 * 	A pointer to the data that is read off of the I2C bus
 *
 * @param[in] i2c
 * 	Pointer to the base peripheral address of the i2c peripheral being opened
 *
 * @param[in] si7021_read_cb
 * 	The scheduler event value, which is required to clear the scheduler when
 * 	the I2C state machine has terminated.
 *
 ******************************************************************************/


void i2c_start(uint32_t slave_add, uint32_t cmd,  uint32_t *read_data, I2C_TypeDef * i2c, uint32_t si7021_read_cb){
//	Check that the state machine is available
	EFM_ASSERT((i2c->STATE & _I2C_STATE_MASK) == I2C_STATE_STATE_IDLE);

	sleep_block_mode(I2C_EM_BLOCK);

	i2c_sm.state = handshake;
	i2c_sm.slave_address = slave_add;
	i2c_sm.command = cmd;
	i2c_sm.data = read_data;
	i2c_sm.read = false;
	i2c_sm.I2Cn = i2c;
	i2c_sm.callback = si7021_read_cb;

	i2c->CMD = I2C_CMD_START;
	i2c->TXDATA = (slave_add << 1) | false;

}

/***************************************************************************//**
 * @brief
 * 	This function is the interrupt handler for the I2C peripheral.
 *
 * @details
 * 	Upon receiving an interrupt, this function first clears the flag register of
 * 	that interrupt while preserving any other interrupts waiting to be serviced.
 * 	It then calls the service routine for any of the enabled I2C interrupts that need
 * 	to be serviced.
 *
 * @note
 * 	This function does not alter or service any interrupts not allowed within the
 * 	I2C interrupt enable (IEN) register.
 *
 ******************************************************************************/
void I2C0_IRQHandler(void){
	uint32_t int_flag;
	int_flag = I2C0->IF & I2C0->IEN;
	I2C0->IFC = int_flag;

	if(int_flag & I2C_IF_ACK){
		i2c_ack();
	}
	if(int_flag & I2C_IF_NACK){
		i2c_nack();
	}
	if(int_flag & I2C_IF_RXDATAV){
		i2c_rxdatav();
	}
	if(int_flag & I2C_IF_MSTOP){
		i2c_mstop();
	}
}
