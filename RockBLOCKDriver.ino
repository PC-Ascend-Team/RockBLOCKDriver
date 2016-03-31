#include <Wire.h>
#include <IridiumSBD.h>
#include <SoftwareSerial.h>

SoftwareSerial sSerialRB(11, 12);

IridiumSBD rockBLOCK(sSerialRB, 10);

String message;
boolean recieved = false;
int nRecieved, nTransmissions;
char sCode; // '0' for all good, '1' for somthing went wrong (char for Wire.h simplicity)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  sSerialRB.begin(19200);  
  rockBLOCK.attachConsole(Serial);
  rockBLOCK.attachDiags(Serial);
  rockBLOCK.setPowerProfile(1);
  rockBLOCK.useMSSTMWorkaround(false);
  if(rockBLOCK.begin() != ISBD_SUCCESS) {
      Serial.println("rockBLOCK.begin() failed");
      sCode = '1';
  }
}

void loop() {
  if(recieved) { // i.e. a message has been recieved from the master controller
    
    if(rockBLOCK.sendSBDText(message.c_str()) == ISBD_SUCCESS) {
      Serial.println("Message sent");
      recieved = false; // get ready for next message
      sCode = '0';
      for(int i = 0; i < 6; i++) {
        digitalWrite(LED_BUILTIN, (i % 2 == 1)? HIGH : LOW);
        delay(500);
      }
    } else {
      Serial.println("Message error");
      sCode = '1';
      for(int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, (i % 2 == 1)? HIGH : LOW);
        delay(1000);
        digitalWrite(LED_BUILTIN, (i % 2 == 1)? LOW : HIGH);
        delay(500);
      }
    }

    message.remove(0); // clears out message for next incoming wire
    
  } else { // i.e. waiting for message and or initial GPS fix
    /*if(rockBLOCK.sendSBDText("Waiting for GPS fix/Sending inital transmission") == ISBD_SUCCESS) {
      Serial.println("Message sent");
      sCode = '0';
      for(int i = 0; i < 6; i++) {
        digitalWrite(LED_BUILTIN, (i % 2 == 1)? HIGH : LOW);
        delay(500);
      }
    } else {
      Serial.println("Message error");
      sCode = '1';
      for(int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, (i % 2 == 1)? HIGH : LOW);
        delay(2000);
        digitalWrite(LED_BUILTIN, (i % 2 == 1)? LOW : HIGH);
        delay(500);
      }
    }*/
  }
  
}

/*
 * recieveEvent reads bytes sent from the master controller, appends 
 * it to the message string and then marks a message recieved if it
 * is the last message
 */
void receiveEvent(int nBytes) {
  if(!recieved) { // i.e. rockBLOCK is ready for another message
    while (1 < Wire.available()) {
      char c = Wire.read(); 
      message += c; 
    }
    char eom = Wire.read();
    if(eom == '\0') { // '\0' is the "end of message"(eom) indicator sent by the master controller
      recieved = true; 
    }
  }
}

/*
 * requestEvent sends the sCode back to the master controller
 */
void requestEvent() {
  Wire.write(sCode);
}

/*
 * ISBDCallback is ran during SBD attempts
 */
 bool ISBDCallback() {
  digitalWrite(LED_BUILTIN, (millis() / 1000) % 2 == 1 ? HIGH : LOW);
  return true;
}

