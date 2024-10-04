#define BLYNK_TEMPLATE_ID "TMPLKkXl1hPU"
#define BLYNK_TEMPLATE_NAME "4RL"

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include "BlynkEdgent.h"

// Định nghĩa các chân LED bằng mảng
const int ledPins[] = {D0,D1,D2,D3,D4,D5,D6,D7,D8}; 
const int ledCount = sizeof(ledPins) / sizeof(ledPins[0]); // Tính số lượng phần tử của mảng

unsigned long previousMillis = 0;
long val = 500; // Thời gian delay mặc định
int n = 5; // Số lần lặp lại cho các hiệu ứng
int currentCount = 0; // Biến đếm số lần đã chạy hiệu ứng
WidgetLED led_connect(V0);

bool switch1 = false; // Biến để theo dõi trạng thái của switch 1
bool switch2 = false; // Biến để theo dõi trạng thái của switch 2
bool switch3 = false; // Biến để theo dõi trạng thái của switch 3
bool fadeOut = false;
bool direction = true; // true: trái sang phải, false: phải sang trái
int currentLED = 0; // LED hiện tại đang sáng
unsigned long times = millis();

void setup() {
  Serial.begin(115200);
  delay(100);
  BlynkEdgent.begin();

  // Thiết lập chế độ cho các chân LED
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  Serial.println("---------------------Int--------------------");
}

// ======================== HÀM TẮT TẤT CẢ LED ======================
void StopAllLed() {
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

// ======================== HÀM SÁNG TẮT ========================
void SangTat() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= val) {
    previousMillis = currentMillis; // Cập nhật thời gian trước đó

    // Đảo trạng thái tất cả các LED
    for (int i = 0; i < ledCount; i++) {
      digitalWrite(ledPins[i], !digitalRead(ledPins[i]));
    }
    // Tăng biến đếm số lần
    currentCount++;
    if (currentCount >= n) {
      StopAllLed();
      currentCount = 0; // Đặt lại biến đếm
    }
  }
}

// ========================= HÀM SÁNG xole ==========================
void SangXoLe() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= val) {
    previousMillis = currentMillis; // Cập nhật thời gian trước đó

    for (int i = 0; i < ledCount; i++) {
      digitalWrite(ledPins[i], (i % 2 == 0)); // Bật LED chẵn
    }

    currentCount++;
    if (currentCount >= n) {
      StopAllLed();
      currentCount = 0; // Đặt lại biến đếm
    }
  }
}


// ========================= HÀM ĐUỔI LED ==========================
void SangDuoi() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= val) {
    previousMillis = currentMillis; // Cập nhật thời gian trước đó

    StopAllLed(); // Tắt tất cả LED trước khi chạy hiệu ứng
    digitalWrite(ledPins[currentCount % ledCount], HIGH); // Bật LED hiện tại

    currentCount++;
    if (currentCount >= n * ledCount) {
      currentCount = 0; // Đặt lại biến đếm
      StopAllLed();
    }
  }
}

// ========================= HÀM SÁNG DÀN ==========================
void SangDan() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= val) {
    previousMillis = currentMillis; // Cập nhật thời gian trước đó

    for (int i = 0; i < ledCount; i++) {
      if (i == currentCount) {
        digitalWrite(ledPins[i], HIGH); // Bật LED hiện tại
      } else {
        digitalWrite(ledPins[i], LOW); // Tắt LED khác
      }
    }

    currentCount++;
    if (currentCount >= ledCount) {
      currentCount = 0; // Đặt lại biến đếm
    }
  }
}

// ============================ HÀM RANDOM ==========================
void Random() {
  if (currentCount < n) {
    int randomIndex = random(0, ledCount); // Chọn ngẫu nhiên một LED
    digitalWrite(ledPins[randomIndex], HIGH); // Bật LED ngẫu nhiên
    delay(val); // Chờ một chút
    digitalWrite(ledPins[randomIndex], LOW); // Tắt LED
    currentCount++;
  }
}


// ============================= HÀM CHƯƠNG TRÌNH CHÍNH ====================================
void loop() {
  BlynkEdgent.run();
  if (millis() - times > 1000) {
    Blynk.virtualWrite(V5, millis() / 1000);
    if (led_connect.getValue()) {
      led_connect.off();
    } else {
      led_connect.on();
    }
    times = millis();
  }
  // Gọi hàm tương ứng dựa trên trạng thái của các switch
  if (switch3) {
    if (switch1) {
      Serial.println("LED 1 ");
      delay(2000);
      Serial.println("LED 2 ");
      delay(2000);
      Serial.println("LED 3");
      delay(2000);
    } else if (switch2) {
       Serial.println(" CT1");
      delay(2000);
      Serial.println("CT2 ");
      delay(2000);
      Serial.println("CT3");
      delay(2000);
    }
  } else {
    StopAllLed(); // Tắt tất cả LED nếu không có hiệu ứng nào đang chạy
  }
}

// ============================================================
BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(V1) { // Điều khiển switch 1
  int p = param.asInt();
  switch1 = (p > 0); // Bật hoặc tắt switch 1
}

BLYNK_WRITE(V2) { // Điều khiển switch 2
  int p = param.asInt();
  switch2 = (p > 0); // Bật hoặc tắt switch 2
}

BLYNK_WRITE(V3) { // Điều khiển switch tổng
  int p = param.asInt();
  switch3 = (p > 0); // Bật hoặc tắt nút tổng
}

BLYNK_WRITE(V4) { // Điều khiển tốc độ
  int sliderValue = param.asInt();
  val = map(sliderValue, 0, 100, 1000, 100); // Tốc độ chung cho tất cả hiệu ứng
  Serial.print("Speed: ");
  Serial.println(val);
}
