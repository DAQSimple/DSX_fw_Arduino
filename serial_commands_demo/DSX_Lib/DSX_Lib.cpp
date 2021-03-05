/*

Name:       DSX_Lib.cpp
Created:    2/15/2021 10:17 PM
Author:     Jay 

 */

#include "Arduino.h"
#include "DSX_Lib.h"
#include <stdint.h>
#include <Servo.h>


/**** VARIABLE DECLERATIONS ****/
// Variables for calculating rpm and direction from encoder readings
long previousMillis = 0;
long currentMillis = 0;
int rpm = 0;	// will hold speed from encoder in rpm
unsigned int encoderValue = 0;	// count from encoder
unsigned int ENC_COUNT_REV = 48;	// encoder count per revolution. Default = 48;
bool CW = 1;
bool CCW = 0;
bool direction;		// to store direction 1: clockwise, 0: counter clockwise

// Variables for inputting into the buffer
char Buffer[MAX_BUFFER_SIZE];           // Allocate space for the string
char inChar;                			// Stores the current character being read 
unsigned char buffer_state = EMPTY; 	// Current state of buffer (full, empty)
unsigned char index = 0;    			// Index into the char array
char keyword[] = "dsx";     			// keyword, used for parsing
DSXpacket_t DSXpacket; 					// To save packet data ID, loc, val

// Arduino Pin Defenitions
unsigned char encoderChA = 2;
unsigned char encoderChB = 4;
unsigned char ardDioInPins[] = {7,10};		// available arduino INPUT digital pins
unsigned char ardDioOutPins[] = {8,9,12,13}; 	// available arduino OUTPUT digital pins
unsigned char ardPwmPins[] = {3,5,11};			// available arduino pwm pins
												// PWM pins 3 and 11 can configure their frequency
unsigned char ardAnalogPins[] = {0,1,2,3,4,5};	// A0,A1,A2,A3,A4,A5	
unsigned char ardServoPin = 6;	// available arduino servo pin

// Create servo object to use servo
Servo myservo;	


/**
 * @brief  	Checks if the pin is a valid Digital INPUT pin
 *
 * @note	true = valid pin, false = invalid pin
 *
 * @param  	int pin
 *
 * @retval 	bool
 */
bool is_valid_dio_in_pin(int pin) {
	bool valid_dio = false;
	for(unsigned int i=0; i<sizeof(ardDioInPins) ; ++i) {
		if(pin == ardDioInPins[i]) valid_dio = true;
	}
	return valid_dio;
}

/**
 * @brief  	Checks if the pin is a valid Digital OUTPUT pin
 *
 * @note	true = valid pin, false = invalid pin
 *
 * @param  	int pin
 *
 * @retval 	bool
 */
bool is_valid_dio_out_pin(int pin) {
	bool valid_dio = false;
	for(unsigned int i=0; i<sizeof(ardDioOutPins) ; ++i) {
		if(pin == ardDioOutPins[i]) valid_dio = true;
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
 * @brief  	Checks if the pin is a valid analog input pin
 *
 * @note   	true = valid pin, false = invalid pin
 *
 * @param  	int pin
 *
 * @retval 	bool
 */
bool is_valid_analog_pin(int pin) {
	bool valid_analogPin = false;
	for(unsigned int i=0 ; i<sizeof(ardAnalogPins) ; ++i) {
		if(pin == ardAnalogPins[i]) valid_analogPin = true;
	}
	return valid_analogPin;
}

/**
 * @brief  	Initializes pins on the Arduino Uno
 *
 * @note   	Every pin in ardDioInPins is set as INPUT with pullup
 *		   	Every pin in ardDioOutPins is set as OUTPUT
 *
 * @param  	None
 *
 * @retval 	None
 */
void initPins() {
	// Initialize arduino input pins with pullup
	for (unsigned int i=0 ; i<sizeof(ardDioInPins) ; ++i) {
		pinMode(ardDioInPins[i],INPUT_PULLUP);
	}

	// Initialize arduino output pins
	for (unsigned int i=0 ; i<sizeof(ardDioOutPins) ; ++i) {
		pinMode(ardDioOutPins[i],OUTPUT);
	}

	// initialize servo pin 6
	myservo.attach(ardServoPin);
	
}

/**
 * @brief  	Interrupt service routine of pin 2 that simply increments a variable every falling edge	
 *
 * @note   	None	
 *
 * @param  	None
 *
 * @retval 	None
 */
void updateEncoder() {
	encoderValue++;
	
	// check if going CW or CCW
	if(digitalRead(encoderChB))
		direction = CW;
	else
		direction = CCW;
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
	/**** MAKE DECISIONS BASED ON PROCESSED PACKET HERE ****/
	if(strcmp(packet.ID, "Dio") == 0) {
		exec_Dio(packet.loc,packet.val);
	}
	else if(strcmp(packet.ID, "digitalRead") == 0) {
		exec_digitalRead(packet.loc);
	}
	else if (strcmp(packet.ID, "analogRead") == 0) {
		exec_analogRead(packet.loc);
	}
	else if(strcmp(packet.ID, "pwm") == 0) {
		exec_pwm(packet.loc,packet.val);
	}
	else if (strcmp(packet.ID, "servo") == 0) {
		exec_servoWrite(packet.loc,packet.val);
	}
	else if (strcmp(packet.ID, "getDioMode") == 0) {
		getDioMode(packet.loc);
	}
	else if (strcmp(packet.ID, "getSerial") == 0) {
		getSerial();
	}
	else if (strcmp(packet.ID, "getEncoderSpeed") == 0) {
		getEncoderSpeed();
	}
	else if (strcmp(packet.ID, "getEncoderDir") == 0) {
		getEncoderDir();
	}
	else if (strcmp(packet.ID, "setPWMFreq") == 0) {
		exec_setPWMFreq(packet.loc,packet.val);
	}
	else if (strcmp(packet.ID, "encoderSpeed") == 0) {
		getEncoderSpeed();
	}
	else if (strcmp(packet.ID, "encoderDir") == 0) {
		getEncoderDir();
	}
	else {
		Serial.println("Unknown command");
	}

}

/**
 * @brief  	read encoder direction	
 *
 * @note   	None	
 *
 * @param  	None	
 *
 * @retval 	None
 */
void getEncoderDir() {
	Serial.println(direction);
}
/**
 * @brief  	Read digital pin, float = 1 because of pullup
 *
 * @note   	If the pin reads HIGH = 1
 *			If the pin reads LOW = 0
 *
 * @param  	int pin
 *
 * @retval 	None
 */
void exec_digitalRead(int pin) {
	if(is_valid_dio_in_pin(pin)) {
		Serial.println(digitalRead(pin));
	}
	else Serial.println("Invalid pin");
}

/**
 * @brief  	Read analog input pin
 *
 * @note   	prints input voltage (0-5V)
 *
 * @param  	int pin
 *
 * @retval 	None
 */
void exec_analogRead(int pin) {
	if(is_valid_analog_pin(pin)) {
		float voltage = (analogRead(pin) / 1023.0) * 5;	// 0 - 5V
		Serial.println(voltage);
	}
	else Serial.println("Invalid pin");
}

/**
 * @brief  	Executes digital write command
 *
 * @note   	pin can be any pin defined in ardDioOutPins[]
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
	if(is_valid_dio_out_pin(pin)) digitalWrite(pin, value);
	else Serial.println("Invalid Pin");
}

/**
 * @brief  	Executes command to set PWM frequency
 *
 * @note   	pin can be 3 or 11
 *
 * @param  	int pin
 *
 * @param  	int value
 *
 * @retval 	None
 */
void exec_setPWMFreq(int pin, int value) {
	if(pin==3 || pin==11) {
		// set pin 3 and pin 11 pwm to the desired frequency
		switch(value) {
			case 30:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_30HZ;
				break;
			case 122:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_122HZ;
				break;
			case 245:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_245HZ;
				break;
			case 490:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_490HZ;
				break;
			case 980:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_980HZ;
				break;
			case 4000:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_4KHZ;
				break;
			case 32000:
				TCCR2B = (TCCR2B & CLEAR_LAST3_LSB) | FREQ_32KHZ;
				break;
			default:
				Serial.println("Invalid Frequency");
				break;
		}
		
	}
	else Serial.println("Invalid Pin");
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
 * @retval 	None
 */
void exec_pwm(int pin, int value) {
	if(value<0) value=0;
	if(value>100) value=100;
	if(is_valid_pwm_pin(pin)) {
		value = map(value, 0,100, 0,255);
		analogWrite(pin,value);
	}
	else Serial.println("Invalid Pin");
}

/**
 * @brief  	Executes command to rotate servo pin 
 *
 * @note   	pin can only be pin (ardServoPin)
 *			value accepts integer only from 0 - 180
 *
 * @param  	int pin
 *
 * @param  	int value
 *
 * @retval 	None
 */
void exec_servoWrite(int pin, int value) {
	if(value<0) value=0;
	else if(value>180) value=180;
	if(pin == ardServoPin) {
		myservo.write(value);
	}
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
 * @brief  	Show serial configuration settings
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	None
 */
void getSerial() {
	Serial.println("9600,8N1");
}

/**
 * @brief  	Returns digital pin mode, either INPUT or OUTPUT
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	None
 */
void getDioMode(int pin) {
	if(is_valid_dio_in_pin(pin)) Serial.println("INPUT");
	else if(is_valid_dio_out_pin(pin)) Serial.println("OUTPUT");
	else if(is_valid_pwm_pin(pin)) Serial.println("PWM OUTPUT");
	else Serial.println("Invalid Pin");
	
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

	// linefeed character detected so we reached the end of packet
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
		// Clear the buffer once the command is executed
		clear_buffer();

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
		// Save ID command 
		strncpy(DSXpacket.ID, parsedStrings[0], sizeof(DSXpacket.ID));
		
		// Save loc value as an integer
		// if ID is analogRead, get only number (ex. A0 -> 0)
		if(strcmp(DSXpacket.ID,"analogRead")==0)
			DSXpacket.loc = atoi(&parsedStrings[1][1]);
		else
			DSXpacket.loc = atoi(parsedStrings[1]);
		
		// Save value as an integer
		if(strcmp(DSXpacket.ID,"pwm16")==0) {
			DSXpacket.fval = atof(parsedStrings[2]);
		}
		else
			DSXpacket.val = atoi(parsedStrings[2]);
	}
	else {  // The keyword was not detected
		Serial.println("No keyword detected\n");
		clear_buffer();

	}
}

/**
 * @brief  	clears buffer
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	None
 */
void clear_buffer() {
	strncpy(Buffer, "", sizeof(Buffer));
	buffer_state = EMPTY;
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

/**
 * @brief  	Initialize encoder reading
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	None
 */
void initEncoder() {
	// initialize interrupt pins 2 for reading encoder
	pinMode(encoderChA, INPUT_PULLUP); 
	pinMode(encoderChB, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(encoderChA),updateEncoder,RISING);
	
	// Setup initial values for timer
	previousMillis = millis();
}

/**
 * @brief  	Update encoder readings, sends out rpm 
 *
 * @note   	None
 *
 * @param  	None
 *
 * @retval 	None
 */
 void readEncoder() {
	// Update RPM value every second
	currentMillis = millis();
	if (currentMillis - previousMillis > 1000) {
		previousMillis = currentMillis;

		// Calculate RPM
		rpm = (float)(encoderValue * 60 / ENC_COUNT_REV);
    
		encoderValue = 0;
	} 
 }
 
void getEncoderSpeed() {
	Serial.println(rpm);
}