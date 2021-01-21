/**
 * @file leuart.c
 * @author Matt Hartnett
 * @date December 4th, 2020
 * @brief LEUART driver
 *
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// private variables
//***********************************************************************************
static uint32_t	leuart_rx_cb;
static uint32_t	leuart0_tx_done_cb;
//static bool		leuart0_tx_busy;
static TX_LEUART_STATE_MACHINE tx_leuart_sm;
static RX_LEUART_STATE_MACHINE rx_leuart_sm;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void leuart_txbl(void);
static void leuart_txc(void);
static void leuart_startf(void);
static void leuart_rxdatav(void);
static void leuart_sigf(void);

/***************************************************************************//**
 * @brief
 * 	This function services the LEUART TXBL interrupt, and behaves based on the state
 * 	of the LEUART state machine.
 *
 * @details
 * 	If the LEUART interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an TSBL is received while in a state that does not expect an TXBL,
 * 	then it enters an EFM_ASSERT.
 *
 ******************************************************************************/
static void leuart_txbl(void){
	switch(tx_leuart_sm.state){
		case start:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case TXdata:{
			leuart_app_transmit_byte(tx_leuart_sm.LEUARTn, tx_leuart_sm.str[tx_leuart_sm.sent_bytes]);
			tx_leuart_sm.sent_bytes++;
			if(tx_leuart_sm.sent_bytes == tx_leuart_sm.str_len){
				tx_leuart_sm.LEUARTn->IEN &= ~LEUART_IEN_TXBL;
				tx_leuart_sm.LEUARTn->IEN |= LEUART_IEN_TXC;
				tx_leuart_sm.state = stop;
			}
		break;
		}
		case stop:{
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
 * 	This function services the LEUART TXC interrupt, and behaves based on the state
 * 	of the LEUART state machine.
 *
 * @details
 * 	If the LEUART interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an TXC is received while in a state that does not expect an TXC,
 * 	then it enters an EFM_ASSERT.
 *
 ******************************************************************************/

static void leuart_txc(void){
	switch(tx_leuart_sm.state){
		case start:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case TXdata:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case stop:{
			tx_leuart_sm.LEUARTn->IEN &= ~LEUART_IEN_TXC;
			tx_leuart_sm.busy = false;
			tx_leuart_sm.state = stop;
			add_scheduled_event(tx_leuart_sm.callback);
			sleep_unblock_mode(LEUART_TX_EM);
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
 * 	This function services the LEUART STARTF interrupt, and behaves based on the state
 * 	of the LEUART RX state machine.
 *
 * @details
 * 	If the LEUART interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an STARTF is received while in a state that does not expect an STARTF,
 * 	then it enters an EFM_ASSERT.
 *
 * @note
 * 	This is the interrupt handler that begins the RX state machine, and only occurs when
 * 	the BLE recieves a '#' character.
 *
 ******************************************************************************/
static void leuart_startf(void){
	switch(rx_leuart_sm.state){
		case RXstart:{
			rx_leuart_sm.state = RXdata;
			rx_leuart_sm.str_len = 0;
			rx_leuart_sm.str[rx_leuart_sm.str_len] = rx_leuart_sm.LEUARTn->RXDATA;
			rx_leuart_sm.str_len++;
			rx_leuart_sm.LEUARTn->IEN |= LEUART_IEN_SIGF;
			rx_leuart_sm.LEUARTn->CMD = LEUART_CMD_RXBLOCKDIS;
			rx_leuart_sm.LEUARTn->IEN |= LEUART_IEN_RXDATAV;
		break;
		}
		case RXdata:{
			rx_leuart_sm.str_len = 0;
		break;
		}
		case decode:{
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
 * 	This function services the LEUART RXDATAV interrupt, and behaves based on the state
 * 	of the LEUART RX state machine.
 *
 * @details
 * 	If the LEUART interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an RXDATAV is received while in a state that does not expect an RXDATAV,
 * 	then it enters an EFM_ASSERT.
 *
 ******************************************************************************/
static void leuart_rxdatav(void){
	switch(rx_leuart_sm.state){
		case RXstart:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case RXdata:{
			rx_leuart_sm.str[rx_leuart_sm.str_len] = rx_leuart_sm.LEUARTn->RXDATA;
			rx_leuart_sm.str_len++;
		break;
		}
		case decode:{
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
 * 	This function services the LEUART SIGF interrupt, and behaves based on the state
 * 	of the LEUART RX state machine.
 *
 * @details
 * 	If the LEUART interrupt is expected for a given state, then this function will handle
 * 	everything that needs to be done within the state machine until the next interrupt
 * 	is received. If an SIGF is received while in a state that does not expect an SIGF,
 * 	then it enters an EFM_ASSERT.
 *
 ******************************************************************************/
static void leuart_sigf(void){
	switch(rx_leuart_sm.state){
		case RXstart:{
			//impossible
			EFM_ASSERT(false);
		break;
		}
		case RXdata:{
			rx_leuart_sm.state = decode;
			rx_leuart_sm.LEUARTn->IEN &= ~LEUART_IEN_SIGF;
			rx_leuart_sm.LEUARTn->IEN &= ~LEUART_IEN_RXDATAV;
			rx_leuart_sm.LEUARTn->CMD = LEUART_CMD_RXBLOCKEN;
			rx_leuart_sm.str[rx_leuart_sm.str_len] = '\0';
			rx_leuart_sm.str_len++;
			rx_leuart_sm.state = RXstart;
			add_scheduled_event(leuart_rx_cb);
		break;
		}
		case decode:{
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



//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Configures the LEUART
 *
 * @details
 * 	Begins by ensuring the the leuart pointer is valid and configures the clock tree
 * 	and interrupts if it is valid. Has a quick test to ensure that the leuart
 * 	can be read from and written to. Finally, it configures the LEUART with
 * 	settings passed in the leuart_settings struct.
 *
 * @param[in] *leuart
 *   Pointer to the LEUART module
 *
 * @param[in] *leuart_settings
 * 	 Pointer to the LEUART_OPEN_STRUCT that contains all of the configuration
 * 	 information for the leuart.
 *
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
	if(leuart == LEUART0){
		CMU_ClockEnable(cmuClock_LEUART0, true);
		NVIC_EnableIRQ(LEUART0_IRQn);
	} else {
		//impossible
		EFM_ASSERT(false);
	}

	if ((leuart->STARTFRAME & 0x01) == 0){
		leuart->STARTFRAME = 0x01;
		EFM_ASSERT(leuart->STARTFRAME & 0x01);
		leuart->STARTFRAME = 0x01;
	} else {
		leuart->STARTFRAME = 0x01;
		EFM_ASSERT(!(leuart->STARTFRAME & 0x01));
	}

	LEUART_Init_TypeDef leuart_init_struct;
	leuart_init_struct.baudrate = leuart_settings->baudrate;
	leuart_init_struct.databits = leuart_settings->databits;
	leuart_init_struct.enable = leuart_settings->enable;
	leuart_init_struct.parity = leuart_settings->parity;
	leuart_init_struct.refFreq = leuart_settings->refFreq;
	leuart_init_struct.stopbits = leuart_settings->stopbits;
	leuart0_tx_done_cb = leuart_settings->tx_done_evt;
	leuart_rx_cb = leuart_settings->rx_done_evt;
	LEUART_Init(leuart, &leuart_init_struct);

	leuart->ROUTELOC0 = leuart_settings->rx_loc | leuart_settings->tx_loc;
	leuart->ROUTEPEN = (leuart_settings->rx_en * _LEUART_ROUTEPEN_RXPEN_MASK) |
				(leuart_settings->tx_en * _LEUART_ROUTEPEN_TXPEN_MASK);

	leuart_cmd_write(leuart, LEUART_CMD_CLEARRX);
	leuart_cmd_write(leuart, LEUART_CMD_CLEARTX);
	leuart_cmd_write(leuart, LEUART_CMD_RXEN);
	leuart_cmd_write(leuart, LEUART_CMD_TXEN);

	LEUART_Enable(leuart, leuart_init_struct.enable);
	if(leuart_settings->tx_en){
		while(!(leuart->STATUS & LEUART_STATUS_TXENS));
	}
	if(leuart_settings->rx_en){
		while(!(leuart->STATUS & LEUART_STATUS_RXENS));
	}

	rx_leuart_sm.LEUARTn = leuart;
	rx_leuart_sm.callback = leuart_rx_cb;
	rx_leuart_sm.state = RXstart;
	rx_leuart_sm.str_len = 0;
	rx_leuart_sm.LEUARTn->STARTFRAME = '#';
	rx_leuart_sm.LEUARTn->SIGFRAME = '!';
	rx_leuart_sm.LEUARTn->IEN |= LEUART_IEN_STARTF;
	leuart_rx_test();
}

/***************************************************************************//**
 * @brief Handles any interrupts from the LEUART
 *
 * @details
 * 	This function clears the interrupt from the register and then enters a
 * 	helper function based on the value of the interrupt flag. If multiple
 * 	interrupts are active, all are serviced.
 *
 ******************************************************************************/

void LEUART0_IRQHandler(void){
	uint32_t int_flag;
	int_flag = LEUART0->IF & LEUART0->IEN;
	LEUART0->IFC = int_flag;

	if(int_flag & LEUART_IF_TXBL){
		leuart_txbl();
	}
	if(int_flag & LEUART_IF_TXC){
		leuart_txc();
	}
	if(int_flag & LEUART_IF_STARTF){
		leuart_startf();
	}
	if(int_flag & LEUART_IF_RXDATAV){
		leuart_rxdatav();
	}
	if(int_flag & LEUART_IF_SIGF){
		leuart_sigf();
	}
}

/***************************************************************************//**
 * @brief Begins the LEUART TX state machine.
 *
 * @details
 * 	Transmits the string input over LEUART and once the transmit is completed,
 * 	raises the tx_done_event. This function also starts the LEUART state machine.
 *
 * @param[in] *leuart
 * 	The pointer to the LEUART peripheral
 *
 * @param[in] *string
 * 	The string to transmitted to the LEUART
 *
 ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string){
//	sleep_block_mode(EM3);
	while(leuart_busy());

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	tx_leuart_sm.str_len = strlen(string);
	tx_leuart_sm.sent_bytes = 0;
	strcpy(tx_leuart_sm.str, string);
	tx_leuart_sm.busy = true;
	sleep_block_mode(LEUART_TX_EM);

	tx_leuart_sm.state = TXdata;
	tx_leuart_sm.LEUARTn = leuart;
	tx_leuart_sm.callback = leuart0_tx_done_cb;

	leuart->IEN |= LEUART_IEN_TXBL;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief Return the busy state of the LEUART
 *
 * @details
 * 	The LEUART state machine is busy when the LEUART is transmitting or waiting
 * 	for an interrupt.
 *
 * @return
 * 	Returns the busy state of the LEUART peripheral.
 *
 ******************************************************************************/

bool leuart_busy(void){
	return tx_leuart_sm.busy;
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *0
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}


/***************************************************************************//**
 * @brief
 *   Runs a test of the RX capabilities of the LEUART module
 *
 * @details
 * 	 This test uses the loopback feature to test for blocking data before a start frame,
 * 	 receiving a start frames, receiving data after a start frame, receiving a SIG frame,
 * 	 and the reception of an entire test string. The test string includes data before the
 * 	 start frame, a start frame, data that is supposed to be passed through, a signal frame,
 * 	 and data after the signal frame. A fully function LEUART RX driver will only pass the
 * 	 start frame, the data meant to be passed, and the signal frame. One test escape is the
 * 	 misconfiguration in the application level, which cannot be tested on a driver level.
 * 	 This test does not explicitly test for a NULL character terminating the recieved string,
 * 	 but the string comparison in the test string will catch strings without NULL characters.
 *
 * @note
 *   Part of this test is clearing the LEUART IEN register for the duration of the
 *   test and then restoring it upon completion. This ensures that the test is not
 *   interrupted by an LEUART IRQ.
 *
 *
 ******************************************************************************/
void leuart_rx_test(void){
// test the interrupt driven state machine
// Need to use loopback mode of LEUART to test

// Before testing, save IEN register, disable interrupts,
// enter loopback mode (through CTRL register), while(syncbusy)
	LEUART_TypeDef * leuart = rx_leuart_sm.LEUARTn;
	uint32_t save_IEN;
	save_IEN = leuart->IEN;
	leuart->IEN = 0;
	leuart->CTRL |= LEUART_CTRL_LOOPBK | LEUART_CTRL_SFUBRX;
	leuart->CMD = LEUART_CMD_RXBLOCKEN;
	while(leuart->SYNCBUSY);

// Test whether LEUART blocks a non-Start Frame
	leuart->IFC = _LEUART_IFC_MASK;
	char test_char = 'a';
	leuart_app_transmit_byte(leuart, test_char);
	timer_delay(4);
	EFM_ASSERT((leuart->IF & LEUART_IF_RXDATAV) == 0);

// Test whether LEUART receives a Start Frame
	leuart->IFC = _LEUART_IFC_MASK;
	test_char = '#';
	leuart_app_transmit_byte(leuart, test_char);
	timer_delay(4);
	EFM_ASSERT((leuart->IF & LEUART_IF_STARTF) != 0);
	EFM_ASSERT(test_char == leuart->RXDATA);
	leuart->IFC = LEUART_IFC_STARTF;

// Test whether any additional character is received
	leuart->IFC = _LEUART_IFC_MASK;
	test_char = 'b';
	leuart_app_transmit_byte(leuart, test_char);
	timer_delay(4);
	EFM_ASSERT((leuart->IF & LEUART_IF_RXDATAV) != 0);
	EFM_ASSERT(test_char == leuart->RXDATA);
// Test whether LEUART properly indicates a Signal Frame
	leuart->IFC = _LEUART_IFC_MASK;
	test_char = '!';
	leuart_app_transmit_byte(leuart, test_char);
	timer_delay(4);
	EFM_ASSERT((leuart->IF & LEUART_IF_SIGF) != 0);
	EFM_ASSERT(test_char == leuart->RXDATA);
	leuart->IFC = LEUART_IFC_SIGF;
// To run the tests above, you send a character through TXDATA, wait 4ms,
// check appropriate interrupt, verify byte received correctly if appropriate


// Need to renable interrupts for this test, re-enable RXDATA block and synchronize
	leuart->IEN = save_IEN;
	leuart->CTRL |= LEUART_CTRL_LOOPBK;
	leuart->CMD = LEUART_CMD_RXBLOCKEN;
	while(leuart->SYNCBUSY);

// Send a string to test out the complete state machine
	// Transmit via leuart_start() "abc#Hello!def"
	char str[] = "abc#Hello!def";
//	char str[11] = "#Hello!def";
	char rx_str[] = "#Hello!";
	leuart_start(leuart, str);
	while(leuart->SYNCBUSY & LEUART_SYNCBUSY_TXDATA);
	// wait 30ms for transmission and reception
	timer_delay(30);
	// should receive "#Hello!" (rx_str)
	EFM_ASSERT(strcmp(rx_leuart_sm.str, rx_str) == 0);
// Disable loop back mode
	leuart->CTRL &= ~LEUART_CTRL_LOOPBK;
	while(leuart->SYNCBUSY);
	remove_scheduled_event(leuart_rx_cb);
}
/***************************************************************************//**
 * @brief
 *   Returns the string currently stored in the RX state machine.
 *
 * @details
 * 	 This function is used to pass the received string from the driver to the application
 * 	 level, which can then use the string to change the operation of the system.
 *
 * 	@note
 * 	 The returned string includes the start and sig frames ('#' and '!', respectively),
 * 	 so that must be accounted for in the application level.
 *
 * @return
 * 	 The string currently stored in the RX state machine.
 *
 ******************************************************************************/
char* rx_str(void){
	return rx_leuart_sm.str;
}
