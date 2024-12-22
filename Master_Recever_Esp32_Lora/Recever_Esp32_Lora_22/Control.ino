
// Hàm kiểm tra và điều khiển tưới nước dựa trên nhiệt độ và độ ẩm đất
bool controlWatering(float temp, float humi, float moil) {
  bool wateringState = false; // Mặc định không tưới

  // Điều kiện tưới nước dựa trên ngưỡng nhiệt độ và độ ẩm
  if (temp > 30.0 && moil < 40.0) {
    // Nếu nhiệt độ > 30 độ C và độ ẩm đất < 40%, bật tưới
    wateringState = true;
    Serial.println("Bật tưới: Nhiệt độ cao và đất khô.");
  } 
  else if (humi > 95.0 || moil > 80.0) {
    // Nếu độ ẩm > 95% hoặc độ ẩm đất > 80%, tắt tưới
    wateringState = false;
    Serial.println("Tắt tưới: Độ ẩm cao.");
  } 
  else {
    Serial.println("Không cần tưới.");
  }
  return wateringState;
}


/////
// Hàm điều khiển servo và relay
void controlServoAndRelay(String nodeName, bool state) {
  int servoAngle = 0;

  if (state) {
    if (nodeName == "node1") {
      servoAngle = 90; // Nếu node1 cần tưới, quay servo 90 độ
    } 
    else if (nodeName == "node2") {
      servoAngle = 70; // Nếu node2 cần tưới, quay servo 70 độ
    }

    // Điều khiển servo
    Serial.print("Quay servo cho ");
    Serial.print(nodeName);
    Serial.print(" tới góc: ");
    Serial.println(servoAngle);
  
    myServo.write(servoAngle); // Quay servo tới góc tương ứng
    delay(3000);  // Cho servo thời gian để quay hoàn thành

    // Bật relay
    Serial.println("Bật relay");
    digitalWrite(relayPin, HIGH);
    delay(5000);  // Giữ relay bật trong 5 giây
    digitalWrite(relayPin, LOW);  // Tắt relay
  } 
  else {
    servoAngle = 0; // Nếu không cần tưới, quay về 0 độ
    Serial.print("Không cần tưới, quay servo cho ");
    Serial.print(nodeName);
    Serial.println(" về góc 0");
    delay(3000); //đợi relay tắt mới quay về
    myServo.write(servoAngle); // Quay servo về góc 0
  }
}

// Hàm đọc dữ liệu và điều khiển
void checkAndControlAtInterval() {
  unsigned long currentMillis = millis();
  
  // Kiểm tra nếu đã qua 30s kể từ lần đọc cuối
  if (currentMillis - lastReadTime >= interval) {
    lastReadTime = currentMillis;  // Cập nhật thời gian lần đọc cuối cùng

    // Kiểm tra kết nối Wi-Fi
    checkWiFiConnection();
    // Sau khi kiểm tra, nếu vẫn chưa kết nối Wi-Fi thì thoát ra
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Không có kết nối Wi-Fi, dừng các tác vụ yêu cầu kết nối.");
      return;
    }
    
    // Giả sử bạn đọc được trạng thái từ Firebase hoặc nguồn khác
    bool node1State = getFirebaseState("node1");  // Lấy trạng thái node1 từ Firebase
    if (node1State) {
      Serial.println("Node 1 đang tưới");
    } else {
      Serial.println("Node 1 không tưới");
    }

    bool node2State = getFirebaseState("node2");  // Lấy trạng thái node2 từ Firebase
    if (node2State) {
      Serial.println("Node 2 đang tưới");
    } else {
      Serial.println("Node 2 không tưới");
    }
    
    // Điều khiển servo và relay cho từng node
    controlServoAndRelay("node1", node1State);
    controlServoAndRelay("node2", node2State);
  }
}

