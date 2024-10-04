#define ledPin1  D0 // Chân điều khiển đèn LED (thường là GPIO2 trên ESP8266)
#define ledPin2  D1
#define ledPin3  D2
#define ledPin4  D3
//String dulieu = ""; // Biến để lưu trữ chuỗi lệnh

void setup() {
  // Khởi động Serial và đèn LED
  Serial.begin(9600);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  //digitalWrite(ledPin, LOW); // Tắt đèn LED lúc khởi động
}

void loop() {
    String dulieu = "";    // Biến để lưu trữ chuỗi lệnh
    // Kiểm tra nếu có dữ liệu từ cổng Serial
    while (Serial.available() > 0) 
      {
        char c = Serial.read();  // Đọc từng ký tự từ cổng Serial
        dulieu += c;
        delay(5);
      }
    if (dulieu == "b1") 
      {
        digitalWrite(ledPin1, HIGH);  // Bật đèn LED
        Serial.println("LED ON");    // Phản hồi lại trạng thái
      }
    if (dulieu == "t1") 
      {
        digitalWrite(ledPin1, LOW);   // Tắt đèn LED
        Serial.println("LED OFF");   // Phản hồi lại trạng thái
      }
    if (dulieu == "b2") 
      {
        digitalWrite(ledPin2, HIGH);  // Bật đèn LED
        Serial.println("LED ON");    // Phản hồi lại trạng thái
      }
    if (dulieu == "t2") 
      {
        digitalWrite(ledPin2, LOW);   // Tắt đèn LED
        Serial.println("LED OFF");   // Phản hồi lại trạng thái
      }
    if (dulieu == "b3") 
      {
        digitalWrite(ledPin3, HIGH);  // Bật đèn LED
        Serial.println("LED ON");    // Phản hồi lại trạng thái
      }
    if (dulieu == "t3") 
      {
        digitalWrite(ledPin3, LOW);   // Tắt đèn LED
        Serial.println("LED OFF");   // Phản hồi lại trạng thái
      }
    if (dulieu == "b4") 
      {
        digitalWrite(ledPin4, HIGH);  // Bật đèn LED
        Serial.println("LED ON");    // Phản hồi lại trạng thái
      }
    if (dulieu == "t4") 
      {
        digitalWrite(ledPin4, LOW);   // Tắt đèn LED
        Serial.println("LED OFF");   // Phản hồi lại trạng thái
      }
}

//   =========================================
// #define ledPin  2 // Chân điều khiển đèn LED (thường là GPIO2 trên ESP8266)
// String dulieu = ""; // Biến để lưu trữ chuỗi lệnh

// void setup() {
//   // Khởi động Serial và đèn LED
//   Serial.begin(9600);
//   pinMode(ledPin, OUTPUT);
//   digitalWrite(ledPin, LOW); // Tắt đèn LED lúc khởi động
// }

// void loop() {
//   // Kiểm tra nếu có dữ liệu từ cổng Serial
//   while (Serial.available() > 0) {
//     char c = Serial.read();  // Đọc từng ký tự từ cổng Serial
//     if (c == '\n') {
//       // Nếu gặp ký tự xuống dòng, xử lý chuỗi lệnh
//       if (dulieu == "b1") {
//         digitalWrite(ledPin, HIGH);  // Bật đèn LED
//         Serial.println("LED ON");    // Phản hồi lại trạng thái
//       }
//       else if (dulieu == "t1") {
//         digitalWrite(ledPin, LOW);   // Tắt đèn LED
//         Serial.println("LED OFF");   // Phản hồi lại trạng thái
//       }
//       // Xóa chuỗi lệnh sau khi xử lý
//       dulieu = "";
//     }
//     else 
//     {
//       dulieu += c;    // Nối các ký tự để tạo thành chuỗi lệnh
//     }
//   }
// }

