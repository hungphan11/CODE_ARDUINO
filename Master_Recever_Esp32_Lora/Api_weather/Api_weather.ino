#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Cấu hình Wi-Fi
#define WIFI_SSID "Hung"
#define WIFI_PASSWORD "a123456789"

// API URL và Key của OpenWeather
const char* apiUrl = "https://api.openweathermap.org/data/2.5/forecast?q=Buon+Ma+Thuot&appid=201bd92755af93923b3854e767deeed0&lang=vi&units=metric&cnt=3";

// Khai báo các giá trị dữ liệu
float temperature = 0.0;
float humidity = 0.0;
float rain = 0.0;

// Kết nối Wi-Fi
void setup() {
  Serial.begin(115200);

  // Kết nối Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Lấy dữ liệu thời tiết
  getWeatherData();
}

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

void loop() {
  // Không cần lặp lại trong loop, chỉ cần thực hiện một lần ở setup()
}