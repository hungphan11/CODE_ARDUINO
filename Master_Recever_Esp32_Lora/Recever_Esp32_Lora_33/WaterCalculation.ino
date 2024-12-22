//===============================================================
// Hàm cập nhật biến currentDayStr theo định dạng dd-mm-yyyy
void updateCurrentDayString() {
    // Cập nhật thời gian từ NTP server
      ntp.update();
      // Lấy thời gian Unix Timestamp
      unsigned long epochTime = ntp.getEpochTime(); 
      // Chuyển đổi epoch time thành ngày tháng năm
      String dateTime = getFormattedDateTime_1(epochTime);
      //Serial.println("Thời gian hiện tại: " + dateTime);
      // Cập nhật currentDayStr theo định dạng dd-mm-yyyy
      currentDayStr = String(dateTime);
}
// Cập nhật lưu lượng nước trong ngày
void updateDailyFlow(int currentMonth) {
  float flowRate = (pulseCount / flowFactor); // Kiểm tra nếu flowFactor không phải 0
  pulseCount = 0;
  
  if (flowFactor != 0) {
    dailyFlow += flowRate;
    consumedMonth[currentMonth - 1] += flowRate;
  } 
  else {
    Serial.println("Giá trị flowFactor bằng 0, không thể tính toán lưu lượng!");
  }

  // Kiểm tra nếu đã qua 5 giây
  const long interval = 5000; // Khoảng thời gian 5 giây
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // In ra thông tin lưu lượng nước theo tháng và ngày
    Serial.print("Lượng nước tháng ");
    Serial.print(currentMonth);
    Serial.print(": ");
    Serial.println(consumedMonth[currentMonth - 1], 2); // In giá trị với 2 chữ số thập phân

    Serial.print("Lượng nước ngày ");
    Serial.print(": ");
    Serial.println(dailyFlow, 2); // In giá trị với 2 chữ số thập phân
  }
}
// Hàm reset lượng nước trong ngày về 0
void resetDailyFlow() {
  dailyFlow = 0;
  Serial.println("Lượng nước trong ngày đã được reset về 0.");
}

//======================== HÀM LƯU DỮ LIỆU =======================================
// Hàm lưu ngày hiện tại vào EEPROM
void saveDayToEEPROM() {
  char currentDayChar[11];
  currentDayStr.toCharArray(currentDayChar, 11);
  for (int i = 0; i < 11; i++) {
    EEPROM.write(DAY_ADDRESS + i, currentDayChar[i]);
  }
  EEPROM.commit();  // Lưu thay đổi vào EEPROM
  Serial.println("Ngày đã được lưu vào EEPROM!");
}

// Lưu dữ liệu vào EEPROM
void saveDataToEEPROM() {
  // Lưu lượng nước ngày vào EEPROM
  EEPROM.put(DAILY_FLOW_ADDRESS, dailyFlow);

  // Lưu ngày hiện tại vào EEPROM
  char currentDayChar[11];
  currentDayStr.toCharArray(currentDayChar, 11);
  for (int i = 0; i < 11; i++) {
    EEPROM.write(DAY_ADDRESS + i, currentDayChar[i]);
  }

  EEPROM.commit();
  Serial.println("Dữ liệu đã được lưu vào EEPROM:");
}

//======================== HÀM ĐỌC DỮ LIỆU =======================================
// Hàm lấy tháng từ thời gian epoch
int getCurrentMonth(unsigned long epochTime) {
  time_t localTime = epochTime + 25200; // Giờ Việt Nam (UTC+7)
  struct tm *ptm = localtime(&localTime); // Sử dụng localtime thay vì gmtime
  return ptm->tm_mon + 1; // Tháng bắt đầu từ 0, nên cần cộng thêm 1
}
// Hàm để chuyển đổi epoch time thành định dạng ngày tháng năm
String getFormattedDateTime_1(unsigned long epochTime) {
  // Không cần cộng thêm 7 giờ nữa vì NTPClient đã điều chỉnh
  time_t localTime = epochTime;
  struct tm *ptm = localtime(&localTime); // Sử dụng localtime thay vì gmtime

  // Định dạng thành chuỗi ngày tháng năm
  char buffer[30];
  sprintf(buffer, "%02d-%02d-%04d",
          ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year + 1900);
  return String(buffer);
}

// Hàm đọc dữ liệu từ eeprom
void readDataFromEEPROM() {
  // Đọc lưu lượng nước ngày từ EEPROM
  EEPROM.get(DAILY_FLOW_ADDRESS, dailyFlow);

  // Đọc lưu lượng nước tháng từ EEPROM
  for (int i = 0; i < 12; i++) {
    EEPROM.get(MONTHLY_FLOW_START + i * sizeof(float), consumedMonth[i]);
  }
  // Đọc ngày hiện tại từ EEPROM
  char currentDayChar[11];  // Để lưu ngày theo định dạng "dd-mm-yyyy"
  for (int i = 0; i < 11; i++) {
    currentDayChar[i] = EEPROM.read(DAY_ADDRESS + i);
  }
  currentDayStr = String(currentDayChar);

  // In ra các dữ liệu đã đọc
  Serial.println("Dữ liệu đã được đọc từ EEPROM:");
  Serial.printf("Lưu lượng ngày: %.2f L\n", dailyFlow);
  Serial.println("Ngày hiện tại: " + currentDayStr);
}

// Hàm đọc lưu lượng nước ngày từ EEPROM
float readDailyFlowFromEEPROM() {
  return EEPROM.readFloat(DAILY_FLOW_ADDRESS);
}
// Hàm đọc ngày lưu trong EEPROM
String getStoredDayFromEEPROM_1() {
  char storedDay[11];
  for (int i = 0; i < 10; i++) {
    storedDay[i] = EEPROM.read(DAY_ADDRESS + i);
  }
  storedDay[10] = '\0'; // Kết thúc chuỗi
  return String(storedDay);
}
// Hàm kiểm tra ngày hiện tại lưu trong EEPROM
String getStoredDayFromEEPROM() {
  char storedDayChar[11];  // Để lưu ngày theo định dạng "dd-mm-yyyy"
  for (int i = 0; i < 11; i++) {
    storedDayChar[i] = EEPROM.read(DAY_ADDRESS + i);
  }
  return String(storedDayChar);
}

//======================== HÀM GỬI DỮ LIỆU =======================================
// Hàm cập nhật dữ liệu mới nhất
void updateLatestMapInFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(firestoreUrlWarter);
    http.addHeader("Content-Type", "application/json");

    // Gửi yêu cầu GET để lấy dữ liệu hiện tại từ Firestore
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, payload);
      
      // Lấy các bản ghi "fields" và số lượng các bản ghi hiện có
      JsonObject fields = doc["fields"];
      JsonObject daily = fields["daily"];
      JsonObject arrayValue = daily["arrayValue"];
      JsonArray valuesArray = arrayValue["values"];
      
      // Kiểm tra số lượng maps hiện tại
      int mapCount = valuesArray.size();
      Serial.print("Số lượng bản ghi hiện tại: ");
      Serial.println(mapCount);

      // Cập nhật bản ghi mới nhất
      if (mapCount > 0) {
        // Lấy ngày và lượng nước lưu từ EEPROM
        String storedDay = getStoredDayFromEEPROM_1();
        float storedDailyFlow = readDailyFlowFromEEPROM();

        // Tạo bản ghi mới
        DynamicJsonDocument newDoc(1024);
        JsonObject newMap = newDoc.to<JsonObject>();
        JsonObject mapFields = newMap.createNestedObject("mapValue").createNestedObject("fields");

        mapFields["day"]["stringValue"] = storedDay;
        mapFields["vol"]["doubleValue"] = storedDailyFlow;

        mapFields["temperature"]["doubleValue"] = temperature;
        mapFields["humidity"]["doubleValue"] = humidity;
        mapFields["rain"]["doubleValue"] = rain;

        // Thay thế bản ghi cuối với dữ liệu mới
        valuesArray[mapCount - 1] = newMap;

        // Chuẩn bị JSON và gửi lại dữ liệu
        String jsonString;
        serializeJson(doc, jsonString);
        httpResponseCode = http.PATCH(jsonString); // Cập nhật dữ liệu với phương thức PATCH
        if (httpResponseCode == 200) {
          Serial.println("Cập nhật bản ghi mới nhất thành công!");
        } else {
          Serial.print("Lỗi khi cập nhật dữ liệu: ");
          Serial.println(httpResponseCode);
        }
      } else {
        Serial.println("Không có bản ghi nào để cập nhật.");
      }
    } else {
      Serial.print("Lỗi khi lấy dữ liệu từ Firestore: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Không kết nối được Wi-Fi, không thể gửi dữ liệu.");
  }
}
// Hàm thêm map dữ liệu mới
bool appendNewMapToFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(firestoreUrlWarter);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, payload);

      JsonObject fields = doc["fields"];
      JsonObject daily = fields["daily"];
      JsonObject arrayValue = daily["arrayValue"];
      JsonArray valuesArray = arrayValue["values"];

      // Lấy dữ liệu từ EEPROM
      String storedDay = getStoredDayFromEEPROM_1();
      float storedDailyFlow = readDailyFlowFromEEPROM();

      // Thêm dữ liệu mới
      JsonObject newDailyData = valuesArray.createNestedObject();
      JsonObject newMapValue = newDailyData.createNestedObject("mapValue");
      JsonObject newFields = newMapValue.createNestedObject("fields");

      JsonObject dayField = newFields.createNestedObject("day");
      dayField["stringValue"] = storedDay;

      JsonObject dailyFlowField = newFields.createNestedObject("vol");
      dailyFlowField["doubleValue"] = storedDailyFlow;

      JsonObject temperatureField = newFields.createNestedObject("temperature");
      temperatureField["doubleValue"] = temperature;

      JsonObject humidityField = newFields.createNestedObject("humidity");
      humidityField["doubleValue"] = humidity;

      JsonObject rainField = newFields.createNestedObject("rain");
      rainField["doubleValue"] = rain;

      String jsonString;
      serializeJson(doc, jsonString);
      httpResponseCode = http.PATCH(jsonString);
      if (httpResponseCode == 200) {
        Serial.println("Dữ liệu mới đã được thêm thành công!");
        http.end();
        return true;
      } else {
        Serial.print("Lỗi khi gửi dữ liệu: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
      }
    } else {
      Serial.print("Lỗi khi lấy dữ liệu từ Firestore: ");
      Serial.println(httpResponseCode);
      http.end();
      return false;
    }
  } else {
    Serial.println("Không kết nối được Wi-Fi, không thể gửi dữ liệu.");
    return false;
  }
}

//======================== HÀM GỬI DỮ LIỆU THỜI TIẾT ===================================
// Hàm lấy dữ liệu thời tiết từ OpenWeather
void getWeatherData() {
  HTTPClient http;

  // Gửi yêu cầu GET đến API
  http.begin(apiUrl);
  int httpCode = http.GET();

  // Kiểm tra mã phản hồi HTTP
  if (httpCode > 0) {
    Serial.printf("HTTP Code: %d\n", httpCode);
    String payload = http.getString();
    
    // Sử dụng ArduinoJson để phân tích dữ liệu JSON
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    // Biến tạm để tính trung bình
    float totalTemp = 0.0;
    float totalHumidity = 0.0;
    float totalRain = 0.0;
    int rainCount = 0;
    int forecastCount = doc["list"].size();  // Số lượng dự báo (cnt = 3)

    // Lấy dữ liệu từ các dự báo và tính trung bình
    for (int i = 0; i < forecastCount; i++) {
      JsonObject list = doc["list"][i];  // Dự báo tại thời điểm thứ i
      totalTemp += list["main"]["temp"].as<float>();  // Chuyển đổi thành float
      totalHumidity += list["main"]["humidity"].as<float>();  // Chuyển đổi thành float

      // Kiểm tra xem có mưa không và thêm vào tổng nếu có
      if (list.containsKey("rain")) {
        totalRain += list["rain"]["3h"].as<float>();  // Lượng mưa trong 3 giờ, chuyển thành float
        rainCount++;
      }
    }

    // Tính giá trị trung bình
    temperature = totalTemp / forecastCount;
    humidity = totalHumidity / forecastCount;
    rain = (rainCount > 0) ? totalRain / rainCount : 0.0;  // Nếu không có mưa, rain = 0

    // Hiển thị dữ liệu lên Serial Monitor
    Serial.print("Average Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Average Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Average Rain (last 3 hours): ");
    Serial.print(rain);
    Serial.println(" mm");
  } else {
    Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}


