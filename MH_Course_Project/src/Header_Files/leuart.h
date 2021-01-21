//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	LEUART_GUARD_H
#define	LEUART_GUARD_H

#include "em_leuart.h"
#include "sleep_routines.h"
#include "HW_delay.h"


//***********************************************************************************
// defined files
//***********************************************************************************

#define LEUART_TX_EM		EM3
#define LEUART_RX_EM		EM3

/***************************************************************************//**
 * @addtogroup leuart
 * @{
 ******************************************************************************/

typedef struct {
	uint32_t					baudrate;
	LEUART_Databits_TypeDef		databits;
	LEUART_Enable_TypeDef		enable;
	LEUART_Parity_TypeDef 		parity;
	uint32_t					refFreq;
	LEUART_Stopbits_TypeDef		stopbits;
	bool						rxblocken;
	bool						sfubrx;
	bool						startframe_en;
	char						startframe;
	bool						sigframe_en;
	char						sigframe;
	uint32_t					rx_loc;
	uint32_t					rx_pin_en;
	uint32_t					tx_loc;
	uint32_t					tx_pin_en;
	bool						rx_en;
	bool						tx_en;
	uint32_t					rx_done_evt;
	uint32_t					tx_done_evt;
} LEUART_OPEN_STRUCT;

typedef struct {
	uint32_t					state;
	bool						busy;
	char 						str[64];
	uint32_t					str_len;
	uint32_t					sent_bytes;
	LEUART_TypeDef *			LEUARTn;
	uint32_t					callback;
} TX_LEUART_STATE_MACHINE;

typedef struct {
	uint32_t					state;
	char						str[64];
	uint32_t					str_len;
	LEUART_TypeDef *			LEUARTn;
	uint32_t					callback;
} RX_LEUART_STATE_MACHINE;


enum tx_leuart_defined_states {
	start,
	TXdata,
	stop
};

enum rx_leuart_defined_states {
	RXstart,
	RXdata,
	decode
};
/** @} (end addtogroup leuart) */

//***********************************************************************************
// function prototypes
//***********************************************************************************
void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings);
void LEUART0_IRQHandler(void);
void leuart_start(LEUART_TypeDef *leuart, char *string);
bool leuart_busy(void);

uint32_t leuart_status(LEUART_TypeDef *leuart);
void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update);
void leuart_if_reset(LEUART_TypeDef *leuart);
void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out);
uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart);
void leuart_rx_test(void);
char* rx_str(void);
#endif
