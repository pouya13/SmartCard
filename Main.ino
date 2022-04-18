#include <Arduino.h>

#include "SCLib.h"

// If you are using a Arduino Mega compatible board you need to change the SC_C2_CLK to 11 as the TIMER1A
// is used for asynchronous clock generation (1MHz with just plain arduino code is no fun ;-) )
// and the SC_C1_VCC can be changed to any other "free" digital pin.
#define SC_C2_RST              7
#define SC_C1_VCC              11
#define SC_C7_IO               10
#define SC_C2_CLK              9

// Default behavior of the signal connected to SC_SWITCH_CARD_PRESENT is
// that the signal is HIGH, when card is present and LOW otherwise.
#define SC_SWITCH_CARD_PRESENT 8

// If the signal on PIN SC_SWITCH_CARD_PRESENT has an inverted
// characteristic (LOW card present, HIGH otherwise) this can be signaled
// via the following define. The present signal will be inverted by SL44x2 object.
#define SC_SWITCH_CARD_PRESENT_INVERT true

#if !defined(ASYNC_CARDS) && !defined(APDU_SUPPORT)
#error This example only works if ASYNC_CARDS and APDU_SUPPORT is enabled in SCLib.h
#endif

// Create SmartCardReader object for further use
SmartCardReader sc(SC_C7_IO, SC_C2_RST, SC_C1_VCC, SC_SWITCH_CARD_PRESENT, SC_C2_CLK, SC_SWITCH_CARD_PRESENT_INVERT);

bool activated = false;

void setup() {
//  pinMode(6, OUTPUT);
//  digitalWrite(6, HIGH);
  Serial.begin(9600);
}

//
// Just send basic select command to async. smart card (T=0)
//
void loop() {
  while(!Serial);

  APDU_t  command;

  if (!activated){
    activated = send_reset_command();
    //Serial.println(activated);

    
    // The return value starting with 0x61 means that the
    // command was processed succesfully (Might not always
    // be the case. Depends on the used card, but if you receive
    // a return value != 0, this means that the card reacted to
    // your request.
    
    uint8_t data1[2] = {0x7F, 0x20};
    uint8_t data2[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    
    uint16_t result = send_command(command, 0xA0, 0xA4, 0x00, 0x00, data1, 0x02, 0x00, false);
    get_response(result, command);

    result = send_command(command, 0xA0, 0x88, 0x00, 0x00, data2, 0x10, 0x00, true);
    get_response(result, command);
    
    
    
  } else {
    Serial.println("Unable to identify card ... Please remove");
  }
  
  Serial.println("--------------------------------------------");

  delay(10000);
  
//  Deactivate smart card slot (Turn of power etc)
  sc.deactivate();
  activated = false;

  // Wait for card to be removed physicaly from slot
  //while (sc.cardInserted());
}


bool send_reset_command(){
  uint16_t bytes_received = 0;

  Serial.println("Waiting for Smartcard");

  while (!sc.cardInserted());

  Serial.println("Smartcard found");


  uint8_t data[255];

  bytes_received = sc.activate(data, MAX_ATR_BYTES);
  if ( bytes_received > 0) {

    #if defined(SC_DEBUG)
      sc.dumpHEX(data, bytes_received);
    #else
      Serial.print(bytes_received);
      Serial.println(" bytes ATR received from card.");
    #endif

    return true;
  }

  return false;
}

uint16_t send_command(APDU_t command, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t data[], uint16_t data_size, uint16_t resp_size, bool triger){
  //uint8_t new_data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

  //uint8_t data[2] = {0x7F, 0x20};
  // We just use the T=0 byte transfer
  command.cla       = cla;
  command.ins       = ins;
  command.p1        = p1;
  command.p2        = p2;
  command.data_buf  = data;
  command.data_size = data_size;
  command.resp_size = 0x00;
  
  //data[0] = 0x2F;
  //data[1] = 0x00;
  
  
  Serial.println("Sending Command ...");
  // Send Select File Command ...
  //
  // If you experience any problems communicating with your card
  // you might also want to try the maxwaits option with values > 0
  // this will add additional wait "cycles" to the communication stack
  //     uint16_t result = sc.sendAPDU(&command, true, 100);

  uint16_t result;
  
  if (triger){
    pinMode(3, OUTPUT);
    digitalWrite(6, HIGH);
    result = sc.sendAPDU(&command);
    digitalWrite(6, LOW);
  }
  else {
    result = sc.sendAPDU(&command);
  }
  
  Serial.print("Received ... ");
  Serial.println(result, HEX);
  
  return result;
}

void get_response(uint16_t result, APDU_t command){
  uint8_t data[255];
    
  if (((result >> 8) & 0xFF) == 0x9F) {
      uint16_t receive_size = result & 0xFF;
      Serial.println("GET RESPONSE Command ...");
//
      // Read Coammnd result via GET RESPONSE
      command.cla       = 0xC0;
      command.ins       = 0xC0;
      command.p1        = 0x00;
      command.p2        = 0x00;
      command.data_buf  = data;
      command.data_size = receive_size;

//      // If you experience any problems communicating with your card
//      // you might also want to try the maxwaits option with values > 0
//      // this will add additional wait "cycles" to the communication stack
//      //
//      //result = sc.sendAPDU(&command, false, 1);
      result = sc.sendAPDU(&command, false);
      
      Serial.print("Received ... ");
      Serial.println(result, HEX);
      
      #if defined(SC_DEBUG)
            sc.dumpHEX(data, receive_size);
      #else
        Serial.print(receive_size);
        Serial.println(" bytes as command response received from card.");
      #endif

  }
}
