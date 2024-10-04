#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Thông tin Wi-Fi
#define WIFI_SSID "Hai Ha F2"
#define WIFI_PASSWORD "haihadam"

// Thông tin Firebase
#define FIREBASE_HOST "https://duaniot-2c8b0-default-rtdb.firebaseio.com/" 
#define FIREBASE_AUTH "O8JJvDN8HbYm2F7YzVaOqrR9CbkChiBO9PzGpZn7"

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// LoRa Pin / GPIO configuration
#define ss 5
#define rst 14
#define dio0 2

// Biến lưu giá trị dữ liệu từ các Node
float temp1, humi1, moil1, vol1, Vol_node1, Vol_node2, relay;
float temp2, humi2, moil2, vol2;

// Biến để lưu dữ liệu đến và đi
String Incoming = ""; // hàm lưu dữ liệu gửi đến
String Message = "";  // hàm lưu dữ liệu gửi đi

// Cấu hình truyền dữ liệu LoRa
byte LocalAddress = 0x10;               // address of this device (Master Address)
byte Destination_ESP32_Slave_1 = 0x02;  // destination to send to Slave 1 (ESP32)
byte Destination_ESP32_Slave_2 = 0x03;  // destination to send to Slave 2 (ESP32)

// Biến khai báo cho timer
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 4000;

// Biến đếm Slave
byte Slv = 0;

// Hàm gửi dữ liệu LoRa
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             // start packet
  LoRa.write(Destination);        // add destination address
  LoRa.write(LocalAddress);       // add sender address
  LoRa.write(Outgoing.length());  // add payload length
  LoRa.print(Outgoing);           // add payload
  LoRa.endPacket();               // finish packet and send it
}

// Hàm nhận dữ liệu LoRa
void onReceive(int packetSize) {
  if (packetSize == 0) 
    return;  // if there's no packet, return

  int recipient = LoRa.read();        // recipient address
  byte sender = LoRa.read();          // sender address
  byte incomingLength = LoRa.read();  // incoming msg length
  Incoming = ""; // Clears Incoming variable data

  while (LoRa.available()) {
    Incoming += (char)LoRa.read();  // Get all incoming data
  }

  if (incomingLength != Incoming.length()) {
    Serial.println("error: message length does not match length");
    return; // skip rest of function
  }

  // Kiểm tra xem địa chỉ gửi có khớp với địa chỉ của Master không
  if (recipient != LocalAddress) {
    Serial.println("This message is not for me.");
    return; // skip rest of function
  }

  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Message: " + Incoming);

  // Parse the incoming message and assign values to temp, humi, moil, vol tương ứng với sender
  if (sender == Destination_ESP32_Slave_1) {
    // Nếu tin nhắn từ Node 1
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &temp1, &humi1, &moil1, &vol1);

    // Gửi dữ liệu vừa nhận được lên Firebase
    sendDataToFirebase("/Node1/temp", temp1);
    sendDataToFirebase("/Node1/humi", humi1);
    sendDataToFirebase("/Node1/moil", moil1);
    sendDataToFirebase("/Node1/vol", vol1);
  } 
  else if (sender == Destination_ESP32_Slave_2) {
    // Nếu tin nhắn từ Node 2
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &temp2, &humi2, &moil2, &vol2);

    // Gửi dữ liệu vừa nhận được lên Firebase
    sendDataToFirebase("/Node2/temp", temp2);
    sendDataToFirebase("/Node2/humi", humi2);
    sendDataToFirebase("/Node2/moil", moil2);
    sendDataToFirebase("/Node2/vol", vol2);
  }
}

void setup() {
  Serial.begin(115200);

  // Kết nối Wi-Fi
  connectToWiFi();

  // Cấu hình Firebase
  configureFirebase();

  // Kiểm tra kết nối Firebase
  checkFirebaseConnection();

  // Settings and start LoRa Ra-02
  LoRa.setPins(ss, rst, dio0);

  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) { // initialize ratio at 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true); // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
}

void loop() {
  unsigned long currentMillis_SendMSG = millis();
  
  if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
    previousMillis_SendMSG = currentMillis_SendMSG;

    Slv++; // define begin Slv = 0
    if (Slv > 2) 
        Slv = 1;

    Message = "SDS" + String(Slv);

    if (Slv == 1) {
      Serial.println("Send message to ESP32 Slave " + String(Slv) + " : " + Message);
      sendMessage(Message, Destination_ESP32_Slave_1);
    }

    // Add condition for sending to Slave 2 if needed
    if (Slv == 2) {
      Serial.println("Send message to ESP32 Slave " + String(Slv) + " : " + Message);
      sendMessage(Message, Destination_ESP32_Slave_2);
    }
  }

  onReceive(LoRa.parsePacket());
}

// Kết nối Wi-Fi
void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");
}

// Cấu hình Firebase
void configureFirebase() {
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
}

// Kiểm tra kết nối Firebase
void checkFirebaseConnection() {
  if (Firebase.ready()) {
    Serial.println("Connected to Firebase");
  } else {
    Serial.println("Failed to connect to Firebase");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

// Hàm gửi dữ liệu lên Firebase
void sendDataToFirebase(String path, float value) {
  if (Firebase.setFloat(firebaseData, path, value)) {
    Serial.println(path + " sent: " + String(value));
  } else {
    Serial.println("Failed to send " + path + " data");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}