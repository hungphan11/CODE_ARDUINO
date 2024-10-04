#include <EEPROM.h>  // Thư viện EEPROM

// Khai báo chân kết nối
const int ledPins[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3, A4, A5};  // Chân LED
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);
const int pinUp = 2;   // Chân nút tăng tốc độ
const int pinDown = 3; // Chân nút giảm tốc độ

// Địa chỉ EEPROM để lưu trữ tốc độ nhấp nháy
const int blinkDelayAddress = 0;

// Biến điều khiển tốc độ nhấp nháy (ms)
int blinkDelay = 50;  // Giá trị mặc định

// Biến trạng thái để theo dõi nút
bool buttonUpPressed = false;
bool buttonDownPressed = false;

// Hàm setup
void setup() {
  // Thiết lập các chân LED làm OUTPUT
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Thiết lập các chân nút bấm làm INPUT_PULLUP
  pinMode(pinUp, INPUT_PULLUP);
  pinMode(pinDown, INPUT_PULLUP);

  // Đọc giá trị tốc độ từ EEPROM
  blinkDelay = EEPROM.read(blinkDelayAddress) * 10;
  if (blinkDelay < 10 || blinkDelay > 2000) {  // Thay đổi giới hạn dưới để nhanh hơn
    blinkDelay = 50;  // Nếu giá trị không hợp lệ, đặt lại giá trị mặc định
  }
}

// Hàm loop
void loop() {
  handleButtonPress();
  
  // Thực hiện hiệu ứng đèn LED
  chaseLeftRight();      // Sáng đuổi trái phải
  lightInOut();          // Sáng dồn tắt dồn
  alternateBlink();      // Sáng nhấp nháy xo le LED
  middleToOutside();     // Sáng từ giữa ra từ ngoài vào
  randomBlink();         // Sáng ngẫu nhiên
}

// ============================= CHƯƠNG TRÌNH CHO LED =============================

// Hàm nhấp nháy LED
void blinkLED() {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
  delay(blinkDelay);
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  delay(blinkDelay);
}

// Hàm LED sáng đuổi trái phải
void chaseLeftRight() {
  int groupSize = 3;  // Số lượng LED sẽ sáng cùng lúc

  // Đuổi từ trái sang phải
  for (int i = 0; i <= numLeds - groupSize; i++) {
    // Bật 3 LED liên tiếp
    for (int j = 0; j < groupSize; j++) {
      digitalWrite(ledPins[i + j], HIGH);
    }
    delay(blinkDelay);

    // Tắt 3 LED liên tiếp
    for (int j = 0; j < groupSize; j++) {
      digitalWrite(ledPins[i + j], LOW);
    }
  }

  // Đuổi từ phải sang trái
  for (int i = numLeds - groupSize; i >= 0; i--) {
    // Bật 3 LED liên tiếp
    for (int j = 0; j < groupSize; j++) {
      digitalWrite(ledPins[i + j], HIGH);
    }
    delay(blinkDelay);

    // Tắt 3 LED liên tiếp
    for (int j = 0; j < groupSize; j++) {
      digitalWrite(ledPins[i + j], LOW);
    }
  }
}

// Hàm sáng dồn, tắt dồn
void lightInOut() {
  int groupSize = 3;  // Số lượng LED sẽ sáng cùng lúc

  // Sáng dồn từ trái sang phải
  for (int i = 0; i <= numLeds - groupSize; i++) {
    // Bật 3 LED liên tiếp
    for (int j = 0; j < groupSize; j++) {
      digitalWrite(ledPins[i + j], HIGH);
    }
    delay(blinkDelay);
  }

  // Tắt dồn từ phải sang trái
  for (int i = numLeds - groupSize; i >= 0; i--) {
    // Tắt 3 LED liên tiếp
    for (int j = 0; j < groupSize; j++) {
      digitalWrite(ledPins[i + j], LOW);
    }
    delay(blinkDelay);
  }
}

// Hàm sáng nhấp nháy xo le
void alternateBlink() {
  // Sáng các LED lẻ
  for (int i = 0; i < numLeds; i += 2) {
    digitalWrite(ledPins[i], HIGH);
  }
  delay(blinkDelay);

  // Tắt các LED lẻ và sáng các LED chẵn
  for (int i = 0; i < numLeds; i += 2) {
    digitalWrite(ledPins[i], LOW);
  }
  for (int i = 1; i < numLeds; i += 2) {
    digitalWrite(ledPins[i], HIGH);
  }
  delay(blinkDelay);

  // Tắt tất cả các LED
  for (int i = 1; i < numLeds; i += 2) {
    digitalWrite(ledPins[i], LOW);
  }
}

// Hàm sáng từ giữa ra từ ngoài vào
void middleToOutside() {
  int middle = numLeds / 2;
  
  // Sáng từ giữa ra ngoài
  for (int i = 0; i <= middle; i++) {
    if (middle - i >= 0) {
      digitalWrite(ledPins[middle - i], HIGH);
    }
    if (middle + i < numLeds) {
      digitalWrite(ledPins[middle + i], HIGH);
    }
    delay(blinkDelay);
  }

  // Tắt từ ngoài vào giữa
  for (int i = middle; i >= 0; i--) {
    if (middle - i >= 0) {
      digitalWrite(ledPins[middle - i], LOW);
    }
    if (middle + i < numLeds) {
      digitalWrite(ledPins[middle + i], LOW);
    }
    delay(blinkDelay);
  }
}

// Hàm sáng ngẫu nhiên
void randomBlink() {
  // Số lần LED sáng ngẫu nhiên
  int randomTimes = 10;  // Số lần random sáng
  for (int i = 0; i < randomTimes; i++) {
    // Chọn ngẫu nhiên số lượng LED sẽ sáng (2 hoặc 3)
    int numRandomLeds = random(2, 4);  // Số lượng LED ngẫu nhiên sẽ sáng (2 hoặc 3)

    // Mảng để lưu các chỉ số LED ngẫu nhiên
    int selectedLeds[numRandomLeds];

    // Chọn LED ngẫu nhiên để sáng
    for (int j = 0; j < numRandomLeds; j++) {
      int randomLed;
      bool isUnique;

      // Chọn LED ngẫu nhiên và đảm bảo không bị trùng
      do {
        randomLed = random(0, numLeds);
        isUnique = true;

        // Kiểm tra xem LED đã được chọn chưa
        for (int k = 0; k < j; k++) {
          if (selectedLeds[k] == randomLed) {
            isUnique = false;
            break;
          }
        }
      } while (!isUnique);

      // Lưu chỉ số LED đã chọn
      selectedLeds[j] = randomLed;

      // Bật LED ngẫu nhiên
      digitalWrite(ledPins[randomLed], HIGH);
    }

    // Giữ sáng trong khoảng thời gian ngẫu nhiên
    delay(random(50, 100));  // Thời gian sáng ngẫu nhiên nhanh (50-100 ms)

    // Tắt các LED đã sáng
    for (int j = 0; j < numRandomLeds; j++) {
      digitalWrite(ledPins[selectedLeds[j]], LOW);
    }

    // Giữ tắt trong khoảng thời gian ngẫu nhiên
    delay(random(50, 100));  // Thời gian tắt ngẫu nhiên nhanh (50-100 ms)
  }
}

// =============================================================================================================================
// Hàm xử lý nút bấm
void handleButtonPress() 
{
  // Kiểm tra nút tăng tốc độ
  if (digitalRead(pinUp) ==   LOW) 
  {
    if (!buttonUpPressed) // Chỉ thực hiện nếu nút chưa được nhấn
    {  
      increaseBlinkSpeed();
      buttonUpPressed = true;  // Đánh dấu nút đã được nhấn
    }
  } 
  else 
  {
    buttonUpPressed = false;  // Đánh dấu nút đã được thả ra
  }
  
  // Kiểm tra nút giảm tốc độ
  if (digitalRead(pinDown) == LOW) 
  {
    if (!buttonDownPressed) // Chỉ thực hiện nếu nút chưa được nhấn
    {  
      decreaseBlinkSpeed();
      buttonDownPressed = true;  // Đánh dấu nút đã được nhấn
    }
  } 
  else 
  {
    buttonDownPressed = false;  // Đánh dấu nút đã được thả ra
  }
}

// Hàm tăng tốc độ nhấp nháy
void increaseBlinkSpeed() 
{
  if (blinkDelay > 10) // Đảm bảo không giảm tốc độ quá thấp
  {  
    blinkDelay -= 10;  // Giảm tốc độ nhanh hơn
    saveBlinkSpeed();  // Lưu giá trị mới vào EEPROM
  }
}

// Hàm giảm tốc độ nhấp nháy
void decreaseBlinkSpeed() 
{
  blinkDelay += 10;  // Tăng tốc độ nhanh hơn
  saveBlinkSpeed();  // Lưu giá trị mới vào EEPROM
}

// Hàm lưu tốc độ nhấp nháy vào EEPROM
void saveBlinkSpeed()
{
  int valueToStore = blinkDelay / 10;  // Lưu giá trị chia 10 để tiết kiệm bộ nhớ
  EEPROM.write(blinkDelayAddress, valueToStore);
}
