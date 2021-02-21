/*

Name:       DSX_Lib.cpp
Created:    2/15/2021 10:17 PM
Author:     Jay 

 */

#include "Arduino.h"
#include "DSX_Lib.h"


/**** VARIABLE DECLERATIONS ****/
char Buffer[MAX_BUFFER_SIZE];           // Allocate space for the string
char inChar;                			// Stores the current character being read 
unsigned char buffer_state = EMPTY; 	// Current state of buffer (full, empty)
unsigned char index = 0;    			// Index into the char array
char keyword[] = "dsx";     			// keyword, used for parsing
DSXpacket_t DSXpacket; 					// To save packet data ID, loc, val
unsigned char ardDioPins[] = {2,3,4,5,7,8,11,12,13};	// available arduino digital pins
unsigned char ardPwmPins[] = {6,9,10};
char * commands[3] = {"Dio", "configDio", "pwm"};


/**
 * @brief  	Checks if the pin is a valid Digital pin
 *
 * @note	true = valid pin, false = invalid pin
 *
 * @param  	int pin
 *
 * @retval 	bool
 */
bool is_valid_dio_pin(int pin) {
	bool valid_dio = false;
	for(unsigned int i=0; i<sizeof(ardDioPins) ; ++i) {
		if(pin == ardDioPins[i]) valid_dio = true;
	}
	return valid_dio;
}

/**
 * @brief  	Checks if the pin is a valid PWM pin
 *
 * @note   	true = valid pin, false = invalid pin
 *
 * @param  	int pin
 *
 * @retval 	bool
 */
bool is_valid_pwm_pin(int pin) {
	bool valid_pwm = false;
	for(unsigned int i=0 ; i<sizeof(ardPwmPins) ; ++i) {
		if(pin == ardPwmPins[i]) valid_pwm = true;
	}
	return valid_pwm;
}

/**
 * @brief  	Initializes pins on the Arduino Uno
 *
 * @note   	Every pin in ardDioPins is set as INPUT
 *		   	Pin 13 is set by default as an OUTPUT
 *
 * @param  	None
 *
 * @retval 	None
 */
void gioInit() {
	for (unsigned int i=0 ; i<sizeof(ardDioPins) ; ++i) {
		pinMode(ardDioPins[i],INPUT);
	}
	pinMode(13,OUTPUT);
}

/**
 * @brief  	Executes the valid commands from packet
 *
 * @note   	The packet must have values for ID, loc, and val
 *
 * @param  	packet of type DSXpacket_t
 *
 * @retval 	None
 */
void exec_command(DSXpacket_t packet) {

	// Check if the ID is valid
	bool temp_valid_ID = false;
	for(unsigned int i=0 ; i<3 ; ++i) {
		if(strcmp(packet.ID, commands[i]) == 0) temp_valid_ID = true; 
	}
	if(temp_valid_ID == false && strlen(packet.ID) > 0) {	// no valid ID found
		Serial.println("Unknown command");
		return;	
	}

	/**** MAKE DECISIONS BASED ON PROCESSED PACKET HERE ****/
	if(strcmp(packet.ID, "Dio") == 0) {
		exec_Dio(packet.loc,packet.val);
	}
	else if(strcmp(packet.ID, "configDio") == 0) {
		exec_configDio(packet.loc,packet.val);
	}
	else if(strcmp(packet.ID, "pwm") == 0) {
		exec_pwm(packet.loc,packet.val);
	}

}

/**
 * @brief  	Executes digital input/output command
 *
 * @note   	pin can be any pin defined in ardDioPins[]
 *		   	value is 0 = LOW, 1 = HIGH, anything else is 0	
 *
 * @param  	int pin
 *
 * @param	int value
 *
 * @retval 	None
 */
void exec_Dio(int pin, int value) {
	if(value < 0 || value > 1) value = 0;
	if(is_valid_dio_pin(pin)) digitalWrite(pin, value);
	else Serial.println("Invalid Pin");
}

/**
 * @brief  	Executes command to configure pin
 *
 * @note   	pin can be any pin defined in ardDioPins[]
 *			value can be 0 = INPUT, 1 = INPUT_PULLUP, 2 = OUTPUT
 *			default is INPUT
 *
 * @param  	int pin
 *
 * @param  	int value
 *
 * @retval 	bool
 */
void exec_configDio(int pin, int config) {
	if(is_valid_dio_pin(pin)) {
		switch(config){
		case 0:
			pinMode(pin, INPUT);
			break;
		case 1:
			pinMode(pin, INPUT_PULLUP);
			break;
		case 2:
			pinMode(pin, OUTPUT);
			break;
		default:
			pinMode(pin, INPUT);
			break;
		}
	}
	else {
		Serial.println("Invalid Pin");
	}
}

/**
 * @brief  	Executes command set PWM duty cycle
 *
 * @note   	pin can be any pin defined in ardPwmPins[]
 *			value accepts integer only from 0 - 100
 *
 * @param  	int pin
 *
 * @param  	int value
 *
 * @retval 	bool
 */
void exec_pwm(int pin, int value) {
	if(value<0) value=0;
	else if(value>100) value=100;
	if(is_valid_pwm_pin(pin)) analogWrite(pin, map(value, 0,100, 0,255));
	else Serial.println("Invalid Pin");
}

/**
 * @brief  	Returns buffer state
 *
 * @note   	Buffer state can either be FULL or EMPTY
 *			FULL buffer state does not mean that the buffer 
 *			has exceded MAX_BUFFER_SIZE
 *
 * @param  	None
 *
 * @retval 	unsigned char
 */
unsigned char get_buffer_state() {
	return buffer_state;
}

/**
 * @brief  	Reads incoming data
 *
 * @note   	Incoming data is saved one byte at a time
 *			Buffer is set FULL if new line detected
 *			or if the buffer is about to overflow
 *
 * @param  	None
 *
 * @retval 	None
 */
void receive_packet() {
	// Store characters one byte at a time into the Buffer
	inChar = Serial.read();
	char tempBuffer[MAX_BUFFER_SIZE];  // temp buffer
	tempBuffer[index]=inChar;
	index++;

	// new line character detected, end of packet
	if(inChar == '\n' || index == MAX_BUFFER_SIZE-1 ){
		buffer_state = FULL;
		strncpy(Buffer, tempBuffer, sizeof(Buffer));
		index=0;
		strncpy(tempBuffer,"",sizeof(Buffer));
	}
}

/**
 * @brief  	Parse raw packet and saves ID, loc, and val information
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	None
 */
void process_packet() {

	char *pointerToFoundData = strstr(Buffer, keyword); //go find keyword
	if (pointerToFoundData != NULL) { // found it
		int positionInString = pointerToFoundData - Buffer;

		//strip good data
		char goodData[50];
		strncpy(goodData, &Buffer[positionInString + strlen(keyword)], sizeof(goodData));
		strncpy(Buffer, "", sizeof(Buffer));
		buffer_state = EMPTY;

		//PARSE *** PARSE *** PARSE *** PARSE *** PARSE *** PARSE *** PARSE ***
		const char delimeter[] = ",";
		char parsedStrings[3][20];    // init 3 character strings with each having 20 bytes
		char *token = strtok(goodData, delimeter);
		strncpy(parsedStrings[0], token, sizeof(parsedStrings[0])); // first one
		for(unsigned int i=1; i<3; ++i) {
			token = strtok(NULL, delimeter);
			strncpy(parsedStrings[i],token, sizeof(parsedStrings[i]));
		}

		// CONVERT TO THE CORRECT NUMBER TYPE AND SAVE TO DSXpacket
		strncpy(DSXpacket.ID, parsedStrings[0], sizeof(DSXpacket.ID));
		DSXpacket.loc = atoi(parsedStrings[1]);
		DSXpacket.val = atoi(parsedStrings[2]);

	}
	else {  // The keyword was not detected
		Serial.println("No keyword detected\n");
		strncpy(Buffer, "", sizeof(Buffer));
		buffer_state = EMPTY;

	}
}

/**
 * @brief  	Returns ready to use packet for exec_command()
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	DSXpacket_t
 */
DSXpacket_t get_packet() {
	return DSXpacket;
}
