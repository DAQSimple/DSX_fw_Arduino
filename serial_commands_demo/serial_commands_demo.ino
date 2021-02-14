// This program will read the input buffer as a char string
// and parse it and perform the actions

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
  Serial.begin(9600);

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
      else if(DSXpacket.val>255) DSXpacket.val=100;
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
    if(inChar == '\n'){
      buffer_state = FULL;
      strncpy(Buffer, tempBuffer, sizeof(Buffer));
      index=0;
      strncpy(tempBuffer,"",sizeof(Buffer));
    }
}

void process_packet() {
  Serial.print(Buffer);
  
  char *pointerToFoundData = strstr(Buffer, keyword); //go find keyword
  if (pointerToFoundData != NULL) { // found it
    int positionInString = pointerToFoundData - Buffer;
    Serial.print("Keyword starts at ");
    Serial.println(positionInString);

    //strip good data
    char goodData[50];
    strncpy(goodData, &Buffer[positionInString + strlen(keyword)], sizeof(goodData));
    strncpy(Buffer, "", sizeof(Buffer));
    buffer_state = EMPTY;

    Serial.println(goodData);

    //PARSE *** PARSE *** PARSE *** PARSE *** PARSE *** PARSE *** PARSE ***
    const char delimeter[] = ",";
    char parsedStrings[3][20];    // init 5 character strings with each having 20 bytes 
    char *token = strtok(goodData, delimeter);
    strncpy(parsedStrings[0], token, sizeof(parsedStrings[0])); // first one
    for(unsigned int i=1; i<3; ++i) {
      token = strtok(NULL, delimeter);
      strncpy(parsedStrings[i],token, sizeof(parsedStrings[i]));
    }

    for(unsigned int i=0; i<3; ++i)
      Serial.println(parsedStrings[i]);  //should have the 5 data strings parsed out

    // CONVERT TO THE CORRECT NUMBER TYPE
    strncpy(DSXpacket.ID, parsedStrings[0], sizeof(DSXpacket.ID));
    DSXpacket.loc = atoi(parsedStrings[1]);
    DSXpacket.val = atoi(parsedStrings[2]);

    // Print converted numbers
    Serial.println("Converted");
    Serial.println(DSXpacket.ID);
    Serial.println(DSXpacket.loc);
    Serial.println(DSXpacket.val);    

  }
  else {  // The keyword was not detected
    strncpy(Buffer, "", sizeof(Buffer));
    buffer_state = EMPTY;    
    Serial.println("No keyword detected\n");
  } 
}
