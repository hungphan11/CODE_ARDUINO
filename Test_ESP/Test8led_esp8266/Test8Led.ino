const int ledPins[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);

void setup() {
  // Thiết lập các chân LED là OUTPUT
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  // Gọi hàm để tất cả các LED sáng tắt 3 lần
  blinkAllLeds(3);

  // Gọi hàm để làm sáng dồn và tắt dần
  shiftOnOff(3);
  // ================Hàm sáng xỏe các LED
  alternateBlink(5);
  // Hàm sáng từ ngoài vào và từ trong ra
  inOutBlink(3);

  randomBlink(20);
  // Hiệu ứng sáng dạng sóng từ trái sang phải và từ phải sang trái
  waveEffect(3);
  delay(300);
  // Hiệu ứng chạy LED zig-zag
  zigZagEffect(3);

  delay(1000); // Đợi 1 giây trước khi lặp lại
}

// Hàm bật/tắt toàn bộ LED với cường độ sáng tăng dần và giảm dần
void blinkAllLeds(int times) {
  for (int t = 0; t < times; t++) {
    // Tăng cường độ sáng dần
    for (int brightness = 0; brightness <= 255; brightness += 10) { // Tăng dần từ 0 đến 255
      for (int i = 0; i < numLeds; i++) {
        analogWrite(ledPins[i], brightness); // Điều chỉnh độ sáng
      }
      delay(20); // Đợi một chút để tạo hiệu ứng tăng dần
    }

    delay(500); // Giữ sáng tối đa trong 500ms

    // Giảm cường độ sáng dần
    for (int brightness = 255; brightness >= 0; brightness -= 10) { // Giảm dần từ 255 xuống 0
      for (int i = 0; i < numLeds; i++) {
        analogWrite(ledPins[i], brightness); // Điều chỉnh độ sáng
      }
      delay(20); // Đợi một chút để tạo hiệu ứng giảm dần
    }

    delay(500); // Giữ tắt hoàn toàn trong 500ms
  }
}


// Hàm làm sáng dần theo thứ tự từ trái sang phải và từ phải sang trái
void shiftOnOff(int times) {
  for (int t = 0; t < times; t++) {
    // Bật sáng dần từ trái sang phải với hiệu ứng dim
    for (int i = 0; i < numLeds; i++) {
      for (int brightness = 0; brightness <= 255; brightness += 10) { // Tăng dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ sáng dần
      }
      delay(100); // Giữ sáng mỗi LED 200ms trước khi bật LED tiếp theo
    }

    delay(500); // Giữ sáng tất cả LED trong 500ms

    // Tắt dần từ phải sang trái với hiệu ứng dim
    for (int i = numLeds - 1; i >= 0; i--) {
      for (int brightness = 255; brightness >= 0; brightness -= 10) { // Giảm dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ tắt dần
      }
      delay(100); // Giữ tắt mỗi LED 200ms trước khi tắt LED tiếp theo
    }

    delay(500); // Giữ tắt tất cả LED trong 500ms

    // Bật sáng dần từ phải sang trái với hiệu ứng dim
    for (int i = numLeds - 1; i >= 0; i--) {
      for (int brightness = 0; brightness <= 255; brightness += 10) { // Tăng dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ sáng dần
      }
      delay(100); // Giữ sáng mỗi LED 200ms trước khi bật LED tiếp theo
    }

    delay(500); // Giữ sáng tất cả LED trong 500ms

    // Tắt dần từ trái sang phải với hiệu ứng dim
    for (int i = 0; i < numLeds; i++) {
      for (int brightness = 255; brightness >= 0; brightness -= 10) { // Giảm dần độ sáng
        analogWrite(ledPins[i], brightness);
        delay(10); // Điều chỉnh thời gian để thay đổi tốc độ tắt dần
      }
      delay(100); // Giữ tắt mỗi LED 200ms trước khi tắt LED tiếp theo
    }

    delay(500); // Giữ tắt tất cả LED trong 500ms
  }
}


// ================Hàm sáng sole các LED
void alternateBlink(int times) {
  for (int t = 0; t < times; t++) {
    // Bật sáng các LED cách nhau một bóng (bóng lẻ)
    for (int i = 0; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], HIGH);
    }
    delay(500); // Giữ sáng trong 500ms

    // Tắt các LED lẻ
    for (int i = 0; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], LOW);
    }

    // Bật sáng các LED cách nhau một bóng (bóng chẵn)
    for (int i = 1; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], HIGH);
    }
    delay(500); // Giữ sáng trong 500ms

    // Tắt các LED chẵn
    for (int i = 1; i < numLeds; i += 2) {
      digitalWrite(ledPins[i], LOW);
    }
  }  
}

// Hàm sáng từ ngoài vào và từ trong ra
void inOutBlink(int times) {
  for (int t = 0; t < times; t++) {
    int middle = numLeds / 2;
    
    // Sáng từ ngoài vào trong
    for (int i = 0; i <= middle; i++) {
      digitalWrite(ledPins[i], HIGH);
      digitalWrite(ledPins[numLeds - 1 - i], HIGH);
      delay(300); // Giữ mỗi LED sáng 300ms trước khi bật tiếp
    }

    // Tắt từ trong ra ngoài
    for (int i = middle; i >= 0; i--) {
      digitalWrite(ledPins[i], LOW);
      digitalWrite(ledPins[numLeds - 1 - i], LOW);
      delay(300); // Giữ mỗi LED tắt 300ms trước khi tắt tiếp
    }
    
    // Sáng từ trong ra ngoài
    for (int i = 0; i <= middle; i++) {
      digitalWrite(ledPins[middle + i], HIGH);
      if (middle - i >= 0) {
        digitalWrite(ledPins[middle - i], HIGH);
      }
      delay(300); // Giữ mỗi LED sáng 300ms trước khi bật tiếp
    }

    // Tắt từ ngoài vào trong
    for (int i = 0; i <= middle; i++) {
      digitalWrite(ledPins[middle + i], LOW);
      if (middle - i >= 0) {
        digitalWrite(ledPins[middle - i], LOW);
      }
      delay(300); // Giữ mỗi LED tắt 300ms trước khi tắt tiếp
    }
  }
}


// Hàm sáng ngẫu nhiên 3 LED cùng một lúc
void randomBlink(int times) {
  for (int t = 0; t < times; t++) {
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

      selectedLeds[i] = randomLed; // Lưu LED đã chọn vào mảng
      digitalWrite(ledPins[randomLed], HIGH); // Bật LED
    }

    delay(200); // Giữ sáng 200ms

    // Tắt tất cả các LED vừa chọn
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPins[selectedLeds[i]], LOW); // Tắt LED
    }
  }
}
// Hiệu ứng sáng dạng sóng từ trái sang phải và từ phải sang trái
void waveEffect(int times) {
  for (int t = 0; t < times; t++) {
    // Hiệu ứng sáng dạng sóng từ trái sang phải
    for (int i = 0; i < numLeds; i++) {
      for (int j = 0; j < numLeds; j++) {
        int brightness = max(0, 255 - abs(i - j) * 50); // Giảm độ sáng theo khoảng cách
        analogWrite(ledPins[j], brightness);
      }
      delay(150); // Giữ mỗi lần sáng 150ms
    }

    // Hiệu ứng sáng dạng sóng từ phải sang trái
    for (int i = numLeds - 1; i >= 0; i--) {
      for (int j = 0; j < numLeds; j++) {
        int brightness = max(0, 255 - abs(i - j) * 50); // Giảm độ sáng theo khoảng cách
        analogWrite(ledPins[j], brightness);
      }
      delay(150); // Giữ mỗi lần sáng 150ms
    }
  }
}



// Hiệu ứng chạy LED zig-zag mà không có dim và bật n LED mỗi lần
void zigZagEffect(int times) {
  const int n = 2; // Số lượng LED bật mỗi lần
  for (int t = 0; t < times; t++) {
    // Bật từ trái sang phải
    for (int i = 0; i < numLeds; i += n) {
      for (int j = 0; j < n && (i + j) < numLeds; j++) { // Bật tối đa n LED
        digitalWrite(ledPins[i + j], HIGH); // Bật LED
      }
      delay(200); // Giữ sáng LED trong 150ms

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
      delay(200); // Giữ sáng LED trong 150ms

      // Tắt các LED đã bật
      for (int j = 0; j < n && (i - j) >= 0; j++) {
        digitalWrite(ledPins[i - j], LOW); // Tắt LED
      }
    }
  }
}


