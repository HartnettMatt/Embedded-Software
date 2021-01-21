/*
 * sleep_routines.h
 *
 *  Created on: Sep 22, 2020
 *      Author: Matt Hartnett
 */

#ifndef SRC_HEADER_FILES_SLEEP_ROUTINES_H_
#define SRC_HEADER_FILES_SLEEP_ROUTINES_H_

#include "em_emu.h"
#include "em_int.h"
#include "em_core.h"
#include "em_assert.h"

#define		EM0				0
#define		EM1				1
#define		EM2				2
#define		EM3				3
#define		EM4				4
#define	MAX_ENERGY_MODES	5

void sleep_open(void);
void sleep_block_mode(uint32_t EM);
void sleep_unblock_mode(uint32_t EM);
void enter_sleep(void);
uint32_t current_block_energy_mode(void);

#endif /* SRC_HEADER_FILES_SLEEP_ROUTINES_H_ */

