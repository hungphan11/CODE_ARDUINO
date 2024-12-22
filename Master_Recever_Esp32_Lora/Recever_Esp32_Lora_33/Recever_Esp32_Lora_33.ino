#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <EEPROM.h>

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

//================================================================ thêm 

String firestoreUrlWarter = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) + "/databases/(default)/documents/analytics_sub/analytics_sub?key=" + String(API_KEY);

// Cảm biến lưu lượng nước
const int flowPin = 4;
const float flowFactor = 7.5;  // Đảm bảo giá trị này hợp lý (không phải 0)
volatile int pulseCount = 0;

// Biến để lưu ngày hiện tại theo định dạng dd-mm-yyyy
String currentDayStr = "";  // lưu Ngày hiện tại
float consumedMonth[12] = {0}; // Lượng nước tiêu thụ hàng tháng, khởi tạo bằng 0
float dailyFlow = 0;           // Lượng nước tiêu thụ trong ngày    
unsigned long previousMillisSend = 0;
const long intervalSend = 40000;   // thời gian chạy chương trình tính lượng nước
unsigned long previousMillisSend_1 = 0;
const long intervalSend_1 = 60000;   // thời gian lấy api thời tiết 

// Cấu hình EEPROM
const int EEPROM_SIZE = sizeof(consumedMonth); // Kích thước cần cho mảng
const int DAY_ADDRESS = 0;        // Địa chỉ lưu trữ ngày hiện tại
const int DAILY_FLOW_ADDRESS = 10; // Địa chỉ lưu trữ lượng nước ngày
const int MONTHLY_FLOW_START = 20; // Địa chỉ lưu trữ lưu lượng tháng

//======================== THÔNG SỐ THỜI TIẾT===================
// API URL và Key của OpenWeather
const char* apiUrl = "https://api.openweathermap.org/data/2.5/forecast?q=Buon+Ma+Thuot&appid=201bd92755af93923b3854e767deeed0&lang=vi&units=metric&cnt=3";

// Khai báo các giá trị dữ liệu
float temperature = 0.0;
float humidity = 0.0;
float rain = 0.0;

// Hàm xử lý ngắt khi đếm xung
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}
//================================================================ kết

// LoRa Pin / GPIO configuration
#define ss 5
#define rst 14
#define dio0 2

// Khai báo servor
Servo myServo;
const int relayPin = 12;                       // Relay kết nối với chân pin 12
unsigned long lastReadTime = 0;                // Thời điểm lần đọc cuối cùng

// Biến lưu giá trị dữ liệu từ các Node
float temp1, humi1, moil1, vol1, Vol_node1, Vol_node2, relay;
float temp2, humi2, moil2, vol2;

bool state1 = false;  // Khởi tạo trạng thái của node 1 là tắt
bool state2 = false;  // Khởi tạo trạng thái của node 2 là tắt 

// Khai báo biến để lưu trữ dữ liệu thời gian set
float firstTime, secondTime, thirdTime;

// Biến để lưu dữ liệu đến và đi
String Incoming = "";  // hàm lưu dữ liệu gửi đến
String Message = "";   // hàm lưu dữ liệu gửi đi

// Cấu hình truyền dữ liệu LoRa
byte LocalAddress = 0x10;               // address of this device (Master Address)
byte Destination_ESP32_Slave_1 = 0x01;  // destination to send to Slave 1 (ESP32)
byte Destination_ESP32_Slave_2 = 0x02;  // destination to send to Slave 2 (ESP32)

// Biến khai báo cho timer
unsigned long previousMillis_SendMSG = 0;
const long interval_SendMSG = 60000;  // thời gian thực hiện gửi nhận dữ liệu các node
unsigned long responseTimeout = 5000; // Thời gian chờ phản hồi (5 giây)
unsigned long lastSendTime = 0; // Thời gian gửi tin nhắn gần nhất
bool responseReceived = false; // Biến kiểm tra phản hồi

// Biến đếm Slave
byte Slv = 0;

// Biến điều khiển thiết bị, mặc định là bật (1)
bool deviceState = true;  // Mặc định bật
bool mode = true;         // mac dinh la che do Auto
bool stateNode1 ;   // trang thai cac node khi o che do Manuel
bool stateNode2 ;

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
  connectToWiFi();
  ntp.begin();
  ntp.setTimeOffset(25200); // Thiết lập múi giờ cho NTP client

  //===============
  // Khởi tạo EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // LoRa, relay, servo config
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  Serial.println("LoRa init succeeded.");
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  myServo.attach(26);


  //=================
  // Đọc dữ liệu từ EEPROM
  readDataFromEEPROM();
  // Đọc dữ liệu từ IPA thời tiết
  getWeatherData();

  pinMode(flowPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowPin), pulseCounter, RISING);

  Serial.println("Hệ thống sẵn sàng!");
  //=================

  // Tạo các luồng
  xTaskCreatePinnedToCore(threadSendDataToNodes, "SendData", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(threadReadMode, "ReadMode", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(threadControlNode, "ControlNode", 8192, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(waterNode, "water", 8192, NULL, 1, NULL, 1);

}

void loop() {
  // Không cần sử dụng loop() vì tất cả đều đã được xử lý trong các luồng

  ntp.update();
  updateCurrentDayString();  // Cập nhật currentDayStr mỗi vòng lặp
  //currentDayStr = "20-12-2024";

  // Tiến hành tính toán lưu lượng nước ngày và tháng
  unsigned long epochTime = ntp.getEpochTime();
  int currentMonth = getCurrentMonth(epochTime);
  updateDailyFlow(currentMonth);

}

// Luồng 1: Gửi tín hiệu đến các node và gửi dữ liệu lên Firebase
void threadSendDataToNodes(void *pvParameters) {
  while (true) {
    checkWiFiConnection();
    if (WiFi.status() == WL_CONNECTED ) {
      unsigned long currentMillis = millis();
      if ((unsigned long)(currentMillis - previousMillis_SendMSG) >= interval_SendMSG) {    // Gửi dữ liệu đến các node mỗi 60 giây
        previousMillis_SendMSG = currentMillis;   

        Slv++;  // Khởi tạo Slv = 0
        if (Slv > 2)
          Slv = 1; // Quay lại từ 1 khi vượt quá 2
        Message = "SDS" + String(Slv);
        
        // Gửi tin nhắn đến Slave 1 hoặc 2
        if (Slv == 1) {
          Serial.println("Gửi tin nhắn đến ESP32 Slave " + String(Slv) + " : " + Message);
          sendMessage(Message, Destination_ESP32_Slave_1);
        } 
        else if (Slv == 2) {
          Serial.println("Gửi tin nhắn đến ESP32 Slave " + String(Slv) + " : " + Message);
          sendMessage(Message, Destination_ESP32_Slave_2);
        }

        // Đánh dấu thời gian gửi tin nhắn
        lastSendTime = currentMillis;
        responseReceived = false; // Reset trạng thái phản hồi
      }
      //===== Kiểm tra dữ liệu LoRa và phản hồi =====
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        Serial.println("Nhận được phản hồi");
        onReceive(packetSize);
        responseReceived = true;
        if (deviceState) 
        {
          if (mode) {
            // Tự động điều khiển theo lịch nếu ở chế độ Auto - sẽ đọc trạng thái của node trên firesotre rồi điều khiển
            checkAndControlAtInterval();
          } 
        }
        //readTime("timer", firstTime, secondTime, thirdTime);
      }

      // Kiểm tra thời gian chờ phản hồi
      if (!responseReceived && (currentMillis - lastSendTime >= responseTimeout)) {
        Serial.println("Không nhận được phản hồi từ node, gửi lại tin nhắn...");
        // Có thể gửi lại tin nhắn nếu cần thiết, hoặc xử lý lỗi ở đây
        if (Slv == 1) {
          sendMessage(Message, Destination_ESP32_Slave_1);
        } else if (Slv == 2) {
          sendMessage(Message, Destination_ESP32_Slave_2);
        }
      }
    }
    //delay(5000); // Chạy lại sau 1 phút
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
  }
}

// Luồng 2: Đọc trạng thái `control`, `manual`, `auto`
void threadReadMode(void *pvParameters) {
  while (true) {
    checkWiFiConnection();
    if (WiFi.status() == WL_CONNECTED) {
      readDeviceOnOff();
      readManualAuto("mode");
    }
    //delay(5000); // Chạy mỗi giây để cập nhật trạng thái thường xuyên
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
  }
}
// Luồng 3: Điều khiển bật/tắt các node và theo thời gian
void threadControlNode(void *pvParameters) {
  while (true) {
    if (deviceState) {
      if (mode) {
        // Tự động điều khiển theo lịch nếu ở chế độ Auto
        //readTime("timer", firstTime, secondTime, thirdTime);
        controlAllNodesTime();
        
      } 
      else {
        // Kiểm tra trạng thái thủ công khi ở chế độ Manuel
        //readNodeState("node1State");
        //readNodeState("node2State");
        controlNodeManuel("node1State");
        controlNodeManuel("node2State");
      }
    }
    //delay(5000); // Kiểm tra bật/tắt mỗi giây
    vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
  }
}

// Luồng 4: Tính toán lượng nước 
void waterNode(void *pvParameters) {
  while (true) {
    Water_voil();
    //delay(5000); // Kiểm tra bật/tắt mỗi giây
    vTaskDelay(60000 / portTICK_PERIOD_MS); // Delay 60 giây để giảm tải
  }
}


void Water_voil(){
  unsigned long currentMillis = millis();
  //==== Lấy dữ liệu thời tiết 
  if (currentMillis - previousMillisSend_1 >= intervalSend_1) {
    previousMillisSend_1 = currentMillis;
    getWeatherData();
  }
  //===== Chạy chương trình chính
  if (currentMillis - previousMillisSend >= intervalSend) {
    previousMillisSend = currentMillis;
    // Kiểm tra nếu ngày hiện tại đã lưu trong EEPROM khác với ngày hiện tại lấy từ NTP
    if (currentDayStr != getStoredDayFromEEPROM()) {
      // Ngày khác nhau, reset lượng nước và lưu lại ngày vào EEPROM
      Serial.println("Xuất hiện ngày mới");
      resetDailyFlow();
      
      //sendNewDataToFirestore(); // Gửi dữ liệu mới lên Firestore, xóa dữ liệu cũ

      /// PHẢI BẮT BUỘC CHẠY THÀNH CÔNG MỚI ĐƯỢC CHẠY DÒNG TIẾP
      //appendNewMapToFirestore();
      // Kiểm tra kết quả của hàm appendNewMapToFirestore()
      bool appendSuccess = appendNewMapToFirestore();
      
      if (!appendSuccess) {
        // Nếu appendNewMapToFirestore không thành công, return để không thực hiện các lệnh tiếp theo
        Serial.println("Lỗi khi gửi dữ liệu lên Firestore. Quay lại đầu.");
        return;  // Dừng vòng lặp và quay lại lần tiếp theo
      }
      //saveDayToEEPROM(); // Lưu lại ngày mới vào EEPROM

    } 
    else {
      // Ngày không thay đổi, cộng dồn dữ liệu và gửi lên Firestore mà không xóa dữ liệu cũ
      Serial.println("Vẫn ngày cũ");
      //sendNewDataToFirestore();
      updateLatestMapInFirestore();
    }
    Serial.print("Ngày hiện tại: ");
    Serial.println(currentDayStr);
    saveDataToEEPROM();
  }
}
