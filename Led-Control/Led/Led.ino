#define BLYNK_TEMPLATE_ID "TMPLKkXl1hPU"
#define BLYNK_TEMPLATE_NAME "4RL"
#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG
#define USE_NODE_MCU_BOARD

#include "BlynkEdgent.h"

// Định nghĩa các chân LED bằng mảng
const int ledPins[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8}; 
const int ledCount = sizeof(ledPins) / sizeof(ledPins[0]); // Tính số lượng phần tử của mảng

unsigned long previousMillis = 0;
long val = 500; // Thời gian delay mặc định

bool switch1 = false; // Biến để theo dõi trạng thái của switch 1
bool switch2 = false; // Biến để theo dõi trạng thái của switch 2
volatile bool switch3 = false;  // Biến để theo dõi trạng thái của switch tổng

unsigned long times = millis();
WidgetLED led_connect(V0);

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

// ======================== HÀM BẬT/TẮT TẤT CẢ LED ======================
void blinkAllLeds(int n, volatile bool &switch3) {
  for (int t = 0; t < n; t++) {
    unsigned long currentMillis = millis();

    // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay
    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    }

    if (currentMillis - previousMillis >= val) {
      previousMillis = currentMillis; // Cập nhật thời gian trước đó

      // Tăng độ sáng LED
      for (int brightness = 0; brightness <= 255; brightness += 30) {
        for (int i = 0; i < ledCount; i++) {
          analogWrite(ledPins[i], brightness);
        }
        delay(val / 50); // Giữ khoảng delay ngắn để cập nhật sáng mờ
      }

      delay(val); // Sử dụng thời gian `val` đã cập nhật
      
      // Giảm độ sáng LED
      for (int brightness = 255; brightness >= 0; brightness -= 30) {
        for (int i = 0; i < ledCount; i++) {
          analogWrite(ledPins[i], brightness);
        }
        delay(val / 50); // Giữ khoảng delay ngắn để cập nhật sáng mờ
      }

      delay(val);
    }
  }
}

// ======================== HÀM SHIFT ON/OFF ======================
// Hàm làm sáng dần theo thứ tự từ trái sang phải và từ phải sang trái
void shiftOnOff(int n) {
  for (int t = 0; t < n; t++) {
    unsigned long currentMillis = millis();

    // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay
    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    }
    
    if (currentMillis - previousMillis >= val) {
      previousMillis = currentMillis; // Cập nhật thời gian trước đó
      for (int i = 0; i < ledCount; i++) {
        for (int brightness = 0; brightness <= 255; brightness += 10) {
          analogWrite(ledPins[i], brightness);
          delay(val / 50);
        }
        delay(val / 5);
      }
      delay(val);
      for (int i = ledCount - 1; i >= 0; i--) {
        for (int brightness = 255; brightness >= 0; brightness -= 10) {
          analogWrite(ledPins[i], brightness);
          delay(val / 50);
        }
        delay(val / 5);
      }
      delay(val);
    }
  }
}

// ======================== HÀM ALTERNATE BLINK ======================
// hàm led sáng so le
void alternateBlink(int n) {
  unsigned long previousMillis = 0; // Khởi tạo biến để theo dõi thời gian
  for (int t = 0; t < n; t++) {
    unsigned long currentMillis = millis();

    // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay
    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    }
    
    // Nếu đã đủ thời gian để thực hiện
    if (currentMillis - previousMillis >= val) {
      previousMillis = currentMillis; // Cập nhật thời gian trước đó

      // Bật sáng các LED lẻ
      for (int i = 0; i < ledCount; i += 2) {
        digitalWrite(ledPins[i], HIGH);
      }
      delay(500); // Giữ sáng trong 500ms

      // Tắt các LED lẻ
      for (int i = 0; i < ledCount; i += 2) {
        digitalWrite(ledPins[i], LOW);
      }

      // Bật sáng các LED chẵn
      for (int i = 1; i < ledCount; i += 2) {
        digitalWrite(ledPins[i], HIGH);
      }
      delay(500); // Giữ sáng trong 500ms

      // Tắt các LED chẵn
      for (int i = 1; i < ledCount; i += 2) {
        digitalWrite(ledPins[i], LOW);
      }
    }
  }
}

// ======================== HÀM IN-OUT BLINK ======================
// Hàm sáng từ ngoài vào và từ trong ra
void inOutBlink(int n) {
  unsigned long previousMillis = 0; // Khởi tạo biến để theo dõi thời gian
  for (int t = 0; t < n; t++) {
    unsigned long currentMillis = millis();

    // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay
    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    }

    if (currentMillis - previousMillis >= val) {
      previousMillis = currentMillis;
      int middle = ledCount / 2;
      
      // Bật LED
      for (int i = 0; i <= middle; i++) {
        digitalWrite(ledPins[i], HIGH);
        digitalWrite(ledPins[ledCount - 1 - i], HIGH);
        delay(200); // Giữ sáng LED
      }
      //delay(200); // Giữ sáng LED sau khi bật
      // Tắt LED
      for (int i = 0; i <= middle; i++) {
        digitalWrite(ledPins[i], LOW);
        digitalWrite(ledPins[ledCount - 1 - i], LOW);
        delay(200); // Giữ sáng LED trước khi tắt
      }

      // Sáng từ trong ra ngoài
      for (int i = 0; i <= middle; i++) {
        digitalWrite(ledPins[middle + i], HIGH);
        if (middle - i >= 0) {
          digitalWrite(ledPins[middle - i], HIGH);
        }
        delay(200); // Giữ mỗi LED sáng 300ms trước khi bật tiếp
      }

      // Tắt từ ngoài vào trong
      for (int i = 0; i <= middle; i++) {
        digitalWrite(ledPins[middle + i], LOW);
        if (middle - i >= 0) {
          digitalWrite(ledPins[middle - i], LOW);
        }
        delay(200); // Giữ mỗi LED tắt 300ms trước khi tắt tiếp
      }
    }
  }
}

// ======================== HÀM RANDOM BLINK ======================
// Hàm sáng ngẫu nhiên 3 LED cùng một lúc
void randomBlink(int times) {
  for (int t = 0; t < times; t++) {
    
    // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay
    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    }

    int selectedLeds[3]; // Mảng chứa 3 LED đã chọn
    // Chọn 3 LED ngẫu nhiên không trùng nhau
    for (int i = 0; i < 3; i++) {
      int randomLed;
      bool isUnique;

      // Lặp lại cho đến khi tìm được một LED chưa được chọn
      do {
        randomLed = random(0, ledCount); // Chọn một LED ngẫu nhiên
        isUnique = true;

        // Kiểm tra xem LED đã được chọn hay chưa
        for (int j = 0; j < i; j++) {
          if (selectedLeds[j] == randomLed) {
            isUnique = false;
            break;
          }
        }
      } while (!isUnique);

      selectedLeds[i] = randomLed; // Lưu LED đã chọn vào mảng
      digitalWrite(ledPins[randomLed], HIGH); // Bật LED
    }

    delay(300); // Giữ sáng 200ms

    // Tắt tất cả các LED vừa chọn
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPins[selectedLeds[i]], LOW); // Tắt LED
    }
  }
}

// ======================== HÀM WAVE EFFECT ====================== 
// Hiệu ứng sáng dạng sóng từ trái sang phải và từ phải sang trái
void waveEffect(int n) {
  unsigned long previousMillis = 0; // Khởi tạo biến để theo dõi thời gian
  // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay  

  for (int t = 0; t < n; t++) {
    unsigned long currentMillis = millis();

    // Kiểm tra nếu switch3 đã tắt, nếu có thì thoát ngay
    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    }
    
    // Nếu đã đủ thời gian để thực hiện
    if (currentMillis - previousMillis >= val) {
      previousMillis = currentMillis; // Cập nhật thời gian trước đó
      // Hiệu ứng sáng dạng sóng từ trái sang phải
      for (int i = 0; i < ledCount; i++) {
        for (int j = 0; j < ledCount; j++) {
          int brightness = max(0, 255 - abs(i - j) * 50); // Giảm độ sáng theo khoảng cách
          analogWrite(ledPins[j], brightness);
        }
        delay(200); // Giữ mỗi lần sáng 150ms
      }

      // Hiệu ứng sáng dạng sóng từ phải sang trái
      for (int i = ledCount - 1; i >= 0; i--) {
        for (int j = 0; j < ledCount; j++) {
          int brightness = max(0, 255 - abs(i - j) * 50); // Giảm độ sáng theo khoảng cách
          analogWrite(ledPins[j], brightness);
        }
        delay(200); // Giữ mỗi lần sáng 150ms
      }
    }
  }
}
    

// ======================== HÀM ZIG-ZAG EFFECT ======================
// Hiệu ứng chạy LED zig-zag mà không có dim và bật n LED mỗi lần
void zigZagEffect(int times) {
  const int n = 2; // Số lượng LED bật mỗi lần
  for (int t = 0; t < times; t++) {

    if (!switch3) {
      StopAllLed(); // Tắt tất cả LED khi switch3 bị tắt
      return; // Thoát hàm
    } 

    // Bật từ trái sang phải
    for (int i = 0; i < ledCount; i += n) {
      for (int j = 0; j < n && (i + j) < ledCount; j++) { // Bật tối đa n LED
        digitalWrite(ledPins[i + j], HIGH); // Bật LED
      }
      delay(val*2); // Thay đổi thời gian giữ sáng LED theo giá trị val

      // Tắt các LED đã bật
      for (int j = 0; j < n && (i + j) < ledCount; j++) {
        digitalWrite(ledPins[i + j], LOW); // Tắt LED
      }
    }

    // Bật từ phải sang trái
    for (int i = ledCount - 1; i >= 0; i -= n) {
      for (int j = 0; j < n && (i - j) >= 0; j++) { // Bật tối đa n LED
        digitalWrite(ledPins[i - j], HIGH); // Bật LED
      }
      delay(val*2); // Thay đổi thời gian giữ sáng LED theo giá trị val

      // Tắt các LED đã bật
      for (int j = 0; j < n && (i - j) >= 0; j++) {
        digitalWrite(ledPins[i - j], LOW); // Tắt LED
      }
    }
  }
}


// ======================== HÀM CHƯƠNG TRÌNH CHÍNH ======================
void loop() {
  BlynkEdgent.run();

  // Gửi thời gian đã trôi qua đến Blynk
  if (millis() - times > 1000) {
    Blynk.virtualWrite(V5, millis() / 1000);
    if (led_connect.getValue()) {
      led_connect.off();
    } else {
      led_connect.on();
    }
    times = millis();
  }


  // Kiểm tra các switch và gọi hàm tương ứng
  if (switch3) {
    if (switch1) {
      blinkAllLeds(3, switch3);
      if (!switch3) return; // Dừng nếu switch3 đã tắt

      shiftOnOff(3);   // Chạy chế độ bật/tắt LED tuần tự
      if (!switch3) return;

      inOutBlink(3);
      if (!switch3) return;

      alternateBlink(3);
      if (!switch3) return;
    }
    if (switch2) {
      randomBlink(20); // Chạy chế độ random blink LED
      if (!switch3) return; // Dừng nếu switch3 đã tắt
      waveEffect(3);
      if (!switch3) return;   // Chạy chế độ sóng LED
      zigZagEffect(3); // Chạy chế độ zig-zag LED
      if (!switch3) return; // Dừng nếu switch3 đã tắt
    }
    if (!switch1 && !switch2) {
      StopAllLed(); // Tắt tất cả LED nếu không có switch nào được bật
    }
  } 
  else {
    StopAllLed(); // Tắt tất cả LED nếu switch3 không được bật
  }
}

// ======================== HÀM KẾT NỐI BLYNK ======================
BLYNK_CONNECTED() {
  Blynk.syncAll();
}

// ======================== HÀM ĐIỀU KHIỂN CÁC SWITCH ======================
BLYNK_WRITE(V1) { // Điều khiển switch 1
  int p = param.asInt();
  switch1 = (p > 0);
}

BLYNK_WRITE(V2) { // Điều khiển switch 2
  int p = param.asInt();
  switch2 = (p > 0);
}

BLYNK_WRITE(V3) { // Điều khiển switch tổng
  int p = param.asInt();
  switch3 = (p > 0);
  if (!switch3) {
    StopAllLed(); // Tắt tất cả LED ngay khi switch3 được tắt
  }
}

BLYNK_WRITE(V4) { // Điều khiển tốc độ
  int sliderValue = param.asInt();
  val = map(sliderValue, 0, 100, 1000, 50); // Tốc độ chung cho tất cả hiệu ứng
  Serial.print("Speed: ");
  Serial.println(val);
}
