#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

// Cài đặt các chân LED
const int ledPins[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};  // Chân LED tương ứng
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]); // Số lượng LED
const char* ssid = "ESP8266_ControlLED";                   // Tên Wi-Fi
const char* password = "12345678";                         // Mật khẩu Wi-Fi

bool isBlinking = false;  // Trạng thái nhấp nháy
bool isAllLedOn = false;  // Trạng thái bật tất cả LED
bool isRandomMode = false; // Trạng thái sáng ngẫu nhiên
bool isSystemActive = true; // Trạng thái hệ thống
int ledSpeed = 300;  // Tốc độ mặc định là 300ms

// Hàm hiển thị trang điều khiển LED

void handleRoot() {
  String html = "<html><head><style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; }";
  html += "button { font-size: 30px; padding: 10px 20px; margin: 20px; cursor: pointer; width: 200px; }";  // Tăng kích thước nút
  html += "input[type=range] { width: 70%; height: 40px; -webkit-appearance: none; }";  // Tăng chiều cao của thanh trượt
  html += "input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; height: 30px; width: 30px; background: #4CAF50; border-radius: 50%; cursor: pointer; }"; // Tùy chỉnh núm trượt
  html += "input[type=range]::-moz-range-thumb { height: 20px; width: 20px; background: #4CAF50; border-radius: 50%; cursor: pointer; }"; // Tùy chỉnh núm trượt cho Firefox

  // Định nghĩa màu sắc cho các nút
  html += "button.toggleBlink { background-color: blue; color: white; }";  // Nút 1 màu xanh dương
  html += "button.toggleAllLed { background-color: green; color: white; }";  // Nút 2 màu xanh lá
  html += "button.toggleRandom { background-color: yellow; color: black; }";  // Nút 3 màu vàng
  html += "button.toggleSystemOn { background-color: red; color: white; }";  // Nút hệ thống khi bật màu đỏ
  html += "button.toggleSystemOff { background-color: lightgray; color: black; }";  // Nút hệ thống khi tắt màu trắng xám
  
  html += "</style>";
  html += "<script>";
  html += "function updateSpeed(val) { document.getElementById('speedValue').innerHTML = val + 'ms'; }";  // Hàm JavaScript để cập nhật giá trị khi trượt
  html += "</script></head><body>";
  html += "<h1>DIEU KHIEN LED</h1>";
  
  // Hàng nút bao gồm "ON/OFF LED", "Sáng/Tắt tất cả", "Sáng ngẫu nhiên"
  html += "<div style='display: flex; justify-content: center;'>";  // Sử dụng flexbox để căn giữa
  html += "<form action=\"/toggleBlink\" method=\"POST\" style=\"display:inline-block;\">";
  html += "<button type=\"submit\" class=\"toggleBlink\">" + String(isBlinking ? "ON 1" : "OFF 1") + "</button></form>";

  html += "<form action=\"/toggleAllLed\" method=\"POST\" style=\"display:inline-block;\">";
  html += "<button type=\"submit\" class=\"toggleAllLed\">" + String(isAllLedOn ? "ON 2" : "OFF 2") + "</button></form>";

  html += "<form action=\"/toggleRandom\" method=\"POST\" style=\"display:inline-block;\">";
  html += "<button type=\"submit\" class=\"toggleRandom\">" + String(isRandomMode ? "ON 3" : "OFF 3") + "</button></form>";
  html += "</div>";

  // Nút tắt hệ thống
  html += "<form action=\"/toggleSystem\" method=\"POST\">";
  if (isSystemActive) {
      html += "<button type=\"submit\" class=\"toggleSystemOn\">" + String("TAT HE THONG") + "</button></form>";
  } else {
      html += "<button type=\"submit\" class=\"toggleSystemOff\">" + String("BAT HE THONG") + "</button></form>";
  }

  // Thanh trượt điều chỉnh tốc độ
  html += "<form action=\"/setSpeed\" method=\"POST\">";
  html += "<input type=\"range\" name=\"speed\" min=\"100\" max=\"2000\" value=\"" + String(ledSpeed) + "\" oninput=\"updateSpeed(this.value)\">";  // Thanh trượt với giá trị min 100ms và max 2000ms
  html += "<p style='display:inline-block; margin-left: 10px;'>Speed: <span id=\"speedValue\">" + String(ledSpeed) + "ms</span></p>";  // Hiển thị giá trị tốc độ bên phải thanh trượt
  html += "<button type=\"submit\">CAP NHAT</button></form>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}



// Hàm bật/tắt nhấp nháy LED
void handleToggleBlink() {
  isBlinking = !isBlinking;  // Đảo trạng thái nhấp nháy
  server.sendHeader("Location", "/");  // Chuyển hướng về trang chính
  server.send(303);
  handleRoot();  // Trả về giao diện sau khi thực hiện
}

// Hàm bật/tắt chế độ sáng ngẫu nhiên
void handleToggleRandom() {
  isRandomMode = !isRandomMode;
  server.sendHeader("Location", "/");  // Chuyển hướng về trang chính
  server.send(303);
  handleRoot();  // Trả về giao diện sau khi thực hiện
}

// Hàm bật/tắt tất cả LED
void handleToggleAllLed() {
  isAllLedOn = !isAllLedOn;  // Đảo trạng thái bật/tắt tất cả LED
  server.sendHeader("Location", "/");  // Chuyển hướng về trang chính
  server.send(303);
  handleRoot();  // Trả về giao diện sau khi thực hiện
}

// Hàm bật/tắt hệ thống
void handleToggleSystem() {
  isSystemActive = !isSystemActive;  // Đảo trạng thái hệ thống

  if (!isSystemActive) {
    isBlinking = false;  // Dừng nhấp nháy nếu hệ thống tắt
    turnOffAllLeds();  // Tắt tất cả LED
  }

  server.sendHeader("Location", "/");  // Chuyển hướng về trang chính
  server.send(303);
  handleRoot();  // Trả về giao diện sau khi thực hiện
}

// Hàm xử lý thay đổi tốc độ từ thanh trượt
void handleSetSpeed() {
  if (server.hasArg("speed")) {
    ledSpeed = server.arg("speed").toInt();  // Lấy giá trị tốc độ từ thanh trượt
  }
  server.sendHeader("Location", "/");
  server.send(303); // Chuyển hướng về trang chính
  handleRoot();  // Trả về giao diện sau khi thực hiện
}

// ===============================================================================================
// Hàm tắt tất cả LED
void turnOffAllLeds() {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW); // Tắt LED
  }
}

//================== Hàm nhấp nháy LED ===============================
// Hàm bật/tắt toàn bộ LED với cường độ sáng tăng dần và giảm dần
void blinkAllLeds(int times) {
  for (int t = 0; t < times && isSystemActive; t++) {
    // Tăng cường độ sáng dần
    for (int brightness = 0; brightness <= 255; brightness += 10) {
      for (int i = 0; i < numLeds; i++) {
        analogWrite(ledPins[i], brightness);
      }
      delay(20); // Đợi một chút để tạo hiệu ứng tăng dần
    }

    delay(ledSpeed); // Giữ sáng tối đa trong thời gian xác định

    // Giảm cường độ sáng dần
    for (int brightness = 255; brightness >= 0; brightness -= 10) {
      for (int i = 0; i < numLeds; i++) {
        analogWrite(ledPins[i], brightness);
      }
      delay(20); // Đợi một chút để tạo hiệu ứng giảm dần
    }

    delay(ledSpeed); // Giữ tắt hoàn toàn trong thời gian xác định
  }
}

//=============  Hiệu ứng sáng dạng sóng ==================
void waveEffect(int times) {
  for (int t = 0; t < times && isSystemActive; t++) {
    // Hiệu ứng sáng dạng sóng từ trái sang phải
    for (int i = 0; i < numLeds; i++) {
      for (int j = 0; j < numLeds; j++) {
        int brightness = max(0, 255 - abs(i - j) * 50); // Giảm độ sáng theo khoảng cách
        analogWrite(ledPins[j], brightness);
      }
      delay(ledSpeed); // Giữ mỗi lần sáng
    }

    // Hiệu ứng sáng dạng sóng từ phải sang trái
    for (int i = numLeds - 1; i >= 0; i--) {
      for (int j = 0; j < numLeds; j++) {
        int brightness = max(0, 255 - abs(i - j) * 50); // Giảm độ sáng theo khoảng cách
        analogWrite(ledPins[j], brightness);
      }
      delay(ledSpeed); // Giữ mỗi lần sáng
    }
  }
}

// =================== Hàm sáng ngẫu nhiên 3 LED =======================
void randomBlink(int times) {
  for (int t = 0; t < times && isSystemActive; t++) {
    int selectedLeds[3]; // Mảng chứa 3 LED đã chọn

    // Chọn 3 LED ngẫu nhiên không trùng nhau
    for (int i = 0; i < 3; i++) {
      int randomLed;
      bool isUnique;

      // Lặp lại cho đến khi tìm được một LED chưa được chọn
      do {
        randomLed = random(0, numLeds); // Chọn một LED ngẫu nhiên
        isUnique = true;

        // Kiểm tra xem LED đã được chọn hay chưa
        for (int j = 0; j < i; j++) {
          if (selectedLeds[j] == randomLed) {
            isUnique = false;
            break;
          }
        }
      } while (!isUnique);

      selectedLeds[i] = randomLed; // Lưu LED đã chọn
    }

    // Bật sáng 3 LED đã chọn
    for (int i = 0; i < 3; i++) {
      analogWrite(ledPins[selectedLeds[i]], 255); // Bật LED
    }
    delay(ledSpeed); // Giữ sáng trong thời gian xác định

    // Tắt 3 LED đã chọn
    for (int i = 0; i < 3; i++) {
      analogWrite(ledPins[selectedLeds[i]], 0); // Tắt LED
    }
    delay(ledSpeed); // Giữ tắt trong thời gian xác định
  }
}

// ======================== HAM ShiftOnOff ==========================
// Hàm làm sáng dần theo thứ tự từ trái sang phải và từ phải sang trái
void shiftOnOff(int times) {
  for (int t = 0; t < times && isSystemActive; t++) {  // Thêm điều kiện kiểm tra trạng thái hệ thống
    // Bật sáng dần từ trái sang phải với hiệu ứng dim
    for (int i = 0; i < numLeds && isSystemActive; i++) {
      for (int brightness = 0; brightness <= 255 && isSystemActive; brightness += 10) { // Tăng dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ sáng dần
      }
      delay(100); // Giữ sáng mỗi LED 100ms trước khi bật LED tiếp theo
    }

    delay(ledSpeed); // Giữ sáng tất cả LED trong 500ms

    // Tắt dần từ phải sang trái với hiệu ứng dim
    for (int i = numLeds - 1; i >= 0 && isSystemActive; i--) {
      for (int brightness = 255; brightness >= 0 && isSystemActive; brightness -= 10) { // Giảm dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ tắt dần
      }
      delay(100); // Giữ tắt mỗi LED 100ms trước khi tắt LED tiếp theo
    }

    delay(ledSpeed); // Giữ tắt tất cả LED trong 500ms

    // Bật sáng dần từ phải sang trái với hiệu ứng dim
    for (int i = numLeds - 1; i >= 0 && isSystemActive; i--) {
      for (int brightness = 0; brightness <= 255 && isSystemActive; brightness += 10) { // Tăng dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ sáng dần
      }
      delay(100); // Giữ sáng mỗi LED 100ms trước khi bật LED tiếp theo
    }

    delay(ledSpeed); // Giữ sáng tất cả LED trong 500ms

    // Tắt dần từ trái sang phải với hiệu ứng dim
    for (int i = 0; i < numLeds && isSystemActive; i++) {
      for (int brightness = 255; brightness >= 0 && isSystemActive; brightness -= 10) { // Giảm dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ tắt dần
      }
      delay(100); // Giữ tắt mỗi LED 100ms trước khi tắt LED tiếp theo
    }

    delay(ledSpeed); // Giữ tắt tất cả LED trong 500ms
  }
}

// ========== Hàm sáng sole các LED =============
void alternateBlink(int times) {
  for (int t = 0; t < times; t++) {
    // Bật sáng các LED cách nhau một bóng (bóng lẻ)
    for (int i = 0; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], HIGH);
    }
    delay(ledSpeed); // Giữ sáng trong 500ms

    // Tắt các LED lẻ
    for (int i = 0; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], LOW);
    }

    // Bật sáng các LED cách nhau một bóng (bóng chẵn)
    for (int i = 1; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], HIGH);
    }
    delay(ledSpeed); // Giữ sáng trong 500ms

    // Tắt các LED chẵn
    for (int i = 1; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], LOW);
    }
  }  
}
//===================== Hàm sáng từ ngoài vào và từ trong ra  =====================
void inOutBlink(int times) {
  for (int t = 0; t < times; t++) {
    int middle = numLeds / 2;
    
    // Sáng từ ngoài vào trong
    for (int i = 0; i <= middle; i++) {
      digitalWrite(ledPins[i], HIGH);
      digitalWrite(ledPins[numLeds - 1 - i], HIGH);
      delay(ledSpeed); // Giữ mỗi LED sáng 300ms trước khi bật tiếp
    }

    // Tắt từ trong ra ngoài
    for (int i = middle; i >= 0; i--) {
      digitalWrite(ledPins[i], LOW);
      digitalWrite(ledPins[numLeds - 1 - i], LOW);
      delay(ledSpeed); // Giữ mỗi LED tắt 300ms trước khi tắt tiếp
    }
    
    // Sáng từ trong ra ngoài
    for (int i = 0; i <= middle; i++) {
      digitalWrite(ledPins[middle + i], HIGH);
      if (middle - i >= 0) {
        digitalWrite(ledPins[middle - i], HIGH);
      }
      delay(ledSpeed); // Giữ mỗi LED sáng 300ms trước khi bật tiếp
    }

    // Tắt từ ngoài vào trong
    for (int i = 0; i <= middle; i++) {
      digitalWrite(ledPins[middle + i], LOW);
      if (middle - i >= 0) {
        digitalWrite(ledPins[middle - i], LOW);
      }
      delay(ledSpeed); // Giữ mỗi LED tắt 300ms trước khi tắt tiếp
    }
  }
}

//============ Hiệu ứng chạy LED zig-zag mà không có dim và bật n LED mỗi lần
void zigZagEffect(int times) {
  const int n = 2; // Số lượng LED bật mỗi lần
  for (int t = 0; t < times; t++) {
    // Bật từ trái sang phải
    for (int i = 0; i < numLeds; i += n) {
      for (int j = 0; j < n && (i + j) < numLeds; j++) { // Bật tối đa n LED
        digitalWrite(ledPins[i + j], HIGH); // Bật LED
      }
      delay(ledSpeed); // Giữ sáng LED trong 150ms

      // Tắt các LED đã bật
      for (int j = 0; j < n && (i + j) < numLeds; j++) {
        digitalWrite(ledPins[i + j], LOW); // Tắt LED
      }
    }

    // Bật từ phải sang trái
    for (int i = numLeds - 1; i >= 0; i -= n) {
      for (int j = 0; j < n && (i - j) >= 0; j++) { // Bật tối đa n LED
        digitalWrite(ledPins[i - j], HIGH); // Bật LED
      }
      delay(ledSpeed); // Giữ sáng LED trong 150ms

      // Tắt các LED đã bật
      for (int j = 0; j < n && (i - j) >= 0; j++) {
        digitalWrite(ledPins[i - j], LOW); // Tắt LED
      }
    }
  }
}

//===============================================================================================
// Hàm vòng lặp chính

void loop() {
  server.handleClient();  // Xử lý các yêu cầu đến từ client

  // Chạy nhấp nháy LED
  if (isBlinking && isSystemActive) {
    blinkAllLeds(3);  // Nhấp nháy 
    delay(200);
    alternateBlink(2); /// ham sang so le chan le 
    delay(200);
  }
  
  // Chạy hiệu ứng sóng nếu đang bật
  if (isAllLedOn && isSystemActive) {
    waveEffect(2);  // Hiệu ứng sóng
    delay(200);
    inOutBlink(2);  // Hàm sáng từ ngoài vào và từ trong ra
    delay(200);
  }

  // Chạy hiệu ứng sáng ngẫu nhiên nếu đang bật chế độ ngẫu nhiên
  if (isRandomMode && isSystemActive) {
    randomBlink(10);  // Nhấp nháy ngẫu nhiên 
    delay(200);
    shiftOnOff(1);   // Hàm làm sáng dần theo thứ tự từ trái sang phải và từ phải sang trái
    delay(200);
  }
}

// Hàm khởi động
void setup() {
  // Khởi tạo chân LED
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);  // Đặt chế độ chân LED là OUTPUT
    digitalWrite(ledPins[i], LOW); // Tắt LED ban đầu
  }

  // Khởi động Wi-Fi
  WiFi.softAP(ssid, password);  // Tạo điểm phát Wi-Fi
  server.on("/", handleRoot);  // Xử lý yêu cầu tại trang chính
  server.on("/toggleBlink", handleToggleBlink); // Xử lý yêu cầu bật/tắt nhấp nháy
  server.on("/toggleAllLed", handleToggleAllLed); // Xử lý yêu cầu bật/tắt tất cả LED
  server.on("/toggleRandom", handleToggleRandom); // Xử lý yêu cầu bật/tắt sáng ngẫu nhiên
  server.on("/toggleSystem", handleToggleSystem); // Xử lý yêu cầu tắt/bật hệ thống
  server.on("/setSpeed", handleSetSpeed); // Xử lý thay đổi tốc độ
  server.begin();  // Bắt đầu server
}

