// This program will read the input buffer as a char string
// and parse it and perform the actions
// Author: Jay Pacamarra

typedef struct {
  char  ID[20];
  int   loc;      // pin number on Arduino Uno
  int   val;      
}DSXpacket_t;

// VARIABLE DECLERATIONS
char Buffer[50];            // Allocate space for the string
unsigned char buffer_state; // Current state of buffer (full, empty)
char inChar;                // Stores the current character being read 
unsigned char index = 0;    // Index into the char array
char keyword[] = "dsx";     // keyword, used for parsing
DSXpacket_t DSXpacket;      // Init DSXpacket data structure

// MACRO DEFENITIONS
#define FULL  1
#define EMPTY 0

// FUNCTION PROTOTYPES
void receive_packet();   
void process_packet();
void gioInit(); 

void setup() {
  Serial.begin(9600,SERIAL_8O1);

  buffer_state = EMPTY; // buffer is initially empty

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


void gioInit() {
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
  
}


void receive_packet() {
    // Store characters one byte at a time into the Buffer
    inChar = Serial.read();
    char tempBuffer[50];  // temp buffer
    tempBuffer[index]=inChar;
    index++;

    // new line character detected, end of packet
    // or max allowed buffer size is reached
    if(inChar == '\n' || index == 49){
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
