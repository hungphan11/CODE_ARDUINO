#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED width
#define SCREEN_HEIGHT 64  // OLED height
#define OLED_RESET    -1  // Reset pin (không cần thiết, dùng -1 nếu không kết nối)

// Khởi tạo đối tượng màn hình OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Khởi động giao tiếp Serial để kiểm tra
  Serial.begin(115200);
  delay(2000);  // Đợi khởi động

  // Khởi động màn hình OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Địa chỉ I2C mặc định là 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  // Dừng nếu khởi động thất bại
  }

  // Xóa màn hình
  display.clearDisplay();

  // Hiển thị văn bản
  display.setTextSize(1);      // Kích thước chữ
  display.setTextColor(SSD1306_WHITE); // Màu trắng
  display.setCursor(0, 0);     // Vị trí bắt đầu hiển thị (x, y)
  display.println(F("Hello, ESP8266!"));
  display.println(F("Testing OLED Display"));

  display.setTextSize(2);      // Kích thước chữ lớn hơn
  display.setCursor(0, 30);    // Chuyển con trỏ xuống vị trí khác
  display.println(F("OLED OK"));

  // Hiển thị nội dung lên màn hình
  display.display(); 

  // Đợi một chút
  delay(2000);

  // Vẽ một số hình ảnh đơn giản
  testdrawlines();      // Vẽ các đường kẻ
  delay(2000);
  testdrawrects();      // Vẽ hình chữ nhật
  delay(2000);
  testfillrects();      // Vẽ và tô hình chữ nhật
  delay(2000);
}

void loop() {
  // Không cần lặp lại gì ở đây, OLED đã hiển thị thông tin
}

// Hàm vẽ các đường thẳng
void testdrawlines() {
  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
}

// Hàm vẽ các hình chữ nhật không tô
void testdrawrects(void) {
  display.clearDisplay();
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
}

// Hàm vẽ và tô các hình chữ nhật
void testfillrects(void) {
  display.clearDisplay();
  for (int16_t i=0; i<display.height()/2; i+=3) {
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, SSD1306_WHITE);
    display.display();
    delay(1);
  }
}
