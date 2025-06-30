#include <esp32_can.h> /* https://github.com/collin80/esp32_can */

struct can_struct{
  uint16_t id;
  uint8_t length;
  uint8_t message[8];
};

can_struct CANMessage;

#define  SHIELD_LED_PIN   26
#define  SHIELD_LED_PIN_2 27

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("===CAN Test===");
  
  CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5);
  
  // Initialize CAN
  if(CAN0.begin(500000) == 0) {
    Serial.println("ERROR: Cannot initialize CAN!");
    while(1) delay(1000);
  }
  
  Serial.println("CAN initialized successfully");
  
  CAN0.watchFor();
  
  Serial.println("Starting test...");
  
  CAN_FRAME testMsg;
  testMsg.id = 0x123;
  testMsg.extended = false;
  testMsg.length = 8;
  testMsg.data.byte[0] = 0xAA;
  testMsg.data.byte[1] = 0xBB;
  testMsg.data.byte[2] = 0xCC;
  testMsg.data.byte[3] = 0xDD;
  testMsg.data.byte[4] = 0xEE;
  testMsg.data.byte[5] = 0xFF;
  testMsg.data.byte[6] = 0x11;
  testMsg.data.byte[7] = 0x22;
  
  // Try to send a message
  Serial.println("Attempting to send test message...");
  if(CAN0.sendFrame(testMsg)) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Failed to send message - check termination/connections");
  }
  
  Serial.println("Listening for any CAN activity...");
}

void loop() {
  static unsigned long lastPrint = 0;
  
  if(CAN0.available() > 0) {
    CAN_FRAME msg;
    if(CAN0.read(msg)) {
      Serial.print("RX: ID=0x");
      Serial.print(msg.id, HEX);
      Serial.print(" DATA=[");
      for(int i = 0; i < msg.length; i++) {
        if(i > 0) Serial.print(" ");
        if(msg.data.byte[i] < 0x10) Serial.print("0");
        Serial.print(msg.data.byte[i], HEX);
        }
        Serial.println("]");
    }
  }
  
  // Print status every 5 seconds
  if(millis() - lastPrint > 5000) {
    lastPrint = millis();
    Serial.println("Still listening... (check bus activity and connections)");
    
    if(CAN0.isFaulted()) {
      Serial.println("CAN BUS FAULT DETECTED");
    }
  }
  
  delay(1);
}