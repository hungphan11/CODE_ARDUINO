#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <NTPClient.h>

// Cấu hình NTP
WiFiUDP udp;
NTPClient ntp(udp, "pool.ntp.org", 25200); // 25200 giây cho múi giờ UTC+7 (Việt Nam)

// Thông tin Wi-Fi
#define WIFI_SSID "Hung"
#define WIFI_PASSWORD "a123456789"

// Thông tin Firebase Firestore
#define API_KEY "AIzaSyD33F_6X-rslRLf7Am8DLsVmS4lferLwt0"
#define PROJECT_ID "test-c7ac1"
String firestoreUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) + "/databases/(default)/documents/system/2Guvl9gNbqPIgiKqU2v1?key=" + String(API_KEY);

// LoRa Pin / GPIO configuration
#define ss 5
#define rst 14
#define dio0 2


// Khai báo servor
Servo myServo;
const int relayPin = 12;                       // Relay kết nối với chân pin 12
unsigned long lastReadTime = 0;                // Thời điểm lần đọc cuối cùng
const unsigned long interval = 1 * 30 * 1000;  // 1 phút (60 giây)

// Biến lưu giá trị dữ liệu từ các Node
float temp1, humi1, moil1, vol1, Vol_node1, Vol_node2, relay;
float temp2, humi2, moil2, vol2;

bool state1 = false;  // Khởi tạo trạng thái của node 1 là tắt
bool state2 = false;  // Khởi tạo trạng thái của node 2 là tắt 

// Biến để lưu dữ liệu đến và đi
String Incoming = "";  // hàm lưu dữ liệu gửi đến
String Message = "";   // hàm lưu dữ liệu gửi đi

// Cấu hình truyền dữ liệu LoRa
byte LocalAddress = 0x10;               // address of this device (Master Address)
byte Destination_ESP32_Slave_1 = 0x01;  // destination to send to Slave 1 (ESP32)
byte Destination_ESP32_Slave_2 = 0x02;  // destination to send to Slave 2 (ESP32)

// Biến khai báo cho timer
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 10000;

// Biến đếm Slave
byte Slv = 0;

// Biến điều khiển thiết bị, mặc định là bật (1)
bool deviceState = true;  // Mặc định bật

// Hàm gửi dữ liệu LoRa
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             // start packet
  LoRa.write(Destination);        // add destination address
  LoRa.write(LocalAddress);       // add sender address
  LoRa.write(Outgoing.length());  // add payload length
  LoRa.print(Outgoing);           // add payload
  LoRa.endPacket();               // finish packet and send it
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
// Hàm kiểm tra và kết nối lại Wi-Fi nếu bị mất kết nối
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi bị mất kết nối, đang thử kết nối lại...");

    // Cố gắng kết nối lại Wi-Fi
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long connectStart = millis();
    bool connected = false;
    
    // Thử kết nối lại trong 10 giây
    while (WiFi.status() != WL_CONNECTED && millis() - connectStart < 10000) {
      delay(500);
      Serial.print(".");  // Hiển thị quá trình kết nối lại
    }

    // Kiểm tra kết nối lại thành công hay không
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Kết nối lại Wi-Fi thành công!");
      connected = true;
    } 
    else {
      Serial.println("Không thể kết nối lại Wi-Fi.");
      connected = false;
    }
    
    // Nếu không kết nối lại được, bạn có thể dừng các tác vụ hoặc tiếp tục chạy ngoại tuyến
    // if (!connected) {
    //   Serial.println("Chuyển sang chế độ ngoại tuyến...");
    //   // Nếu cần, dừng hoặc tạm hoãn các tác vụ yêu cầu kết nối mạng tại đây
    // }
  }
}

//==============================================================================
void setup() {
  Serial.begin(115200);
  // Kết nối Wi-Fi
  connectToWiFi();

  // Bắt đầu NTP client
  ntp.begin();
  ntp.setTimeOffset(25200); // Thiết lập múi giờ cho NTP client

  // Settings and start LoRa Ra-02
  LoRa.setPins(ss, rst, dio0);

  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) {  // initialize ratio at 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ;  // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");

  // Thiết lập relay và servo
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Đảm bảo relay ở trạng thái tắt ban đầu
  myServo.attach(25);           // Giả sử servo kết nối với chân pin 9
}

void loop() {
  // Kiểm tra kết nối Wi-Fi trước khi thực hiện các tác vụ khác
  checkWiFiConnection();
  
  // Chỉ thực hiện các chức năng nếu Wi-Fi đang kết nối
  if (WiFi.status() == WL_CONNECTED) {
    // Thực hiện các tác vụ yêu cầu Wi-Fi
    if (deviceState) {
      unsigned long currentMillis_SendMSG = millis();

      if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
        previousMillis_SendMSG = currentMillis_SendMSG;

        Slv++;  // define begin Slv = 0
        if (Slv > 2)
          Slv = 1;
        Message = "SDS" + String(Slv);
        if (Slv == 1) {
          Serial.println("Gửi tin nhắn đến ESP32 Slave " + String(Slv) + " : " + Message);
          sendMessage(Message, Destination_ESP32_Slave_1);
        }
        if (Slv == 2) {
          Serial.println("Gửi tin nhắn đến ESP32 Slave " + String(Slv) + " : " + Message);
          sendMessage(Message, Destination_ESP32_Slave_2);
        }
      }
      // Nhận dữ liệu LoRa
      onReceive(LoRa.parsePacket());
    }

    // Đọc trạng thái từ Firestore (luôn kiểm tra để cập nhật trạng thái)
    readDeviceStateFromFirestore();
    checkAndControlAtInterval();
  } 
  else {
    Serial.println("Không có kết nối Wi-Fi, tạm dừng các tác vụ yêu cầu mạng.");
    // Thực hiện các tác vụ ngoại tuyến (nếu có)
    // Hoặc đơn giản bỏ qua các tác vụ yêu cầu Wi-Fi
  }
}
