void setup() {
  Serial.begin(115200);

  // Tạo các luồng để in ra thông báo
  xTaskCreate(printTask1, "PrintTask1", 1000, NULL, 1, NULL);
  xTaskCreate(printTask2, "PrintTask2", 1000, NULL, 1, NULL);
  xTaskCreate(printTask3, "PrintTask3", 1000, NULL, 1, NULL);
}

void loop() {
  // Không cần làm gì trong loop() vì các luồng tự xử lý
}

void printTask1(void *parameter) {
  while (true) {
    Serial.println("Task 1: Hello from Task 1!");
    vTaskDelay(4000 / portTICK_PERIOD_MS);  // Tạm nghỉ 1 giây
  }
}

void printTask2(void *parameter) {
  while (true) {
    Serial.println("Task 2: Hello from Task 2!");
    vTaskDelay(8000 / portTICK_PERIOD_MS);  // Tạm nghỉ 2 giây
  }
}

void printTask3(void *parameter) {
  while (true) {
    Serial.println("Task 3: Hello from Task 3!");
    vTaskDelay(12000 / portTICK_PERIOD_MS);  // Tạm nghỉ 3 giây
  }
}
