#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

//Wi-Fi FIREBASE
#define WIFI_SSID "THANAKON 2G"  //ชื่อ wifi
#define WIFI_PASSWORD "07082543" //รหัส wifi
#define FIREBASE_HOST "project-app-2fcd6-default-rtdb.firebaseio.com" //project-app-2fcd6-default-rtdb.firebaseio.com
#define FIREBASE_AUTH "2eYbcLPZI1QgFkdNn098kT31Br8PFTHqHGuBeapv" //2eYbcLPZI1QgFkdNn098kT31Br8PFTHqHGuBeapv
FirebaseData firebaseData;

#define ss 5
#define rst 14
#define dio0 2

byte Address = 0x01;

float humi1, humi2, humi3, humi4;
String sendmsg1, sendmsg2, System1, System2, Status1, Status2, Set1, Set2;

long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  LoRa.setPins(ss, rst, dio0);
  while (!Serial);
  Serial.println("LoRa Connect ");
  if (!LoRa.begin(925E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  Firebase.getString(firebaseData, "/Controller1/Controller1/System");
  System1 = firebaseData.stringData();
  Firebase.getString(firebaseData, "/Controller2/Controller2/System");
  System2 = firebaseData.stringData();

}

void loop() {
  if (millis() - lastSendTime > interval) {
    Firebase.getString(firebaseData, "/Controller1/Controller1/System");
    System1 = firebaseData.stringData();
    Firebase.getString(firebaseData, "/Controller2/Controller2/System");
    System2 = firebaseData.stringData();
    Firebase.getString(firebaseData, "/Controller1/Controller1/Status");
    Status1 = firebaseData.stringData();
    Firebase.getString(firebaseData, "/Controller2/Controller2/Status");
    Status2 = firebaseData.stringData();
    Firebase.getString(firebaseData, "/Controller1/Controller1/Set");
    Set1 = firebaseData.stringData();
    Firebase.getString(firebaseData, "/Controller2/Controller2/Set");
    Set2 = firebaseData.stringData();

    if (System1 == "Auto") sendmsg1 = System1 + "-" + Set1;
    else if (System1 == "Manual") sendmsg1 = System1 + "-" + Status1;
    else sendmsg1 = System1 + "-" + "Waite";

    if (System2 == "Auto") sendmsg2 = System2 + "-" + Set2;
    else if (System2 == "Manual") sendmsg2 = System2 + "-" + Status2;
    else sendmsg2 = System2 + "-" + "Waite";


    sendMessage(sendmsg1 , 0x02);
    Serial.println("01: " + sendmsg1);
    sendMessage(sendmsg2 , 0x03);
    Serial.println("02: " + sendmsg2);
    lastSendTime = millis();
    interval = random(2000) + 1000;
    LoRa.receive();
  }
  onReceive(LoRa.parsePacket());
}

void sendMessage(String message, byte receiverAddress) {
  LoRa.beginPacket();
  LoRa.write(Address);
  LoRa.write(receiverAddress);
  LoRa.print(message);
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
    if (String(senderAddress, HEX) == "2") {  //ที่อยู่ 0x02 ส่งมา
      String status1 = message.substring(0, message.indexOf("-"));
      String status2 = message.substring(message.indexOf("-") + 1);
      humi1 = status1.toFloat();
      humi2 = status2.toFloat();
      Firebase.setFloat(firebaseData, "/Controller1/Controller1/Humidity1", humi1);
      Firebase.setFloat(firebaseData, "/Controller1/Controller1/Humidity2", humi2);
      delay(1000);
      Serial.print("01: ");
      Serial.print(humi1);
      Serial.print(" ");
      Serial.println(humi2);
    }
    if (String(senderAddress, HEX) == "3") { //ที่อยู่ 0x03 ส่งมา
      String status1 = message.substring(0, message.indexOf("-"));
      String status2 = message.substring(message.indexOf("-") + 1);
      humi3 = status1.toFloat();
      humi4 = status2.toFloat();
      Firebase.setFloat(firebaseData, "/Controller2/Controller2/Humidity1", humi3);
      Firebase.setFloat(firebaseData, "/Controller2/Controller2/Humidity2", humi4);
      delay(1000);
      Serial.print("02: ");
      Serial.print(humi3);
      Serial.print(" ");
      Serial.println(humi4);
    }
  }
}
