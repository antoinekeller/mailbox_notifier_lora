#include <SPI.h>
#include <LoRa.h>
#include "WiFi.h"
#include <HTTPClient.h>

#define BUILTIN_LED 2

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 4
#define EXTERNAL_LED 13

const char *ssid = "WIFI_SSID";
const char *password = "WIFI_PASSWORD";

const char *MESSAGE = "You have something in your mailbox!";
const char *TWILIO_NUMBER = "PHONE_NUMBER_PROVIDED_BY_TWILIO";
const char *MY_PHONE_NUMBER = "YOUR_PHONE_NUMBER";

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += '+';
    }
    else if (isalnum(c))
    {
      encodedString += c;
    }
    else
    {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9)
      {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9)
      {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}

void connectToWiFi()
{
  pinMode(BUILTIN_LED, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to " + String(ssid) + "...");

  int led_state = LOW;

  //Serial.println(WiFi.status());
  //Serial.println(WL_CONNECTED);

  unsigned long start_time = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - start_time > 10000)
    {
      Serial.print("\n");
      Serial.println("Wifi connection failed!");
      digitalWrite(BUILTIN_LED, LOW);
      return;
    }

    Serial.print(".");
    digitalWrite(BUILTIN_LED, led_state);
    led_state = led_state == HIGH ? LOW : HIGH;
    delay(100);
  }

  Serial.println("\n");
  Serial.println("Connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(BUILTIN_LED, HIGH);
  delay(1000);
}

void pushSMS()
{
  if ((WiFi.status() == WL_CONNECTED))
  { //Check the current connection status

    HTTPClient http;

    http.begin("https://api.twilio.com/TWILIO_MESSAGES.json"); //Specify the URL
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    http.setAuthorization("TWILIO_ACCOUNT_SSID", "TWILIO_AUTH_TOKEN");

    String httpRequestData = "Body=" + urlencode(MESSAGE);
    httpRequestData += "&From=" + urlencode(TWILIO_NUMBER);
    httpRequestData += "&To=" + urlencode(MY_PHONE_NUMBER);

    int httpCode = http.POST(httpRequestData); //Make the request

    if (httpCode > 0)
    { //Check for the returning code

      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
    }

    else
    {
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }

  delay(60000); // WAIT A BIT BEFORE NEW SMS
}

void setup()
{

  pinMode(EXTERNAL_LED, OUTPUT);
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(868E6))
  {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  connectToWiFi();
}

void loop()
{

  if ((WiFi.status() != WL_CONNECTED))
  {
    digitalWrite(BUILTIN_LED, LOW);
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH);
  }

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  //Serial.println(packetSize);
  if (packetSize)
  {
    // received a packet
    Serial.print("Received packet with ID ");

    byte incomingMsgId = LoRa.read();
    Serial.println(incomingMsgId);
    byte length = LoRa.read();

    String LoRaData = "";
    // read packet
    if (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.println(LoRaData);
    }

    // print RSSI of packet
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());

    if (LoRaData == "Mailbox is open")
    {
      digitalWrite(EXTERNAL_LED, HIGH);
      if ((WiFi.status() != WL_CONNECTED))
      {
        connectToWiFi();
      }
      pushSMS();
    }
    else
    {
      Serial.println("Unrecognized message! --> Ignore it");
    }
  }
}
