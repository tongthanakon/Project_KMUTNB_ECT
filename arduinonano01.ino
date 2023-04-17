#include <SoftwareSerial.h>
#include <SPI.h>
#include <LoRa.h>

#define ss 10
#define rst 9
#define dio0 2

#define RO_PIN 8
#define DI_PIN 7
#define Relay 3

SoftwareSerial rs485(RO_PIN, DI_PIN);

byte Address = 0x02;
byte receiverAddress = 0x01;

float humidity1, humidity2;
String Status;

long lastSendTime = 0;
int interval = 2000;

void setup() {
  pinMode(Relay, OUTPUT);
  Serial.begin(115200);
  rs485.begin(4800);
  LoRa.setPins(ss, rst, dio0);
  while (!Serial);
  Serial.println("LoRa Connect Server");
  if (!LoRa.begin(925E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  digitalWrite(Relay, 1);
  LoRa.onReceive(onReceive);
  LoRa.receive();
}

void rs485_sersor() {
  if (rs485.find("\0x01\0x03")) {
    byte command[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
    rs485.write(command, sizeof(command));
    delay(1000);
    byte response[8];
    int i = 0;
    while (rs485.available()) {
      response[i] = rs485.read();
      i++;
    }
    /*for (int j = 0; j < sizeof(response); j++) {
      Serial.print(response[j], HEX);
      Serial.print(" ");
      }*/
    humidity1 = (response[3] * 256 + response[4]) / 10.0;
  }
  if (rs485.find("\0x02\0x03")) {
    byte command[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x39};
    rs485.write(command, sizeof(command));
    delay(1000);
    byte response[8];
    int i = 0;
    while (rs485.available()) {
      response[i] = rs485.read();
      i++;
    }
    /*for (int j = 0; j < sizeof(response); j++) {
      Serial.print(response[j], HEX);
      Serial.print(" ");
      }*/
    humidity2 = (response[3] * 256 + response[4]) / 10.0;
  }
}

void loop() {
  if (millis() - lastSendTime > interval) {
    if (Status == "ON") {
      rs485_sersor();
      String msg1 = String(humidity1);
      String msg2 = String(humidity2);
      String sendmsg = msg1 + "-" + msg2;
      sendMessage(sendmsg);
      Serial.println(sendmsg);
      lastSendTime = millis();
      interval = random(4000) + 1000;
    } else {
      humidity1 = 0;
      humidity2 = 0;
      String msg1 = String(humidity1);
      String msg2 = String(humidity2);
      String sendmsg = msg1 + "-" + msg2;
      sendMessage(sendmsg);
      digitalWrite(Relay, 1);
      Serial.println(sendmsg);
      lastSendTime = millis();
      interval = random(4000) + 1000;
    }
  }
  LoRa.onReceive(onReceive);
  LoRa.receive();
  delay(2000);
}


void sendMessage(String message) {
  LoRa.beginPacket();
  LoRa.write(Address);  //ที่อยู่ผู้ส่ง
  LoRa.write(receiverAddress); //ที่อยู่ผู้รับ
  LoRa.print(message);  //ข้อความที่จะส่ง
  LoRa.endPacket();
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;

  byte senderAddress = LoRa.read();
  byte recvAddress = LoRa.read();
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  if (recvAddress == Address) {
    if (String(senderAddress, HEX) == "1") {  //ที่อยู่ 0x01 ส่งมา
      String System = message.substring(0, message.indexOf("-"));
      String msg = message.substring(message.indexOf("-") + 1);
      Serial.println(System + " " + msg);
      if (System == "Auto" || System == "Manual") {
        Status = "ON";
        if (System == "Auto") {
          float humi = msg.toFloat();
          if ((humidity1 <= humi) && (humidity2 <= humi)) digitalWrite(Relay, 0);
          else digitalWrite(Relay, 1);
        } else {
          if (msg == "ON") digitalWrite(Relay, 0);
          else  digitalWrite(Relay, 1);
        }
      } else Status = "OFF";
    }
  }
}
