/**
 * @file ble.c
 * @author Matt Hartnett and Keith Graham
 * @date December 4th, 2020
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "ble.h"
#include <string.h>

//***********************************************************************************
// private variables
//***********************************************************************************
static CIRC_TEST_STRUCT test_struct;
static BLE_CIRCULAR_BUF ble_cbuf;

//***********************************************************************************
// private functions
//***********************************************************************************

static void ble_circ_init(void);
static void ble_circ_push(char *string);
static uint8_t ble_circ_space(void);
static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);
static void update_circ_readtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);
/***************************************************************************//**
 * @brief BLE module
 * @details
 *  This module contains all the functions to interface the application layer
 *  with the HM-18 Bluetooth module.  The application does not have the
 *  responsibility of knowing the physical resources required, how to
 *  configure, or interface to the Bluetooth resource including the LEUART
 *  driver that communicates with the HM-18 BLE module.
 *
 ******************************************************************************/

//***********************************************************************************
// Global functions
//***********************************************************************************


/***************************************************************************//**
 * @brief Opens the BLE module and configures for LEUART communication
 *
 * @details
 * 	This function uses the parameters specified in ble.h to ensure that the
 * 	BLE module can communicate over UART with the CPU. Once configured, this
 * 	function calls the leuart_open() function to configure the LEUART module.
 *
 * @param[in] tx_event
 *   Transmit event for the scheduler.
 *
 * @param[in] rx_event
 *   Recieve event for the scheduler.
 *
 *
 ******************************************************************************/

void ble_open(uint32_t tx_event, uint32_t rx_event){
	LEUART_OPEN_STRUCT	leuart_ble_struct;
	leuart_ble_struct.baudrate = HM10_BAUDRATE;
	leuart_ble_struct.databits = HM10_DATABITS;
	leuart_ble_struct.enable = HM10_ENABLE;
	leuart_ble_struct.parity = HM10_PARITY;
	leuart_ble_struct.refFreq = HM10_REFFREQ;
	leuart_ble_struct.rx_done_evt = rx_event;
	leuart_ble_struct.rx_en = true;
	leuart_ble_struct.rx_loc = LEUART0_RX_ROUTE;
	leuart_ble_struct.rx_pin_en = true;
	leuart_ble_struct.rxblocken = false;
	leuart_ble_struct.stopbits = HM10_STOPBITS;
	leuart_ble_struct.tx_done_evt = tx_event;
	leuart_ble_struct.tx_en = true;
	leuart_ble_struct.tx_loc = LEUART0_TX_ROUTE;
	leuart_ble_struct.tx_pin_en = true;
	leuart_open(HM10_LEUART0, &leuart_ble_struct);
	ble_circ_init();
}


/***************************************************************************//**
 * @brief Transmits a string over the BLE module.
 *
 * @details
 *	Creates a local copy of the string before transmitting over UART to
 *	the BLE module.
 *
 * @param[in] *string
 *   Pointer to the string to be transmitted.
 *
 * @param[in] tx_done_event
 *   The event with one hot encoding to signify that a transmit has been
 *   completed.
 *
 ******************************************************************************/

void ble_write(char* string){
	ble_circ_push(string);
 	ble_circ_pop(false);
}

/***************************************************************************//**
 * @brief
 *   BLE Test performs two functions.  First, it is a Test Driven Development
 *   routine to verify that the LEUART is correctly configured to communicate
 *   with the BLE HM-18 module.  Second, the input argument passed to this
 *   function will be written into the BLE module and become the new name
 *   advertised by the module while it is looking to pair.
 *
 * @details
 * 	 This global function will use polling functions provided by the LEUART
 * 	 driver for both transmit and receive to validate communications with
 * 	 the HM-18 BLE module.  For the assignment, the communication with the
 * 	 BLE module must use low energy design principles of being an interrupt
 * 	 driven state machine.
 *
 * @note
 *   For this test to run to completion, the phone most not be paired with
 *   the BLE module.  In addition for the name to be stored into the module
 *   a breakpoint must be placed at the end of the test routine and stopped
 *   at this breakpoint while in the debugger for a minimum of 5 seconds.
 *
 * @param[in] *mod_name
 *   The name that will be written to the HM-18 BLE module to identify it
 *   while it is advertising over Bluetooth Low Energy.
 *
 * @return
 *   Returns bool true if successfully passed through the tests in this
 *   function.
 ******************************************************************************/

bool ble_test(char *mod_name){
	uint32_t	str_len;

	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();

	// This test will limit the test to the proper setup of the LEUART
	// peripheral, routing of the signals to the proper pins, pin
	// configuration, and transmit/reception verification.  The test
	// will communicate with the BLE module using polling routines
	// instead of interrupts.
	// How is polling different than using interrupts?
	// ANSWER: Instead of waiting for an interrupt, polling constantly checks
	// to see if the device is ready
	// How does interrupts benefit the system for low energy operation?
	// ANSWER: Interrupts allow the CPU to go to sleep while the device is working
	// on getting open and ready, which saves energy.
	// How does interrupts benefit the system that has multiple tasks?
	// ANSWER: Interrupts allow the CPU to focus on another task while the device
	// is opening, which allows for better performance when doing multiple tasks

	// First, you will need to review the DSD HM10 datasheet to determine
	// what the default strings to write data to the BLE module and the
	// expected return statement from the BLE module to test / verify the
	// correct response

	// The test_str is used to tell the BLE module to end a Bluetooth connection
	// such as with your phone.  The ok_str is the result sent from the BLE module
	// to the micro-controller if there was not active BLE connection at the time
	// the break command was sent to the BLE module.
	// Replace the test_str "" with the command to break or end a BLE connection
	// Replace the ok_str "" with the result that will be returned from the BLE
	//   module if there was no BLE connection
	char		test_str[80] = "AT";
	char		ok_str[80] = "OK";


	// output_str will be the string that will program a name to the BLE module.
	// From the DSD HM10 datasheet, what is the command to program a name into
	// the BLE module?
	// The  output_str will be a string concatenation of the DSD HM10 command
	// and the input argument sent to the ble_test() function
	// Replace the output_str "" with the command to change the program name
	// Replace the result_str "" with the first part of the expected result
	//  the backend of the expected response will be concatenated with the
	//  input argument
	char		output_str[80] = "AT+NAME";
	char		result_str[80] = "OK+Set:";


	// To program the name into your module, you must reset the module after you
	// have sent the command to update the modules name.  What is the DSD HM10
	// name to reset the module?
	// Replace the reset_str "" with the command to reset the module
	// Replace the reset_result_str "" with the expected BLE module response to
	//  to the reset command
	char		reset_str[80] = "AT+RESET";
	char		reset_result_str[80] = "OK+RESET";
	char		return_str[80];

	bool		success;
	bool		rx_disabled, rx_en, tx_en;
	uint32_t	status;

	// These are the routines that will build up the entire command and response
	// of programming the name into the BLE module.  Concatenating the command or
	// response with the input argument name
	strcat(output_str, mod_name);
	strcat(result_str, mod_name);

	// The test routine must not alter the function of the configuration of the
	// LEUART driver, but requires certain functionality to insure the logical test
	// of writing and reading to the DSD HM10 module.  The following c-lines of code
	// save the current state of the LEUART driver that will be used later to
	// re-instate the LEUART configuration

	status = leuart_status(HM10_LEUART0);
	if (status & LEUART_STATUS_RXBLOCK) {
		rx_disabled = true;
		// Enabling, unblocking, the receiving of data from the LEUART RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKDIS);
	}
	else rx_disabled = false;
	if (status & LEUART_STATUS_RXENS) {
		rx_en = true;
	} else {
		rx_en = false;
		// Enabling the receiving of data from the RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_RXENS));
	}

	if (status & LEUART_STATUS_TXENS){
		tx_en = true;
	} else {
		// Enabling the transmission of data to the TX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_TXENS));
		tx_en = false;
	}
//	leuart_cmd_write(HM10_LEUART0, (LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX));

	// This sequence of instructions is sending the break ble connection
	// to the DSD HM10 module.
	// Why is this command required if you want to change the name of the
	// DSD HM10 module?
	// ANSWER: The name can only be changed if the device does not have a
	// bluetooth connection, so the first step is to disconnect
	str_len = strlen(test_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, test_str[i]);
	}

	// What will the ble module response back to this command if there is
	// a current ble connection?
	// ANSWER: If there is a connection, it will return OK+LOST
	str_len = strlen(ok_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (ok_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// This sequence of code will be writing or programming the name of
	// the module to the DSD HM10
	str_len = strlen(output_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, output_str[i]);
	}

	// Here will be the check on the response back from the DSD HM10 on the
	// programming of its name
	str_len = strlen(result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// It is now time to send the command to RESET the DSD HM10 module
	str_len = strlen(reset_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, reset_str[i]);
	}

	// After sending the command to RESET, the DSD HM10 will send a response
	// back to the micro-controller
	str_len = strlen(reset_result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (reset_result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// After the test and programming have been completed, the original
	// state of the LEUART must be restored
	if (!rx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXDIS);
	if (rx_disabled) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKEN);
	if (!tx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXDIS);
	leuart_if_reset(HM10_LEUART0);

	success = true;


	CORE_EXIT_CRITICAL();
	return success;
}

/***************************************************************************//**
 * @brief
 *   Circular Buff Test is a Test Driven Development function to validate
 *   that the circular buffer implementation
 *
 * @details
 * 	 This Test Driven Development test has tests integrated into the function
 * 	 to validate that the routines can successfully identify whether there
 * 	 is space available in the circular buffer, the write and index pointers
 * 	 wrap around, and that one or more packets can be pushed and popped from
 * 	 the circular buffer.
 *
 * @note
 *   If anyone of these test will fail, an EFM_ASSERT will occur.  If the
 *   DEBUG_EFM=1 symbol is defined for this project, exiting this function
 *   confirms that the push, pop, and the associated utility functions are
 *   working.
 *
 * @par
 *   There is a test escape that is not possible to test through this
 *   function that will need to be verified by writing several ble_write()s
 *   back to back and verified by checking that these ble_write()s were
 *   successfully transmitted to the phone app.
 *
 ******************************************************************************/

 void circular_buff_test(void){
	 bool buff_empty;
	 int test1_len = 50;
	 int test2_len = 25;
	 int test3_len = 5;

	 // Why this 0 initialize of read and write pointer?
	 // Student Response:
	 //	When the buffer is empty, the read and write pointers point to the same location,
	 // and its easiest to start at 0.
	 ble_cbuf.read_ptr = 0;
	 ble_cbuf.write_ptr = 0;

	 // Why do none of these test strings contain a 0?
	 // Student Response:
	 //	If the test strings contain a zero, it could be interpreted as a NULL character,
	 // which would cause issues with moving the entire string into the buffer.
	 for (int i = 0;i < test1_len; i++){
		 test_struct.test_str[0][i] = i+1;
	 }
	 test_struct.test_str[0][test1_len] = 0;

	 for (int i = 0;i < test2_len; i++){
		 test_struct.test_str[1][i] = i + 20;
	 }
	 test_struct.test_str[1][test2_len] = 0;

	 for (int i = 0;i < test3_len; i++){
		 test_struct.test_str[2][i] = i +  35;
	 }
	 test_struct.test_str[2][test3_len] = 0;

	 // What is this test validating?
	 // Student response:
	 // This is validating that the circular buffer initializes the space correctly
	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // Why is there only one push to the circular buffer at this stage of the test
	 // Student Response:
	 // This test is to check to see if a single push works
	 ble_circ_push(&test_struct.test_str[0][0]);

	 // What is this test validating?
	 // Student response:
	 // That a single push reduces the space of the buffer by the correct amount
	 EFM_ASSERT(ble_circ_space() == (CSIZE - test1_len - 1));

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 // buff_empty should be false since we pushed something on the buffer earlier
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test1_len; i++){
		 EFM_ASSERT(test_struct.test_str[0][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response:
	 // This is validating that the string has the same length before and
	 // after being pushed and popped from the buffer
	 EFM_ASSERT(strlen(test_struct.result_str) == test1_len);

	 // What is this test validating?
	 // Student response:
	 // That the buffer is empty after a string has been popped
	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // What does this next push on the circular buffer test?
	 // Student Response:
	 // This tests pushing a second string onto the buffer after the
	 // first string has been popped
	 ble_circ_push(&test_struct.test_str[1][0]);


	 EFM_ASSERT(ble_circ_space() == (CSIZE - test2_len - 1));

	 // What does this next push on the circular buffer test?
	 // Student Response:
	 // This push tests having multiple strings on the buffer
	 // at the same time
	 ble_circ_push(&test_struct.test_str[2][0]);


	 EFM_ASSERT(ble_circ_space() == (CSIZE - test2_len - 1 - test3_len - 1));

	 // What does this next push on the circular buffer test?
	 // Student Response: The next push tests that the buffer doesn't
	 // accept a string larger than the space left
	 EFM_ASSERT(abs(ble_cbuf.write_ptr - ble_cbuf.read_ptr) < CSIZE);

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 // We have not removed all of the test strings, so the buffer is not
	 // empty yet
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test2_len; i++){
		 EFM_ASSERT(test_struct.test_str[1][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response: That the popped string has the same length as
	 // the original string that was pushed.
	 EFM_ASSERT(strlen(test_struct.result_str) == test2_len);

	 EFM_ASSERT(ble_circ_space() == (CSIZE - test3_len - 1));

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 // The test[1] string is still on the buffer, as the last pop
	 // only took test[2] off
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test3_len; i++){
		 EFM_ASSERT(test_struct.test_str[2][i] == test_struct.result_str[i]);
	 }

	 // What is this test validating?
	 // Student response: That the popped string is the
	 // same length as the pushed string
	 EFM_ASSERT(strlen(test_struct.result_str) == test3_len);

	 EFM_ASSERT(ble_circ_space() == CSIZE);

	 // Using these three writes and pops to the circular buffer, what other test
	 // could we develop to better test out the circular buffer?
	 // Student Response:
	 // We could test with a greater variety of test strings, such as ones
	 // that are empty or contain the zero character


	 // Why is the expected buff_empty test = true?
	 // Student Response:
	 //	Every string that has been pushed onto the buffer has been
	 // popped off, so the buffer should be empty.
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == true);
	 ble_write("\nPassed Circular Buffer Test\n");

 }

 /***************************************************************************//**
  * @brief Initializes the BLE circular buffer
  *
  * @details
  *	Begins the circular buffer with the correct size and mask as defined in ble.h,
  *	and initializes the write and read pointers to zero.
  *
  ******************************************************************************/

 static void ble_circ_init(void){
	 ble_cbuf.write_ptr = 0;
	 ble_cbuf.read_ptr = 0;
	 ble_cbuf.size = CSIZE;
	 ble_cbuf.size_mask = CSIZE-1;
 }

 /***************************************************************************//**
  * @brief Pushes a string onto the circular buffer
  *
  * @details
  *	Adds the input parameter to circular buffer as long as there is enough space for the packet.
  *	The packet pushed onto the buffer has a beginning header of the packet length,
  *	which is used in the pop function to determine the length of string since the
  *	NULL character is not pushed to the buffer. This also updates the write pointer.
  *
  *	@param[in] *string
  * The string to be pushed onto the buffer if there is enough space.
  *
  ******************************************************************************/

 static void ble_circ_push(char *string){
	 if(strlen(string) + PACKET_HEADER > ble_circ_space()){
		 // Buffer doesn't have enough space for the string
		 EFM_ASSERT(false);
	 }

	 ble_cbuf.cbuf[ble_cbuf.write_ptr] = strlen(string) + PACKET_HEADER;
	 update_circ_wrtindex(&ble_cbuf, PACKET_HEADER);
	 for(uint32_t i = 0; i < strlen(string); i++){
		 ble_cbuf.cbuf[ble_cbuf.write_ptr] = string[i];
		 update_circ_wrtindex(&ble_cbuf, 1);
	 }
 }

 /***************************************************************************//**
  * @brief Pops a string off of the circular buffer
  *
  * @details
  *	If the LEUART is busy or if the circular buffer is empty, returns true.
  *	This function then pops the string off of the buffer using the header of
  *	the packet, and then appends a NULL character to the end of the string. Based
  *	on the test parameter, either sends it to the LEUART or the test case.
  *
  *	@param[in] test
  * This parameter determines if the popped string is outputted to LEUART if false,
  * or to the test value if true.
  *
  ******************************************************************************/

 bool ble_circ_pop(bool test){
 	if(leuart_busy() || ble_circ_space() == CSIZE){
 		return true;
 	}
 	uint32_t header = ble_cbuf.cbuf[ble_cbuf.read_ptr];
 	char print_str[header - PACKET_HEADER + 1]; //the extra 1 is for the null character at the end of the string
 	update_circ_readtindex(&ble_cbuf, PACKET_HEADER);

 	for(uint32_t i = 0; i < header - PACKET_HEADER; i++){
 		print_str[i] = ble_cbuf.cbuf[ble_cbuf.read_ptr];
 		update_circ_readtindex(&ble_cbuf, 1);
 	}
 	print_str[header - PACKET_HEADER] = '\0';
 	if(test){
 		for(int i = 0; i < header; i++){
 			test_struct.result_str[i] = print_str[i];
 		}
 	} else {
 		leuart_start(HM10_LEUART0, print_str);
 	}
 	return false;
 }

 /***************************************************************************//**
  * @brief Returns the amount of space on the circular buffer
  *
  * @details
  *	Uses the bit masking property to return the empty space on the buffer by
  *	calculating how much space is removed and then subtracting that from the
  *	total size of the buffer.
  *
  ******************************************************************************/

 static uint8_t ble_circ_space(void){
	 return ble_cbuf.size - ((ble_cbuf.write_ptr - ble_cbuf.read_ptr) & ble_cbuf.size_mask);
 }


 /***************************************************************************//**
  * @brief Updates the read pointer
  *
  * @details
  *	Increments the read pointer of the circular buffer by the value of the update_by
  *	parameter, using bit masking to ensure that the read pointer wraps around correctly.
  *
  *	@param[in] *index_struct
  *	A pointer to the circular buffer that is changed. NOTE: This parameter is not used
  *	and its purpose is unclear, but included to match lab requirements.
  *
  *	@param[in] update_by
  *	The numerical value to increment the read pointer by.
  *
  ******************************************************************************/

 static void update_circ_readtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	 //make atomic
	 ble_cbuf.read_ptr = (ble_cbuf.read_ptr + update_by) & ble_cbuf.size_mask;
 }

 /***************************************************************************//**
  * @brief Updates the write pointer
  *
  * @details
  *	Increments the write pointer of the circular buffer by the value of the update_by
  *	parameter, using bit masking to ensure that the write pointer wraps around correctly.
  *
  *	@param[in] *index_struct
  *	A pointer to the circular buffer that is changed. NOTE: This parameter is not used
  *	and its purpose is unclear, but included to match lab requirements.
  *
  *	@param[in] update_by
  *	The numerical value to increment the write pointer by.
  *
  ******************************************************************************/

 static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	 // make atomic
	 ble_cbuf.write_ptr = (ble_cbuf.write_ptr + update_by) & ble_cbuf.size_mask;
 }
