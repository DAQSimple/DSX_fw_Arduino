// This demo program will read the input buffer as a char string
// and parse it and perform the actions
// Author: Jay Pacamarra


#include <DSX_Lib.h>

DSXpacket_t packet;


void setup() {
  Serial.begin(9600);
  gioInit();
  
}

void loop() {
  while(Serial.available() > 0){
    receive_packet();       // Buffer is updated with incoming packet until FULL
  }
  if(get_buffer_state() == FULL) {
    process_packet();       // Parse keyword and data after keyword
    packet = get_packet();
    exec_command(packet);     // Execute predefined commands based on received packet
  }
   
}
