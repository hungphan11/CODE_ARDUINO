#include <DHT.h>

#define ledPin1  D0 // Chân điều khiển đèn LED (thường là GPIO2 trên ESP8266)
#define ledPin2  D1
#define ledPin3  D2
#define ledPin4  D3
#define DHTTYPE DHT11
#define DHTPIN D4

// khỏi tạo cảm biến dht
DHT dht(DHTPIN, DHTTYPE);

//String dulieu = ""; // Biến để lưu trữ chuỗi lệnh
String s_flag;  // bit bắt đầu
String sender;
String receiver;
String command;  // lenh dieu khien
String data;     // du lieu
String e_flag;  // cờ nhớ 

unsigned long pre_time; // biến để tính thời gian delay

void setup() {
  // Khởi động Serial và đèn LED
  Serial.begin(9600);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  dht.begin();
  pre_time = millis();  // khi pre bằng millis() thì bắt đầu tính thời gian
  //digitalWrite(ledPin, LOW); // Tắt đèn LED lúc khởi động
}
void Boc_tach(String dl)
{
  s_flag = dl.charAt(0);
  sender = dl.charAt(1);
  receiver = dl.charAt(2);
  command = dl.charAt(3);
  data = dl.charAt(4);
  e_flag = dl.charAt(5);

  Serial.println("data receiver: "+dl);
  Serial.println("s_flag: "+s_flag);
  Serial.println("sender: "+sender);
  Serial.println("receiver: "+receiver);
  Serial.println("command: "+command);
  Serial.println("data: "+data); 
  Serial.println("e_flag: "+e_flag); 

  if(s_flag == "@"  &&  e_flag == "#")
  {
    if(receiver = "1") // kiểm tra dữ liệu gửi đến có phải cho thiết bị có địa chỉ là 1 o
    {
      if(command == "B")
      {
        if(data == "1")
        {
          digitalWrite(ledPin1, HIGH);
        }
        if(data == "2")
        {
          digitalWrite(ledPin2, HIGH);
        }
      }
      if(command == "T")
      {
        if(data == "1")
        {
          digitalWrite(ledPin1, LOW);
        }
        if(data == "2")
        {
          digitalWrite(ledPin2, LOW);
        }
      }
    }

    if(receiver = "2") // kiểm tra dữ liệu gửi đến có phải cho thiết bị có địa chỉ là 1 o
    {
      if(command == "B")
      {
        if(data == "3")
        {
          digitalWrite(ledPin3, HIGH);
        }
        if(data == "4")
        {
          digitalWrite(ledPin4, HIGH);
        }
      }
      if(command == "T")
      {
        if(data == "3")
        {
          digitalWrite(ledPin3, LOW);
        }
        if(data == "4")
        {
          digitalWrite(ledPin4, LOW);
        }
      }
    }
  }

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

    if(dulieu != "")
    {
      Boc_tach(dulieu);
      
    }
    // ======= truyền nhiệt độ lên 
    if(millis() - pre_time >=1000)
    {
      pre_time = millis();
      float temp = dht.readTemperature();
      float humi = dht.readHumidity();
      if(isnan(temp) || isnan(humi))
      {
        Serial.println("failed to read");
        return;
      }
      // In kết quả ra Serial Monitor
      // frame temp : @10AT + ...+ #

      //Serial.print("Temp: ");
      Serial.println("@10AT" + String(temp) + "#");
      //Serial.print(" °C");
      Serial.println("@10AH" + String(humi) + "#");
      //Serial.print("Humi: ");
      
      //Serial.println(" %");
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



