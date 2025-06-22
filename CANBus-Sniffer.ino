

#include <esp32_can.h> /* https://github.com/collin80/esp32_can */

struct can_struct{
  uint16_t id;
  uint8_t length;
  uint8_t message[8];
};

can_struct CANMessage;

#define  SHIELD_LED_PIN   26
#define  SHIELD_LED_PIN_2 27

void setup()
{
  Serial.begin(115200);

  pinMode(SHIELD_LED_PIN, OUTPUT);
  pinMode(SHIELD_LED_PIN_2, OUTPUT);

  digitalWrite(SHIELD_LED_PIN, LOW);
  digitalWrite(SHIELD_LED_PIN_2, LOW); 

  Serial.println(" CAN...............INIT");
  CAN0.setCANPins(GPIO_NUM_5, GPIO_NUM_4); //config for shield v1.3+, see important note above!
  CAN0.begin(500000); // 500Kbps
  CAN0.watchFor();
  digitalWrite(SHIELD_LED_PIN, HIGH);
  Serial.println(" CAN............500Kbps");

}


void loop()
{
  CAN_FRAME can_message;

  Serial.println("Reading");
  Serial.println(CAN0.read(can_message));

  if (CAN0.read(can_message))
  {
    digitalWrite(SHIELD_LED_PIN_2, HIGH);
    Serial.print("CAN MSG: 0x");
    Serial.print(can_message.id, HEX);
    Serial.print(" [");
    Serial.print(can_message.length, DEC);
    Serial.print("] <");
    for (int i = 0; i < can_message.length; i++)
    {
      if (i != 0) Serial.print(":");
      Serial.print(can_message.data.byte[i], HEX);
    }
    Serial.println(">");
  }
}