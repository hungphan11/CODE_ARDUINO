#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

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
//==============================================================================
void setup() {
  Serial.begin(115200);
  // Kết nối Wi-Fi
  connectToWiFi();
  // Settings and start LoRa Ra-02
  LoRa.setPins(ss, rst, dio0);

  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) { // initialize ratio at 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);  // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");

}
//==============================================================================
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
  //readMasterFieldFromFirestore();
}

//==============================================================================

// Hàm gửi dữ liệu từ LoRa lên Firestore
// Hàm gửi dữ liệu từ LoRa lên Firestore
void sendDataToFirestore(String nodeName, float temp, float humi, float moil, float vol) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // URL để lấy dữ liệu hiện tại của document "vFLsuo1WbrY1h6eFIXMb" trong collection "data"
    String firestoreGetUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) +
                              "/databases/(default)/documents/data/vFLsuo1WbrY1h6eFIXMb?key=" + String(API_KEY);
    http.begin(firestoreGetUrl);
    int httpResponseCode = http.GET(); // Gửi yêu cầu GET

    if (httpResponseCode > 0) {
      String payload = http.getString();  // Nhận phản hồi từ Firestore
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload); // Phân tích JSON

      if (error) {
        Serial.println("Lỗi phân tích JSON: " + String(error.c_str()));
        return; // Kết thúc hàm nếu có lỗi
      }

      // Tạo một đối tượng mới để thêm vào mảng node1 hoặc node2
      StaticJsonDocument<256> newDoc;
      JsonObject newNode = newDoc.createNestedObject();
      JsonObject newNodeFields = newNode.createNestedObject("mapValue").createNestedObject("fields");
      newNodeFields["name"]["stringValue"] = nodeName; // Thay đổi ở đây
      newNodeFields["temp"]["stringValue"] = String(temp);
      newNodeFields["humi"]["stringValue"] = String(humi);
      newNodeFields["moil"]["stringValue"] = String(moil);
      newNodeFields["vol"]["stringValue"] = String(vol);
      newNodeFields["date"]["stringValue"] = getCurrentDate(); // Thêm ngày hiện tại

      // Lấy mảng node1 hoặc node2 hiện tại
      JsonArray nodeArray;
      if (nodeName == "node1") {
        nodeArray = doc["fields"]["node1"]["arrayValue"]["values"];
      } else if (nodeName == "node2") {
        nodeArray = doc["fields"]["node2"]["arrayValue"]["values"];
      } else {
        Serial.println("Tên node không hợp lệ: " + nodeName);
        return; // Kết thúc hàm nếu tên node không hợp lệ
      }

      // Nếu mảng không tồn tại, cần tạo mới
      if (!nodeArray) {
        // Tạo cấu trúc cho node nếu chưa có
        doc["fields"][nodeName]["arrayValue"] = JsonObject(); // Tạo một đối tượng JSON mới cho mảng
        nodeArray = doc["fields"][nodeName]["arrayValue"]["values"].createNestedArray(); // Tạo một mảng mới
      }

      // Thêm đối tượng mới vào mảng tương ứng
      if (nodeArray) {
        nodeArray.add(newNode);
      }

      // Chuyển đổi mảng đã cập nhật thành chuỗi JSON
      String updatedJson;
      serializeJson(doc, updatedJson);

      // Gửi dữ liệu đã cập nhật lên Firestore
      String firestorePushUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) +
                                 "/databases/(default)/documents/data/vFLsuo1WbrY1h6eFIXMb?key=" + String(API_KEY);
      http.begin(firestorePushUrl);
      http.addHeader("Content-Type", "application/json");
      httpResponseCode = http.PATCH(updatedJson); // Gửi yêu cầu PATCH để cập nhật dữ liệu

      if (httpResponseCode > 0) {
        Serial.println("Đã thêm dữ liệu mới thành công.");
      } else {
        Serial.println("Lỗi khi gửi dữ liệu: " + String(httpResponseCode));
      }
    } else {
      Serial.println("Lỗi khi lấy dữ liệu hiện tại: " + String(httpResponseCode));
    }

    http.end(); // Kết thúc HTTP request
  } else {
    Serial.println("Wi-Fi chưa kết nối.");
  }
}
//==============================================================================

// Hàm lấy ngày hiện tại (giữ một định nghĩa duy nhất)
String getCurrentDate() {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%d-%m-%Y", &tstruct);
  return String(buf);
}
//==============================================================================

// Trong hàm onReceive, sau khi nhận dữ liệu từ LoRa
void onReceive(int packetSize) {
  if (packetSize == 0)
    return;  // if there's no packet, return

  int recipient = LoRa.read();        // recipient address
  byte sender = LoRa.read();          // sender address
  byte incomingLength = LoRa.read();  // incoming msg length
  Incoming = "";  // Clears Incoming variable data

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

  // Phân tích dữ liệu từ Incoming
  if (sender == Destination_ESP32_Slave_1) {
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &temp1, &humi1, &moil1, &vol1);
    
    // Gửi dữ liệu lên Firestore
    Serial.println("Gui node 1");
    sendDataToFirestore("node1", temp1, humi1, moil1, vol1);
  
  } else if (sender == Destination_ESP32_Slave_2) {
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &temp2, &humi2, &moil2, &vol2);
    
    // Gửi dữ liệu lên Firestore
    Serial.println("Gui node 2");
    sendDataToFirestore("node2", temp2, humi2, moil2, vol2);
  }
}
//==============================================================================





void readMasterFieldFromFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
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

      // Lấy giá trị của trường "master"
      JsonArray masterArray = doc["fields"]["master"]["arrayValue"]["values"];

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
  } else {
    Serial.println("Wi-Fi chưa kết nối.");
  }
}
//==============================================================================

// Hàm đọc trạng thái từ Firestore
void readDeviceStateFromFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(firestoreUrl);  // URL Firestore đã được khai báo trước
    int httpResponseCode = http.GET();  // Gửi yêu cầu GET

    if (httpResponseCode > 0) {
      String payload = http.getString();  // Nhận phản hồi từ Firestore
      //Serial.println("Dữ liệu nhận được từ Firestore:");
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
      Serial.print("Control : " + String(controlValue));

      // Cập nhật trạng thái thiết bị dựa trên giá trị "control"
      if (controlValue == 1) {
        deviceState = true;
        Serial.println(" - BẬT");
      } else if (controlValue == 0) {
        deviceState = false;
        Serial.println(" - TẮT");
      }
      // Điều khiển thiết bị theo trạng thái mới
      //controlDevice(deviceState);
    } 
    else {
      Serial.println("Không thể lấy dữ liệu từ Firestore, mã lỗi2: " + String(httpResponseCode));
    }
    http.end();  // Kết thúc yêu cầu HTTP
  } 
  else {
    Serial.println("Wi-Fi chưa kết nối.");
  }
}
//==============================================================================

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