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
void gioInit();
void exec_command(DSXpacket_t);
void exec_Dio(int pin, int value);
void exec_configDio(int pin, int config);
void exec_pwm(int pin, int value);
bool is_valid_pwm_pin(int pin);
bool is_valid_dio_pin(int pin);
unsigned char get_buffer_state();
DSXpacket_t get_packet();


#endif
