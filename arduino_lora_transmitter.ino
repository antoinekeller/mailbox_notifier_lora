#include <SPI.h>
#include <LoRa.h>
int pot = A0;
byte msgCount = 0;        // count of outgoing messages
byte localAddress = 0xFF; // address of this device
byte destination = 0xBB;  // destination to send to

void setup()
{
  Serial.begin(19200);
  pinMode(pot, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  while (!Serial)
    ;
  Serial.println("Reset LoRa Sender");
  if (!LoRa.begin(868E6))
  { // or 915E6, the MHz speed of yout module
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  delay(50);
  Serial.println("Succeeded Init LoRa Sender");
  delay(50);
}

void loop()
{
  bool is_open = analogRead(A0);
  if (is_open)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Mailbox is open");
    Serial.println(msgCount);
    LoRa.beginPacket();
    //LoRa.write(destination);              // add destination address
    //LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);
    String outgoing = "Mailbox is open";
    LoRa.write(outgoing.length());
    LoRa.print(outgoing);
    LoRa.endPacket();
    delay(50);
    msgCount++;
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(10);
  }
}
