#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>

// Khởi tạo đối tượng WiFi và NTP client
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

// Biến lưu giá trị dữ liệu từ các Node
float temp1, humi1, moil1, vol1, Vol_node1, Vol_node2, relay;
float temp2, humi2, moil2, vol2;

// Biến để lưu dữ liệu đến và đi
String Incoming = ""; // hàm lưu dữ liệu gửi đến
String Message = "";  // hàm lưu dữ liệu gửi đi

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
  if (!LoRa.begin(433E6)) { // initialize ratio at 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);  // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");

}

void loop() {
  // Chỉ thực hiện các chức năng nếu thiết bị đang bật
  if (deviceState) {
    unsigned long currentMillis_SendMSG = millis();

    if (currentMillis_SendMSG - previousMillis_SendMSG >= interval_SendMSG) {
      previousMillis_SendMSG = currentMillis_SendMSG;

      Slv++;  // define begin Slv = 0
      if (Slv > 2)
        Slv = 1;

      Message = "SDS" + String(Slv);

      if (Slv == 1) {
        Serial.println("Send message to ESP32 Slave " + String(Slv) + " : " + Message);
        sendMessage(Message, Destination_ESP32_Slave_1);
      }

      if (Slv == 2) {
        Serial.println("Send message to ESP32 Slave " + String(Slv) + " : " + Message);
        sendMessage(Message, Destination_ESP32_Slave_2);
      }
    }

    // Nhận dữ liệu LoRa
    onReceive(LoRa.parsePacket());
  }

  // Đọc trạng thái từ Firestore (luôn kiểm tra để cập nhật trạng thái)
  readDeviceStateFromFirestore();
  readMasterFieldFromFirestore();
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

// Hàm gửi dữ liệu từ LoRa lên Firestore
void sendNodeDataToFirestore(const String& nodeName, float temp,float humi, float moil, float vol) {
  // Kiểm tra trạng thái kết nối WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi chưa kết nối.");
    return;
  }

  HTTPClient http;

  // Cập nhật thời gian từ NTP server
    ntp.update();
    // Lấy thời gian định dạng
    String currentTime = ntp.getFormattedTime();
    // Lấy thời gian Unix Timestamp
    unsigned long epochTime = ntp.getEpochTime();

    // Chuyển đổi epoch time thành ngày tháng năm
    String dateTime = getFormattedDateTime(epochTime);
    Serial.println("Thời gian hiện tại: " + dateTime);

  // Tạo đối tượng JSON để gửi lên Firestore
  StaticJsonDocument<512> doc; // Tăng kích thước nếu cần
  JsonObject fields = doc.createNestedObject("fields");

  // Tạo đối tượng JSON cho node
  JsonObject nodeData = fields.createNestedObject(nodeName).createNestedObject("mapValue").createNestedObject("fields");

  // Gán dữ liệu vào nodeData
  nodeData["temp"]["stringValue"] = (temp >= 0) ? String(temp) : "0.00";
  nodeData["humi"]["stringValue"] = (humi >= 0) ? String(humi) : "0.00";
  nodeData["moil"]["stringValue"] = (moil >= 0) ? String(moil) : "0.00";
  nodeData["vol"]["stringValue"] = (vol >= 0) ? String(vol) : "0.00";
  nodeData["date"]["stringValue"] = dateTime; // Ngày hiện tại

  // Chuyển đổi đối tượng JSON thành chuỗi
  String output;
  serializeJson(doc, output);

  // URL cho Firestore
  String firestoreUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) +
                        "/databases/(default)/documents/data_sub/" + nodeName + "?key=" + String(API_KEY);

  // Gửi yêu cầu PATCH để cập nhật dữ liệu
  http.begin(firestoreUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.PATCH(output);

  // Kiểm tra mã phản hồi từ Firestore
  if (httpResponseCode > 0) {
    Serial.println("Dữ liệu cho " + nodeName + " đã được cập nhật thành công.");
  } else {
    Serial.println("Lỗi khi cập nhật dữ liệu: " + String(httpResponseCode));
  }

  http.end(); // Kết thúc yêu cầu HTTP
}

// Hàm để chuyển đổi epoch time thành định dạng ngày tháng năm
String getFormattedDateTime(unsigned long epochTime) {
  // Chuyển đổi thời gian epoch sang struct tm
  struct tm *ptm = gmtime((time_t *)&epochTime);
  
  // Định dạng thành chuỗi ngày tháng năm
  char buffer[30];
  sprintf(buffer, "%02d-%02d-%04d %02d:%02d:%02d",
          ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year + 1900,
          ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  return String(buffer);
}


// Hàm lấy ngày hiện tại (giữ một định nghĩa duy nhất)
String getCurrentDate() {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%d-%m-%Y", &tstruct);
  return String(buf);
}

// Trong hàm onReceive, sau khi nhận dữ liệu từ LoRa
void onReceive(int packetSize) {
  if (packetSize == 0) return;  // Không có packet thì bỏ qua

  int recipient = LoRa.read();        // Địa chỉ người nhận
  byte sender = LoRa.read();          // Địa chỉ người gửi
  byte incomingLength = LoRa.read();  // Độ dài của tin nhắn
  Incoming = "";  // Xóa dữ liệu cũ

  while (LoRa.available()) {
    Incoming += (char)LoRa.read();  // Lấy toàn bộ dữ liệu
  }

  if (incomingLength != Incoming.length()) {
    Serial.println("Lỗi: độ dài tin nhắn không khớp");
    return; // Bỏ qua nếu độ dài không khớp
  }

  if (recipient != LocalAddress) {
    Serial.println("Tin nhắn không dành cho tôi.");
    return;
  }

  Serial.println("Nhận từ: 0x" + String(sender, HEX));
  Serial.println("Tin nhắn: " + Incoming);

  // Chỉ cập nhật giá trị cho node cụ thể
  if (sender == Destination_ESP32_Slave_1) {
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &humi1, &temp1, &moil1, &vol1);
    sendNodeDataToFirestore("node1", temp1, humi1, moil1, vol1);
  } 
  else if (sender == Destination_ESP32_Slave_2) {
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &humi2, &temp2,  &moil2, &vol2);
    sendNodeDataToFirestore("node2", temp1, humi1, moil1, vol1);
  }

  // Gọi hàm gửi dữ liệu lên Firestore sau khi đã nhận dữ liệu từ cả hai node
  //sendDataForTwoNodes(temp1, humi1, moil1, vol1, temp2, humi2, moil2, vol2);
}


void readMasterFieldFromFirestore() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi chưa kết nối.");
    return;
  }

  HTTPClient http;

  // Tạo URL truy cập document cụ thể
  String firestoreUrlMaster = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) + "/databases/(default)/documents/data/vFLsuo1WbrY1h6eFIXMb?key=" + String(API_KEY);

  http.begin(firestoreUrlMaster);  // Bắt đầu yêu cầu HTTP
  int httpResponseCode = http.GET();  // Gửi yêu cầu GET

  if (httpResponseCode > 0) {
    String payload = http.getString();  // Nhận phản hồi từ Firestore
    Serial.println("Dữ liệu nhận được từ Firestore:");

    // Phân tích JSON để lấy giá trị của trường "master"
    StaticJsonDocument<512> doc; // Tăng kích thước nếu cần
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("Không thể phân tích dữ liệu JSON: " + String(error.c_str()));
      return;
    }

    // Kiểm tra trường "master" và tự tạo nếu cần
    JsonArray masterArray = doc["fields"]["master"]["arrayValue"]["values"];
    if (masterArray.size() == 0) {
      // Tạo một mảng mới nếu "master" trống
      JsonObject newNode = masterArray.createNestedObject();
      JsonObject newFields = newNode.createNestedObject("mapValue").createNestedObject("fields");
      
      newFields["name"]["stringValue"] = "node_default";
      newFields["temp"]["stringValue"] = "0.00";
      newFields["humi"]["stringValue"] = "0.00";
      newFields["moil"]["stringValue"] = "0.00";
      newFields["vol"]["stringValue"] = "0.00";
      newFields["date"]["stringValue"] = getCurrentDate();

      Serial.println("Đã tạo node mặc định trong trường 'master'.");
    }

    Serial.println("Dữ liệu trong trường 'master':");
    for (JsonObject masterObject : masterArray) {
      JsonObject fields = masterObject["mapValue"]["fields"];
      
      // Lấy các trường cần thiết
      String name = fields["name"]["stringValue"] | "N/A";
      String temp = fields["temp"]["stringValue"] | "N/A";
      String humi = fields["humi"]["stringValue"] | "N/A";
      String moil = fields["moil"]["stringValue"] | "N/A";
      String vol = fields["vol"]["stringValue"] | "N/A";
      String date = fields["date"]["stringValue"] | "N/A";

      // Hiển thị dữ liệu
      Serial.println("Node Name: " + name);
      Serial.println("Temperature: " + temp);
      Serial.println("Humidity: " + humi);
      Serial.println("Moisture: " + moil);
      Serial.println("Volume: " + vol);
      Serial.println("Date: " + date);
      Serial.println("----------------------");  // Ngăn cách giữa các node
    }
  } else {
    Serial.println("Không thể lấy dữ liệu từ Firestore, mã lỗi: " + String(httpResponseCode));
  }

  http.end();  // Kết thúc yêu cầu HTTP
}

// Hàm đọc trạng thái từ Firestore
void readDeviceStateFromFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(firestoreUrl);  // URL Firestore đã được khai báo trước
    int httpResponseCode = http.GET();  // Gửi yêu cầu GET

    if (httpResponseCode > 0) {
      String payload = http.getString();  // Nhận phản hồi từ Firestore
      Serial.println("Dữ liệu nhận được từ Firestore:");
      //Serial.println(payload);

      // Phân tích JSON để lấy giá trị của trường "control"
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.println("Không thể phân tích dữ liệu JSON: " + String(error.c_str()));
        return;
      }

      // Lấy giá trị của trường "control"
      int controlValue = doc["fields"]["control"]["integerValue"];
      Serial.println("Giá trị của 'control': " + String(controlValue));

      // Cập nhật trạng thái thiết bị dựa trên giá trị "control"
      if (controlValue == 1) {
        deviceState = true;
        Serial.println("Thiết bị đang BẬT");
      } else if (controlValue == 0) {
        deviceState = false;
        Serial.println("Thiết bị đang TẮT");
      }

      // Điều khiển thiết bị theo trạng thái mới
      controlDevice(deviceState);
    } else {
      Serial.println("Không thể lấy dữ liệu từ Firestore, mã lỗi: " + String(httpResponseCode));
    }

    http.end();  // Kết thúc yêu cầu HTTP
  } else {
    Serial.println("Wi-Fi chưa kết nối.");
  }
}

// Hàm điều khiển thiết bị
void controlDevice(bool state) {
  if (state) {
    // Thực hiện các hành động khi bật thiết bị
    Serial.println("Thiết bị đang BẬT.");
  } else {
    // Thực hiện các hành động khi tắt thiết bị
    Serial.println("Thiết bị đang TẮT.");
  }
}