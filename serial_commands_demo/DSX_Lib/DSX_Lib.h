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


#endif