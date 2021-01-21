/*
 * ble.h
 *
 *  Created on: Novermeber 1st, 2020
 *      Author: Matt Hartnett and Keith Graham
 */
#ifndef	BLE_GUARD_H
#define	BLE_GUARD_H
//***********************************************************************************
// Include files
//***********************************************************************************


//** Standard Libraries
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Driver functions
#include "leuart.h"
#include "gpio.h"

//***********************************************************************************
// define
//***********************************************************************************

#define HM10_LEUART0		LEUART0
#define HM10_BAUDRATE		9600
#define	HM10_DATABITS		leuartDatabits8
#define HM10_ENABLE			leuartEnable
#define HM10_PARITY			leuartNoParity
#define HM10_REFFREQ		0					// use reference clock
#define HM10_STOPBITS		leuartStopbits1

#define LEUART0_TX_ROUTE	LEUART_ROUTELOC0_TXLOC_LOC18   	// Route to PD11_18
#define LEUART0_RX_ROUTE	LEUART_ROUTELOC0_RXLOC_LOC18   	// Route to PD10_18

#define CIRC_TEST			true
#define	CIRC_OPER			false

#define	CSIZE				64
typedef struct {
	char					cbuf[CSIZE];
	uint8_t					size_mask;
	uint32_t				size;
	uint32_t				read_ptr;
	uint32_t				write_ptr;
} BLE_CIRCULAR_BUF;
#define	PACKET_HEADER		1
#define CIRC_TEST_SIZE		3
typedef	struct {
	char test_str[CIRC_TEST_SIZE][CSIZE];
	char result_str[CSIZE];
} CIRC_TEST_STRUCT;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void ble_open(uint32_t tx_event, uint32_t rx_event);
void ble_write(char *string);

bool ble_test(char *mod_name);
void circular_buff_test(void);
bool ble_circ_pop(bool test);
#endif
