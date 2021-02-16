#include <DSX_Lib.h>

// This program will read the input buffer as a char string
// and parse it and perform the actions
// Author: Jay Pacamarra


extern unsigned char buffer_state;
extern DSXpacket_t DSXpacket;


void setup() {
  Serial.begin(9600);

  gioInit();  
}

void loop() {
  while(Serial.available() > 0){
    receive_packet();       // Buffer is updated with incoming packet until FULL
  }

  if(buffer_state == FULL) {
    process_packet();       // Parse keyword and data after keyword
  }

  /**** MAKE DECISIONS BASED ON PROCESSED PACKET HERE ****/
  if(strcmp(DSXpacket.ID, "Dio") == 0) {
    if(DSXpacket.loc == 11) { 
      if(DSXpacket.val == 0) digitalWrite(DSXpacket.loc, LOW);
      else if(DSXpacket.val == 1) digitalWrite(DSXpacket.loc, HIGH);
    }
    else if(DSXpacket.loc == 13) {
      if(DSXpacket.val == 0) digitalWrite(DSXpacket.loc, LOW);
      else if(DSXpacket.val == 1) digitalWrite(DSXpacket.loc, HIGH);
    }
  }
  else if(strcmp(DSXpacket.ID, "configDio") == 0) {
    if(DSXpacket.loc == 11) { 
      if(DSXpacket.val == 0) pinMode(DSXpacket.loc, INPUT);
      else if(DSXpacket.val == 1) pinMode(DSXpacket.loc, INPUT_PULLUP);
      else if(DSXpacket.val == 2) pinMode(DSXpacket.loc, OUTPUT);
    } 
    else if(DSXpacket.loc == 13) {
      if(DSXpacket.val == 0) pinMode(DSXpacket.loc, INPUT);
      else if(DSXpacket.val == 1) pinMode(DSXpacket.loc, INPUT_PULLUP);
      else if(DSXpacket.val == 2) pinMode(DSXpacket.loc, OUTPUT);
    } 
  }
  else if(strcmp(DSXpacket.ID, "pwm") == 0) {
    if(DSXpacket.loc == 11) {
      if(DSXpacket.val<0) DSXpacket.val=0;
      else if(DSXpacket.val>100) DSXpacket.val=100;
      else
        analogWrite(DSXpacket.loc, map(DSXpacket.val, 0,100, 0,255));
    }
  }
   
  
}
