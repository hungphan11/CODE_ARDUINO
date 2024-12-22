#include <ESP32Servo.h>

Servo servo1;  // Tạo đối tượng servo 1
Servo servo2;  // Tạo đối tượng servo 2

void setup() {
  Serial.begin(115200);

  // Gán servo 1 vào chân 25 và servo 2 vào chân 26
  servo1.attach(25);
  servo2.attach(26);

  Serial.println("Servos are ready.");
}

void loop() {
  // Quay cả hai servo từ 0 đến 180 độ
  for (int pos = 0; pos <= 180; pos++) {
    servo1.write(pos);  // Điều khiển servo 1 đến vị trí pos
    servo2.write(pos);  // Điều khiển servo 2 đến vị trí pos
    delay(15);          // Chờ để servo di chuyển
  }

  // Quay cả hai servo từ 180 về 0 độ
  for (int pos = 180; pos >= 0; pos--) {
    servo1.write(pos);  // Điều khiển servo 1 đến vị trí pos
    servo2.write(pos);  // Điều khiển servo 2 đến vị trí pos
    delay(15);          // Chờ để servo di chuyển
  }
}
