
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
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &humi1, &temp1, &moil1, &vol1);

    // Kiểm tra độ ẩm và xác định trạng thái
    state1 = (humi1 > 95.0); 
    
    //state1 = controlWatering(temp1, humi1, moil1);

    // Gửi dữ liệu lên Firestore
    Serial.println("Gui node 1");
    sendDataToFirestore("node1", state1, temp1, humi1, moil1, vol1);
  
  } 
  else if (sender == Destination_ESP32_Slave_2) {
    sscanf(Incoming.c_str(), "%f:%f:%f:%f", &humi2, &temp2, &moil2, &vol2);
    
    // Kiểm tra độ ẩm và xác định trạng thái
    state2 = (humi2 > 95.0); 
    //state2 = controlWatering(temp2, humi2, moil2);

    // Gửi dữ liệu lên Firestore
    Serial.println("Gui node 2");
    sendDataToFirestore("node2", state2, temp2, humi2, moil2, vol2);
  }
}

//==============================================================================

// Hàm gửi dữ liệu từ LoRa lên Firestore
void sendDataToFirestore(String nodeName,bool state, float temp, float humi, float moil, float vol) {
  // Kiểm tra kết nối Wi-Fi
  checkWiFiConnection();
  // Sau khi kiểm tra, nếu vẫn chưa kết nối Wi-Fi thì thoát ra
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Không có kết nối Wi-Fi, dừng các tác vụ yêu cầu kết nối.");
    return;
  }

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
    newNodeFields["state"]["stringValue"] = String(state);
    newNodeFields["temp"]["stringValue"] = String(temp);
    newNodeFields["humi"]["stringValue"] = String(humi);
    newNodeFields["moil"]["stringValue"] = String(moil);
    newNodeFields["vol"]["stringValue"] = String(vol);
    newNodeFields["date"]["stringValue"] = getCurrentDate(); // Thêm ngày hiện tại

    // Lấy mảng node1 hoặc node2 hiện tại
    JsonArray nodeArray;
    if (nodeName == "node1") {
      nodeArray = doc["fields"]["node1"]["arrayValue"]["values"];
    } 
    else if (nodeName == "node2") {
      nodeArray = doc["fields"]["node2"]["arrayValue"]["values"];
    } 
    else {
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
    } 
    else {
      Serial.println("Lỗi khi gửi dữ liệu: " + String(httpResponseCode));
    }
  } 
  else {
    Serial.println("Lỗi khi lấy dữ liệu hiện tại: " + String(httpResponseCode));
  }
  http.end(); // Kết thúc HTTP request
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