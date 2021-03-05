// This demo program will read the input buffer as a char string
// and parse it and perform the actions
// Author: Jay Pacamarra


#include "DSX_Lib.h"

DSXpacket_t packet; // create a DSXpacket object to store ID,loc,val info


void setup() {
  Serial.begin(9600);
  initPins();
  initEncoder();
}

void loop() {
  while (Serial.available() > 0) {
    receive_packet();       // Buffer is updated with incoming packet until FULL
  }
  if (get_buffer_state() == FULL) {
    process_packet();       // Parse keyword and data after keyword
    packet = get_packet();  // Save packet (contains ID, loc, and val)
    exec_command(packet);   // Execute predefined commands based on received packet
  }

  // Update encoder readings every second, no delays were used
  readEncoder();

}
