#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>  // Thư viện EEPROM
#include <WiFiUdp.h>
#include <NTPClient.h>

// Thông tin WiFi
#define WIFI_SSID "Hung"
#define WIFI_PASSWORD "a123456789"

// Thông tin Firebase Firestore
#define API_KEY "AIzaSyD33F_6X-rslRLf7Am8DLsVmS4lferLwt0"
#define PROJECT_ID "test-c7ac1"
String firestoreUrl = "https://firestore.googleapis.com/v1/projects/" + String(PROJECT_ID) + "/databases/(default)/documents/analytics_sub/analytics_sub?key=" + String(API_KEY);

// Cấu hình NTPClient cho múi giờ Việt Nam (UTC+7)
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 25200); 

// Cấu hình cảm biến lưu lượng nước
const int flowPin = 4;       
const float flowFactor = 7.5; 
volatile int pulseCount = 0;  

unsigned long previousMillisSend = 0;  
const long intervalSend = 10000;       

#define EEPROM_SIZE 128  // Kích thước bộ nhớ EEPROM

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Khởi tạo EEPROM
  EEPROM.begin(EEPROM_SIZE);

  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Đã kết nối!");

  timeClient.begin();
  timeClient.setTimeOffset(25200);

  pinMode(flowPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowPin), pulseCounter, RISING);

  Serial.println("Hệ thống đã sẵn sàng!");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisSend >= intervalSend) {
    previousMillisSend = currentMillis;

    // Cập nhật thời gian từ NTP server
      timeClient.update();
      // Lấy thời gian định dạng
      String currentTime = timeClient.getFormattedTime();
      // Lấy thời gian Unix Timestamp
      unsigned long epochTime = timeClient.getEpochTime(); 
      // Chuyển đổi epoch time thành ngày tháng năm
      String currentDay = getFormattedDateTime(epochTime);
      //Serial.println("Thời gian hiện tại: " + dateTime);

    //String dateTime = getFormattedDateTime(timeClient.getEpochTime());
    saveDateTimeToEEPROM(currentDay);

    sendConsumedToFirestore();
  }
}

// Hàm lưu `datetime` vào EEPROM
void saveDateTimeToEEPROM(String dateTime) {
  int len = dateTime.length();
  EEPROM.write(0, len);  
  for (int i = 0; i < len; i++) {
    EEPROM.write(i + 1, dateTime[i]);
  }
  EEPROM.commit();  
  Serial.println("Đã lưu thời gian vào EEPROM: " + dateTime);
}

// Hàm đọc `datetime` từ EEPROM
String readDateTimeFromEEPROM() {
  int len = EEPROM.read(0);  
  char buffer[len + 1];
  for (int i = 0; i < len; i++) {
    buffer[i] = EEPROM.read(i + 1);
  }
  buffer[len] = '\0';  
  return String(buffer);
}

// Hàm gửi dữ liệu từ EEPROM lên Firestore
void sendConsumedToFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String firestoreSetUrl = firestoreUrl;
    http.begin(firestoreSetUrl);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(8192);
    JsonObject fields = doc.createNestedObject("fields");

    // Đọc thời gian từ EEPROM
    String dateTime = readDateTimeFromEEPROM();

    int vol = 100; 

    JsonObject daily = fields.createNestedObject("daily");
    JsonObject dailyArrayValue = daily.createNestedObject("arrayValue");
    JsonArray dailyValuesArray = dailyArrayValue.createNestedArray("values");

    JsonObject dayData = dailyValuesArray.createNestedObject();
    JsonObject mapValue = dayData.createNestedObject("mapValue");
    JsonObject mapFields = mapValue.createNestedObject("fields");

    JsonObject dayField = mapFields.createNestedObject("day");
    dayField["stringValue"] = dateTime;

    JsonObject volField = mapFields.createNestedObject("vol");
    volField["integerValue"] = vol;
    
    String jsonString;
    serializeJson(doc, jsonString);
    int httpResponseCode = http.PATCH(jsonString);

    if (httpResponseCode == 200) {
      Serial.println("Cập nhật dữ liệu lên Firestore thành công!");
    } else {
      Serial.print("Lỗi cập nhật Firestore: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi không kết nối");
  }
}

// Hàm chuyển đổi epoch time thành định dạng ngày tháng năm
String getFormattedDateTime(unsigned long epochTime) {
  struct tm *ptm = gmtime((time_t *)&epochTime);
  char buffer[30];
  sprintf(buffer, "%02d-%02d-%04d",
          ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year + 1900);
  return String(buffer);
}
