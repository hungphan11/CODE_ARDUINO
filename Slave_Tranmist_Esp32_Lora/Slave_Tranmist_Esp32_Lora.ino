//    >>> Test Slave (1 or 2) ESP32 Lora Ra-02
//---------------------------------------- Include Library.
#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//---------------------------------------- Defines the DHT11 Pin and the DHT type.
#define DHTPIN      15
#define DHTTYPE     DHT11

// --------------------- OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --------------------- Soil moisture sensor pin
#define SOIL_MOISTURE_PIN 34

//---------------------------------------- Defines LED Pins.
#define LED_1_Pin   27
#define LED_2_Pin   25

//---------------------------------------- LoRa Pin / GPIO configuration.
#define ss 5
#define rst 14
#define dio0 2

// Initializes the DHT sensor (DHT11).
DHT dht11(DHTPIN, DHTTYPE);

//---------------------------------------- Variable declaration to hold incoming and outgoing data.
String Incoming = "";
String Message = "";             

//---------------------------------------- LoRa data transmission configuration
//byte LocalAddress = 0x02;       //--> address of this device (Slave 1)
byte LocalAddress = 0x01;     //--> address of this device (Slave 2)

byte Destination_Master = 0x10; //--> destination to send to Master (ESP32)

//---------------------------------------- Variable declarations for temperature and humidity values.
int h = 0;
float t = 0.0;
int soilMoisture = 0;
byte LED_1_State;  // trạng thái led
byte LED_2_State; 

//---------------------------------------- Millis / Timer to update temperature and humidity values from DHT11 sensor.
unsigned long previousMillis_UpdateDHT11 = 0;
const long interval_UpdateDHT11 = 2000;   // cập nhật mỗi 2s 

//---------------------------------------- Function to send LoRa data.
void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();             //--> start packet 
  LoRa.write(Destination);        //--> add destination address
  LoRa.write(LocalAddress);       //--> add sender address
  LoRa.write(Outgoing.length());  //--> add payload length
  LoRa.print(Outgoing);           //--> add payload
  LoRa.endPacket();               //--> finish packet and send it
}

//---------------------------------------- Function to receive LoRa data.
void onReceive(int packetSize) {
  if (packetSize == 0) 
    return;  //--> if there's no packet, return 

  //---------------------------------------- Read packet header bytes.
  int recipient = LoRa.read();        //--> recipient address 
  byte sender = LoRa.read();          //--> sender address
  byte incomingLength = LoRa.read();  //--> incoming msg length

  Incoming = "";

  //---------------------------------------- Get all incoming data.
  while (LoRa.available()) {
    Incoming += (char)LoRa.read();
  }

  //---------------------------------------- Check length for error.
  if (incomingLength != Incoming.length()) {
    Serial.println();
    Serial.println("error: message length does not match length");
    return; //--> skip rest of function
  }

  //---------------------------------------- Checks whether the incoming data or message is for this device.
  if (recipient != LocalAddress) {
    Serial.println();
    Serial.println("This message is not for me.");
    return; //--> skip rest of function
  }

  //---------------------------------------- If message is for this device, print details.
  Serial.println();
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Message: " + Incoming);

  //---------------------------------------- Calls the Processing_incoming_data() subroutine.
  Processing_incoming_data();
}

//---------------------------------------- Function to process incoming data and send data to Master.
void Processing_incoming_data() {
  if (Incoming == "SDS1") {  // SDS1 or SDS2 sent from Master
    digitalWrite(LED_1_Pin, !digitalRead(LED_1_Pin));
    digitalWrite(LED_2_Pin, !digitalRead(LED_2_Pin));
   
    LED_1_State = digitalRead(LED_1_Pin);
    LED_2_State = digitalRead(LED_2_Pin);

    Message = "";
    Message = String(h) + ":" + String(t) + ":" + String(soilMoisture) + ":" + String(LED_1_State) + ":" + String(LED_2_State);

    Serial.println();
    Serial.println("Send message to Master");
    Serial.print("Message: ");
    Serial.println(Message);
   
    sendMessage(Message, Destination_Master);
  }
}

//---------------------------------------- Function to update OLED display
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);

  display.println("Temp:      " + String(t) + " C");
  display.println("Humidity:  " + String(h) + " %");
  display.println("Soil Moi:  " + String(soilMoisture)+ " %");

  display.display();
}

//---------------------------------------- VOID SETUP
void setup() {
  Serial.begin(115200);

  pinMode(LED_1_Pin, OUTPUT);
  pinMode(LED_2_Pin, OUTPUT);

  delay(100);

  digitalWrite(LED_1_Pin, HIGH);
  digitalWrite(LED_2_Pin, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();

  dht11.begin();

  //---------------------------------------- Settings and start LoRa Ra-02.
  LoRa.setPins(ss, rst, dio0);

  Serial.println();
  Serial.println("Start LoRa init...");
  if (!LoRa.begin(433E6)) {             // Initialize radio at 433 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // If failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
}

//---------------------------------------- VOID LOOP
void loop() {
  //---------------------------------------- Millis / Timer to update temperature and humidity values from the DHT11 sensor every 2 seconds.
  unsigned long currentMillis_UpdateDHT11 = millis();
  
  if (currentMillis_UpdateDHT11 - previousMillis_UpdateDHT11 >= interval_UpdateDHT11) {
    previousMillis_UpdateDHT11 = currentMillis_UpdateDHT11;

    // Reading humidity
    h = dht11.readHumidity();
    // Read temperature as Celsius 
    t = dht11.readTemperature();

    // Read soilMoisture
    int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
    // Convert the read value to soil moisture percentage (assuming 0-100%)
    soilMoisture = map(soilMoistureValue, 0, 4095, 0, 100);
  
    if (isnan(h) || isnan(t) || isnan(soilMoisture)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      h = 0;
      t = 0.0;
      soilMoisture = 0;
    }
    updateDisplay();
  }
  
  //---------------------------------------- Check and process received LoRa packets.
  onReceive(LoRa.parsePacket());
}
