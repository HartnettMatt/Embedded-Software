//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	APP_HG
#define	APP_HG

/* System include statements */
#include <stdio.h>
#include <string.h>
/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "Si7021.h"
#include "ble.h"
#include "HW_Delay.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define		PWM_PER				2.7		// PWM period in seconds
#define		PWM_ACT_PER			0.15	// PWM active period in seconds
#define		PWM_ROUTE_0			LETIMER_ROUTELOC0_OUT0LOC_LOC28
#define 	PWM_ROUTE_1			LETIMER_ROUTELOC0_OUT1LOC_LOC28
// Application scheduled events
#define LETIMER0_COMP0_CB		0x00000001	//0b00001
#define LETIMER0_COMP1_CB		0x00000002	//0b00010
#define LETIMER0_UF_CB			0x00000004	//0b00100
#define	SI7021_READ_CB			0x00000008  //0b01000
#define BOOT_UP_CB				0x00000010  //0b10000
#define BLE_TX_CB				0x00000020
#define BLE_RX_CB				0x00000040

#define SYSTEM_BLOCK_EM			EM3

//#define BLE_TEST_ENABLED

//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void scheduled_letimer0_uf_cb (void);
void scheduled_letimer0_comp0_cb (void);
void scheduled_letimer0_comp1_cb (void);
void si7021_temp_done_evt(void);
void scheduled_boot_up_cb (void);
void scheduled_ble_rx_cb (void);
void scheduled_ble_tx_cb (void);
#endif
