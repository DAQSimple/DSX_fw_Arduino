/*

Name:       DSX_Lib.h
Created:    2/15/2021 10:17 PM
Author:     Jay 

 */

#ifndef DSX_LIB_H
#define DSX_LIB_H


/**** DSX packet data structure */
typedef struct {
	char    ID[20];
	int     loc;
	int     val;
}DSXpacket_t;


/**** MACRO DEFINITIONS ****/
#define FULL                1
#define EMPTY               0
#define MAX_BUFFER_SIZE     50


/**** FUNCTION PROTOTYPES ****/
void receive_packet();
void process_packet();
void initPins();
void pin2IntCount();
void exec_command(DSXpacket_t);
void exec_Dio(int pin, int value);
void getDioMode(int pin);
void exec_digitalRead(int pin);
void getEncoderSpeed();
void getEncoderDir();
void exec_analogRead(int pin);
void exec_pwm(int pin, int value);
void exec_servoWrite(int pin, int value);
void clear_buffer();
bool is_valid_pwm_pin(int pin);
bool is_valid_dio_in_pin(int pin);
bool is_valid_dio_out_pin(int pin);
bool is_valid_analog_pin(int pin);
unsigned char get_buffer_state();
void getSerial();
DSXpacket_t get_packet();


#endif
