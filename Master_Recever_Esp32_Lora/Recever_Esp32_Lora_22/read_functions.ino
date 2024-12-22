
// Hàm đọc dư liệu trường Master
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

/////////
// Hàm lấy trạng thái từ Firebase
int getFirebaseState(String nodeName) {
  if (WiFi.status() != WL_CONNECTED) {
    return 0;  // Trả về 0 nếu Wi-Fi chưa kết nối (mặc định tắt)
  }

  HTTPClient http;
  
  // URL để lấy tài liệu từ Firestore
  String firestoreGetUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) +
                            "/databases/(default)/documents/data/vFLsuo1WbrY1h6eFIXMb?key=" + String(API_KEY);
  http.begin(firestoreGetUrl);
  int httpResponseCode = http.GET();  // Gửi yêu cầu GET

  if (httpResponseCode <= 0) {
    http.end();
    return 0;  // Trả về 0 nếu có lỗi khi lấy dữ liệu từ Firestore (mặc định tắt)
  }

  // Nhận phản hồi từ Firestore
  String payload = http.getString();

  // Phân tích JSON
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    http.end();
    return 0;  // Trả về 0 nếu có lỗi phân tích JSON (mặc định tắt)
  }

  // Lấy trạng thái "state" từ JSON
  int state = 0;  // Mặc định là tắt nếu không có dữ liệu
  if (nodeName == "node1") {
    // Kiểm tra mảng node1 có tồn tại và không rỗng
    JsonArray nodeArray = doc["fields"]["node1"]["arrayValue"]["values"];
    if (nodeArray.size() > 0) {
      // Lấy giá trị "state" của phần tử cuối cùng trong mảng node1
      state = nodeArray[nodeArray.size() - 1]["mapValue"]["fields"]["state"]["stringValue"].as<String>().toInt();
      Serial.println("NODE1 state: " + String(state));
      delay(1000);
    } 
  } 
  else if (nodeName == "node2") {
    // Kiểm tra mảng node2 có tồn tại và không rỗng
    JsonArray nodeArray = doc["fields"]["node2"]["arrayValue"]["values"];
    if (nodeArray.size() > 0) {
      // Lấy giá trị "state" của phần tử cuối cùng trong mảng node2
      state = nodeArray[nodeArray.size() - 1]["mapValue"]["fields"]["state"]["stringValue"].as<String>().toInt();
      Serial.println("NODE2 state: " + String(state));
      delay(1000);
    }
  }

  http.end();  // Kết thúc HTTP request

  // Trả về giá trị 1 hoặc 0 nếu hợp lệ, mặc định trả về 0 nếu không hợp lệ
  return (state == 1 || state == 0) ? state : 0;
}






