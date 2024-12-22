//==============================================================================
// Hàm kiểm tra và điều khiển tưới nước dựa trên nhiệt độ và độ ẩm đất - tra ve gia tri true/false de gui len state
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

//==============================================================================
bool isControlling = false; // Biến cờ để theo dõi trạng thái điều khiển
// Hàm điều khiển servo và relay
void controlServoAndRelay(String nodeName, bool state) {
  if (isControlling) {
    Serial.println("Đang điều khiển servo và relay. Vui lòng đợi.");
    return; // Nếu đang điều khiển, thoát hàm
  }
  isControlling = true; // Đánh dấu là đang điều khiển
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
    //delay(3000);  // Cho servo thời gian để quay hoàn thành
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải

    // Bật relay
    Serial.println("Bật RELAY");
    digitalWrite(relayPin, HIGH);
    //delay(5000);  // Giữ relay bật trong 5 giây
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
    digitalWrite(relayPin, LOW);  // Tắt relay
  } 
  else {
    // Nếu không cần tưới, thông báo không tưới
    Serial.print("Không cần tưới cho ");
    Serial.println(nodeName);
  }

  // Quay servo về góc 0
  Serial.print("Quay servo cho ");
  Serial.print(nodeName);
  Serial.println(" về góc 0");
  myServo.write(0); // Quay servo về góc 0
  //delay(2000); // Thời gian cho servo về góc 0
  vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
  isControlling = false; // Đánh dấu là không còn điều khiển
}
// Hàm đọc dữ liệu và điều khiển servo và relay  ////////
void checkAndControlAtInterval() {
  unsigned long currentMillis = millis();

  // Kiểm tra kết nối Wi-Fi
  checkWiFiConnection();
  // Sau khi kiểm tra, nếu vẫn chưa kết nối Wi-Fi thì thoát ra
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Không có kết nối Wi-Fi, dừng các tác vụ yêu cầu kết nối.");
    return;
  }

  // Giả sử bạn đọc được trạng thái từ Firebase hoặc nguồn khác
  bool node1State = getStateNode("node1");  // Lấy trạng thái node1 từ Firebase  /====/
  bool node2State = getStateNode("node2");  // Lấy trạng thái node2 từ Firebase

  // Thực hiện tưới cho từng node nếu cần
  if (node1State) {
    controlServoAndRelay("node1", true);
  }
  
  if (node2State) {
    controlServoAndRelay("node2", true);
  }
}

//==============================================================================
// Hàm điều khiển servo và relay dựa trên lượng nước set
void controlWaterAmount(String nodeName, bool state) {
  if (isControlling) {
    Serial.println("Đang điều khiển servo và relay. Vui lòng đợi.");
    return; // Nếu đang điều khiển, thoát hàm
  }
  isControlling = true; // Đánh dấu là đang điều khiển
  int servoAngle = 0;

  if (state) {
    if (nodeName == "node1") {
      servoAngle = 180; // Nếu node1 cần tưới, quay servo 90 độ
    } 
    else if (nodeName == "node2") {
      servoAngle = 100; // Nếu node2 cần tưới, quay servo 70 độ
    }

    // Điều khiển servo
    Serial.print("Quay servo cho ");
    Serial.print(nodeName);
    Serial.print(" tới góc: ");
    Serial.println(servoAngle);
  
    myServo.write(servoAngle); // Quay servo tới góc tương ứng
    //delay(3000);  // Cho servo thời gian để quay hoàn thành
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải

    // Bật relay
    Serial.println("Bật RELAY");
    digitalWrite(relayPin, HIGH);
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
    digitalWrite(relayPin, LOW);  // Tắt relay
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
    digitalWrite(relayPin, HIGH);
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
    digitalWrite(relayPin, LOW);  // Tắt relay
  } 
  else {
    // Nếu không cần tưới, thông báo không tưới
    Serial.print("Không cần tưới cho ");
    Serial.println(nodeName);
  }
  // Quay servo về góc 0
  Serial.print("Quay servo cho ");
  Serial.print(nodeName);
  Serial.println(" về góc 0");
  myServo.write(0); // Quay servo về góc 0
  //delay(2000); // Thời gian cho servo về góc 0
  vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay 5 giây để giảm tải
  isControlling = false; // Đánh dấu là không còn điều khiển
}
//==============================================================================
// Hàm điều khiển tất cả các node tưới theo thời gian set
void controlAllNodesTime() {
  float firstTime, secondTime, thirdTime; // Các biến lưu thời gian cần tưới cho các mốc thời gian
  String nodeNames[] = {"node1", "node2"}; // Danh sách tên các node

  // Kiểm tra nếu đọc thời gian từ Firebase thành công
  if (readTime("timer", firstTime, secondTime, thirdTime))   // === Dùng hàm readTime để đọc thời gian set
  {
    ntp.update(); // Cập nhật thời gian từ NTP
    
    float currentTime = ntp.getHours() + ntp.getMinutes() / 60.0; // Lấy thời gian hiện tại từ NTP dưới dạng float
    //Serial.print("Thời gian lấy từ NTP: ");
    //Serial.println(currentTime);
    // Duyệt qua tất cả các node
    for (String nodeName : nodeNames) {
      // Kiểm tra thời gian tưới cho từng node
      bool isWatering = false; // Biến kiểm tra có tưới hay không cho từng node

      if (abs(currentTime - firstTime) < 0.01 || 
          abs(currentTime - secondTime) < 0.01 || 
          abs(currentTime - thirdTime) < 0.01) {
        Serial.println("Đang tưới cho " + nodeName + " tại mốc thời gian.");
        //controlServoAndRelay(nodeName, true);
        controlWaterAmount(nodeName, true);  
        isWatering = true; // Đánh dấu là đang tưới
      }
      if (!isWatering) {
        // Nếu không trùng thời gian tưới nào, tắt tưới cho node đó
        //Serial.println("Không phải thời gian tưới cho " + nodeName + ", dừng tưới.");
        controlWaterAmount(nodeName, false);
      }
    }
  } else {
    Serial.println("Không thể đọc thời gian từ Firebase.");
  }
}

// van tai su dung duoc ham controlservoandrelay ma ko so xung dot, vi che do auto/manual
//==============================================================================
// Hàm điều khiển servo và relay tuoi cho tung node che do manual
void controlNodeManuel(String nodeName) {
  // Đọc trạng thái của node
  readNodeState(nodeName);

  // Kiểm tra node1State hoặc node2State và điều khiển tương ứng
  if (nodeName == "node1State" && stateNode1) {
    Serial.println("ĐK Node 1 thủ công");
    controlServoAndRelay("node1", true);  // Điều khiển cho node1 bật
  } 
  else if (nodeName == "node2State" && stateNode2) {
    Serial.println("ĐK Node 1 thủ công");
    controlServoAndRelay("node2", true);  // Điều khiển cho node2 bật
  } 
  else {
    //Serial.println("Node không ở chế độ bật thủ công.");
    controlServoAndRelay(nodeName, false);  // Đặt về trạng thái tắt nếu không cần tưới
  }
}




