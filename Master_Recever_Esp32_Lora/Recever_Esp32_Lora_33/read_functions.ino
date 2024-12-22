//==============================================================================

// Hàm đọc trạng thái bat tat he thong từ Firestore
void readDeviceOnOff() {
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
      } 
      else if (controlValue == 0) {
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
// Hàm đọc trạng thái manual/control từ Firestore
void readManualAuto(String nodeName) {
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

      // Lấy giá trị của trường "mode"
      int controlValue = doc["fields"][nodeName]["integerValue"];
      Serial.print("Mode : " + String(controlValue));
      // Cập nhật trạng thái thiết bị dựa trên giá trị "control"
      if (controlValue == 1) {
        mode = true;
        Serial.println(" - Auto");
      } else if (controlValue == 0) {
        mode = false;
        Serial.println(" - Manual");
      }
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

void readNodeState(String nodeName) {
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

      // Lấy giá trị của trường "mode"
      int controlValue = doc["fields"][nodeName]["integerValue"];
      //Serial.print("statenode : " + String(controlValue));

      // Cập nhật trạng thái thiết bị dựa trên giá trị "control"
      if(nodeName == "node1State")
      {
        if (controlValue == 1) {
          stateNode1 = true;
          //Serial.println(" - Node 1 Manual: Bat");
          //delay(3000);
        } 
        else if (controlValue == 0) {
          stateNode1 = false;
          //Serial.println(" - Node 1 Manual: Tat");
          //delay(3000);
        }
      }
      else if(nodeName == "node2State")
      {
        if (controlValue == 1) {
          stateNode2 = true;
          //Serial.println(" - Node 2 Manual: Bat");
          //delay(2000);
        } 
        else if (controlValue == 0) {
          stateNode2 = false;
          //Serial.println(" - Node 2 Manual: tat");
          //delay(2000);
        }
      }
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
// Hàm lấy trạng thái từ cac Node Firebase trong truong data
int getStateNode(String nodeName) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi chưa kết nối, trả về 0.");
    return 0;  // Trả về 0 nếu Wi-Fi chưa kết nối (mặc định tắt)
  }
  HTTPClient http;
  // URL để lấy tài liệu từ Firestore
  String firestoreGetUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) +
                            "/databases/(default)/documents/data/vFLsuo1WbrY1h6eFIXMb?key=" + String(API_KEY);
  http.begin(firestoreGetUrl);
  int httpResponseCode = http.GET();  // Gửi yêu cầu GET

  if (httpResponseCode <= 0) {
    Serial.printf("Lỗi khi gửi yêu cầu: %s\n", http.errorToString(httpResponseCode).c_str());
    http.end();
    return 0;  // Trả về 0 nếu có lỗi khi lấy dữ liệu từ Firestore
  }

  // Nhận phản hồi từ Firestore
  String payload = http.getString();

  // Phân tích JSON
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.printf("Lỗi phân tích JSON: %s\n", error.c_str());
    http.end();
    return 0;  // Trả về 0 nếu có lỗi phân tích JSON
  }

  // Lấy trạng thái "state" từ JSON
  int state = 0;  // Mặc định là tắt nếu không có dữ liệu
  JsonArray nodeArray;

  if (nodeName == "node1") {
    nodeArray = doc["fields"]["node1"]["arrayValue"]["values"];
  } 
  else if (nodeName == "node2") {
    nodeArray = doc["fields"]["node2"]["arrayValue"]["values"];
  } else {
    Serial.println("Tên node không hợp lệ.");
    http.end();
    return 0;
  }

  if (nodeArray.size() > 0) {
    // Lấy giá trị "state" của phần tử cuối cùng trong mảng
    state = nodeArray[nodeArray.size() - 1]["mapValue"]["fields"]["state"]["stringValue"].as<String>().toInt();
    //Serial.println(nodeName + " state: " + String(state));
  } 
  else {
    Serial.println("Mảng node rỗng.");
  }

  http.end();  // Kết thúc HTTP request

  return (state == 1 || state == 0) ? state : 0;
}

//==============================================================================
// Hàm đọc thời gian được set để tưới định kỳ
bool readTime(String nodeName, float &first, float &second, float &third) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;  // Trả về false nếu Wi-Fi chưa kết nối
  }
  HTTPClient http;
  String firestoreGetUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) +
                           "/databases/(default)/documents/system/2Guvl9gNbqPIgiKqU2v1?key=" + String(API_KEY);
  http.begin(firestoreGetUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode <= 0) {
    http.end();
    return false;  // Trả về false nếu có lỗi khi lấy dữ liệu từ Firestore
  }
  String payload = http.getString();
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    http.end();
    return false;  // Trả về false nếu có lỗi phân tích JSON
  }

  // Kiểm tra xem nodeName có hợp lệ và có dữ liệu không
  if (nodeName == "timer" && doc["fields"]["timer"]["mapValue"]["fields"]) {
    // Kiểm tra sự tồn tại của các trường trước khi truy cập
    if (doc["fields"]["timer"]["mapValue"]["fields"]["first"]["stringValue"].isNull() ||
        doc["fields"]["timer"]["mapValue"]["fields"]["second"]["stringValue"].isNull() ||
        doc["fields"]["timer"]["mapValue"]["fields"]["third"]["stringValue"].isNull()) {
      Serial.println("Một trong các trường thời gian không tồn tại hoặc không có dữ liệu.");
      http.end();
      return false;  // Trả về false nếu không có dữ liệu hợp lệ
    }

    // Đọc chuỗi thời gian và chuyển đổi thành float
    first = convertToFloat(doc["fields"]["timer"]["mapValue"]["fields"]["first"]["stringValue"].as<String>());
    second = convertToFloat(doc["fields"]["timer"]["mapValue"]["fields"]["second"]["stringValue"].as<String>());
    third = convertToFloat(doc["fields"]["timer"]["mapValue"]["fields"]["third"]["stringValue"].as<String>());

    //Serial.println("First: " + String(first));
    //Serial.println("Second: " + String(second));
    //Serial.println("Third: " + String(third));

    http.end();
    return true;  // Dữ liệu hợp lệ
  }
  
  Serial.println("Map timer không tồn tại hoặc không có dữ liệu.");
  http.end();
  return false;  // Trả về false nếu không có dữ liệu hợp lệ
}

// Hàm chuyển chuỗi giờ phút thành float
float convertToFloat(String timeStr) {
  int colonIndex = timeStr.indexOf(':');  // Tìm vị trí dấu hai chấm
  if (colonIndex == -1) return 0;  // Trả về 0 nếu không có dấu hai chấm
  
  int hours = timeStr.substring(0, colonIndex).toInt();
  int minutes = timeStr.substring(colonIndex + 1).toInt();
  
  return hours + (minutes / 60.0);  // Trả về giá trị float của giờ
}






