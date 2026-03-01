#include <DHT.h>
#include <TFT_eSPI.h>
#include <SlideMeter.h>

#define DHTPIN 5
#define DHTTYPE DHT11
#define QUEUE_SIZE 5

typedef struct sensor_data {
  float humidity;
  float temperature;
  float heat_index;
} sensor_data;

TaskHandle_t read_temperature_handle = NULL;
TaskHandle_t display_temperature_handle = NULL;

SemaphoreHandle_t serialSem;

QueueHandle_t tftQueue = NULL;

DHT dht(DHTPIN, DHTTYPE);

TFT_eSPI tft = TFT_eSPI();

SlideMeter tempMeter(tft, "TEMP", 70, 50, 70, 170, 10.0, 40.0);
SlideMeter humiMeter(tft, "HUM", 200, 50, 80, 200, 50.0, 100.0);
SlideMeter heatMeter(tft, "HEAT", 330, 50, 100, 260, 10.0, 40.0);

void setup() {
  Serial.begin(115200);
  while (! Serial); // Wait untilSerial is ready - Leonardo
  Serial.println("Serial port connected.");   

  tft.begin();
  tft.init();
  tft.fillScreen(TFT_LIGHTGREY);
  tft.setRotation(1);

  serialSem = xSemaphoreCreateMutex();
  if (serialSem==NULL){
    Serial.println("xSemaphoreCreateMutex returned NULL");
    while(1);
  }

  tftQueue = xQueueCreate(QUEUE_SIZE, sizeof(sensor_data));
  if (tftQueue == NULL) {
    Serial.println("Failed to create Oled queue!");
    while (1);
  }
  
  dht.begin();

  delay(2000);

  tempMeter.draw();
  humiMeter.draw();
  heatMeter.draw();

  tempMeter.updateReading();
  humiMeter.updateReading();
  heatMeter.updateReading();

  delay(2000);

  xTaskCreatePinnedToCore(read_temperature, "read_temperature", 4000, NULL, 1, &read_temperature_handle, 0);
  Serial.println("Read temperature task created.");

  xTaskCreatePinnedToCore(display_temperature, "display_temperature", 4000, NULL, 1, &display_temperature_handle, 0);
  Serial.println("Display temperature task created.");

  delay(100);
}

void loop() {

}

void read_temperature(void *parameter) {

  for (;;) {

    sensor_data my_temp_Data;
    my_temp_Data.humidity = dht.readHumidity();
    my_temp_Data.temperature = dht.readTemperature();

    if (isnan(my_temp_Data.humidity) || isnan(my_temp_Data.temperature)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      xSemaphoreGive(serialSem);
      return;
    }

    my_temp_Data.heat_index = dht.computeHeatIndex(my_temp_Data.temperature, my_temp_Data.humidity, false);

    Serial.printf("Temperature: %.2f C\n", my_temp_Data.temperature);
    Serial.printf("Humidity: %.2f %\n", my_temp_Data.humidity);
    Serial.printf("Heat Index: %.2f C\n", my_temp_Data.heat_index);

    xQueueSend(tftQueue, &my_temp_Data, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void display_temperature(void *parameter) {

  for (;;) {

    sensor_data my_temp_Data;
    if (xQueueReceive(tftQueue, &my_temp_Data, portMAX_DELAY)) {

      xSemaphoreTake(serialSem, portMAX_DELAY);

      tempMeter.updateReading(my_temp_Data.temperature);
      humiMeter.updateReading(my_temp_Data.humidity);
      heatMeter.updateReading(my_temp_Data.heat_index);

      xSemaphoreGive(serialSem);

    }

  }
}
