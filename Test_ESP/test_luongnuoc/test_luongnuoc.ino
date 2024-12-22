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

// Khai báo NTPClient
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 25200); // 25200 giây cho múi giờ UTC+7 (Việt Nam)

// Cấu hình cảm biến lưu lượng nước
const int flowPin = 4;       // Pin nhận xung từ cảm biến YF-S401
const float flowFactor = 7.5; // YF-S401: 7.5 xung mỗi lít
volatile int pulseCount = 0;  // Biến đếm số xung

float consumedMonth[12]; // Mảng lưu lượng nước cho 12 tháng
const int EEPROM_SIZE = sizeof(consumedMonth); // Kích thước cần cho mảng

unsigned long previousMillisSend = 0;  // Thời gian trước đó khi gửi dữ liệu lên Firestore
const long intervalSend = 10000;       // Khoảng thời gian 1 phút cho việc gửi

// Hàm xử lý ngắt cho cảm biến lưu lượng nước
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// Hàm khởi tạo
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Khởi tạo EEPROM với kích thước đủ để lưu mảng
  EEPROM.begin(EEPROM_SIZE);

  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Đã kết nối!");

  // Khởi động NTPClient
  timeClient.begin();
  timeClient.update(); // Cập nhật thời gian lần đầu tiên

  // Đọc dữ liệu từ Firestore một lần duy nhất
  readFirestoreData();

  // Đọc dữ liệu đã lưu từ EEPROM
  for (int i = 0; i < 12; i++) {
    EEPROM.get(i * sizeof(float), consumedMonth[i]);
  }

  // Kiểm tra dữ liệu hợp lệ, nếu không gán giá trị mặc định
  for (int i = 0; i < 12; i++) {
    if (isnan(consumedMonth[i])) {
      consumedMonth[i] = 0.0;
    }
  }

  // Thiết lập ngắt cho cảm biến lưu lượng nước
  pinMode(flowPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowPin), pulseCounter, RISING);
  Serial.println("Hệ thống đã sẵn sàng!");
}

void loop() {
  // Tính lưu lượng nước
  updateWaterFlow();

  unsigned long currentMillis = millis();

  // Kiểm tra thời gian gửi dữ liệu lên Firestore
  if (currentMillis - previousMillisSend >= intervalSend) {
    previousMillisSend = currentMillis;

    // Lưu dữ liệu vào EEPROM
    saveConsumedToEEPROM();

    // Gửi dữ liệu lên Firestore
    sendConsumedToFirestore();
  }
}

// Hàm tính lưu lượng nước và cập nhật mảng theo tháng
void updateWaterFlow() {
  static float totalWaterUsage = 0.0;
  static unsigned long previousMillis = 0; // Biến lưu thời gian trước đó
  const long interval = 5000;              // Khoảng thời gian 5 giây

  float flowRate = (pulseCount / flowFactor); // Lượng nước (lít)
  pulseCount = 0;                             // Reset số xung

  totalWaterUsage += flowRate;

  // Lấy tháng hiện tại
  timeClient.update();
  int currentMonth = getCurrentMonth();

  // Cộng dồn vào tháng tương ứng
  if (currentMonth > 0 && currentMonth <= 12) {
    consumedMonth[currentMonth - 1] += flowRate;
  }

  // Kiểm tra nếu đã qua 5 giây
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // In ra thông tin lưu lượng nước
    Serial.print("Tháng ");
    Serial.print(currentMonth);
    Serial.print(": ");
    Serial.println(consumedMonth[currentMonth - 1], 2); // In giá trị với 2 số lẻ
  }
}

// Hàm lấy tháng hiện tại từ thời gian epoch
int getCurrentMonth() {
  unsigned long epochTime = timeClient.getEpochTime();
  time_t localTime = epochTime + 25200; // Thêm múi giờ UTC+7
  struct tm *ptm = gmtime(&localTime);
  return ptm->tm_mon + 1; // tm_mon là tháng từ 0 đến 11
}

// Hàm lưu mảng consumedMonth vào EEPROM
void saveConsumedToEEPROM() {
  for (int i = 0; i < 12; i++) {
    EEPROM.put(i * sizeof(float), consumedMonth[i]);
  }
  EEPROM.commit();  // Lưu thay đổi vào EEPROM
  Serial.println("Đã lưu dữ liệu vào EEPROM.");
}


// Hàm đọc dữ liệu từ Firestore và lưu vào EEPROM
void readFirestoreData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(firestoreUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("Đọc dữ liệu từ Firestore thành công!");

      // Xử lý JSON
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, payload);

      JsonArray consumedArray = doc["fields"]["consumed"]["arrayValue"]["values"];

      for (size_t i = 0; i < consumedArray.size(); i++) {
        String monthStr = consumedArray[i]["mapValue"]["fields"]["month"]["stringValue"];
        String volStr = consumedArray[i]["mapValue"]["fields"]["vol"]["stringValue"];

        int monthIndex = monthStr.toInt() - 1; // Chuyển chuỗi thành chỉ số tháng (0-11)
        float volume = volStr.toFloat();       // Chuyển chuỗi lượng nước thành số thực

        if (monthIndex >= 0 && monthIndex < 12) {
          consumedMonth[monthIndex] = volume;  // Gán giá trị lượng nước vào mảng
        }
      }

      // Lưu mảng vào EEPROM
      saveConsumedToEEPROM();

      // Kiểm tra dữ liệu sau khi gán và lưu
      for (int i = 0; i < 12; i++) {
        Serial.print("Lượng nước tháng ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(consumedMonth[i]);
      }
    } 
    else {
      Serial.print("Lỗi đọc dữ liệu từ Firestore: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } 
  else {
    Serial.println("WiFi không kết nối");
  }
}


// Hàm gửi dữ liệu từ mảng lên Firestore
void sendConsumedToFirestore() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // URL Firestore
    String firestoreSetUrl = firestoreUrl;
    http.begin(firestoreSetUrl);
    http.addHeader("Content-Type", "application/json");

    // Tạo JSON để gửi
    DynamicJsonDocument doc(4096);
    JsonObject fields = doc.createNestedObject("fields");
    JsonObject consumed = fields.createNestedObject("consumed");
    JsonObject arrayValue = consumed.createNestedObject("arrayValue");
    JsonArray valuesArray = arrayValue.createNestedArray("values");

    for (int i = 0; i < 12; i++) {
      JsonObject monthData = valuesArray.createNestedObject();
      JsonObject mapValue = monthData.createNestedObject("mapValue");
      JsonObject mapFields = mapValue.createNestedObject("fields");

      JsonObject monthField = mapFields.createNestedObject("month");
      monthField["stringValue"] = String(i + 1);

      JsonObject volField = mapFields.createNestedObject("vol");
      volField["stringValue"] = String(consumedMonth[i]);
    }

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
  } 
  else {
    Serial.println("WiFi không kết nối");
  }
}
