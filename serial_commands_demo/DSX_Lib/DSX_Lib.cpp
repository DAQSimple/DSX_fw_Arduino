/*

Name:       DSX_Lib.cpp
Created:    2/15/2021 10:17 PM
Author:     Jay 

*/

#include "Arduino.h"
#include "DSX_Lib.h"


/**** VARIABLE DECLERATIONS ****/
char Buffer[50];            // Allocate space for the string
unsigned char buffer_state = EMPTY; // Current state of buffer (full, empty)
char inChar;                // Stores the current character being read 
unsigned char index = 0;    // Index into the char array
char keyword[] = "dsx";     // keyword, used for parsing
DSXpacket_t DSXpacket;      // Init DSXpacket data structure


void gioInit() {
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
}


void receive_packet() {
    // Store characters one byte at a time into the Buffer
    inChar = Serial.read();
    char tempBuffer[50];  // temp buffer
    tempBuffer[index]=inChar;
    index++;

    // new line character detected, end of packet
    if(inChar == '\n'){
      buffer_state = FULL;
      strncpy(Buffer, tempBuffer, sizeof(Buffer));
      index=0;
      strncpy(tempBuffer,"",sizeof(Buffer));
    }
}

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
    char parsedStrings[3][20];    // init 5 character strings with each having 20 bytes 
    char *token = strtok(goodData, delimeter);
    strncpy(parsedStrings[0], token, sizeof(parsedStrings[0])); // first one
    for(unsigned int i=1; i<3; ++i) {
      token = strtok(NULL, delimeter);
      strncpy(parsedStrings[i],token, sizeof(parsedStrings[i]));
    }

    // CONVERT TO THE CORRECT NUMBER TYPE
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
