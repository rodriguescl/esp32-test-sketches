#include<LedMatrixProcessor.h>
#include <array>
#include <algorithm>
#include <ranges>

struct InterruptData {
  const touch_pad_t touch_pin;
  const uint8_t touch_gpio;
  TaskHandle_t task_handle;
};

const int queue_size = 10;

TaskHandle_t trigger_leds_handle = NULL;
TimerHandle_t go_to_sleep_timer = NULL;

QueueHandle_t interrupt_queue = NULL;
SemaphoreHandle_t task_semaphore = NULL;

const uint32_t task_delay = 25;
const int threshold = 70;

unsigned long last_interrupt_time = 0;
const unsigned long debounce_delay = 500;
unsigned long timer_delay = 20000;

BaseType_t return_value;

std::array<InterruptData, 4> touch_pins = { 
                                            InterruptData { touch_pad_t::TOUCH_PAD_NUM5, 12, NULL },
                                            InterruptData { touch_pad_t::TOUCH_PAD_NUM4, 13, NULL },
                                            InterruptData { touch_pad_t::TOUCH_PAD_NUM3, 15, NULL },
                                            InterruptData { touch_pad_t::TOUCH_PAD_NUM6, 14, NULL } 
                                          };

LedMatrixProcessor matrix_processor;

void IRAM_ATTR handleTouch(void* arg) {
  uint32_t current_time = millis();
  if (current_time - last_interrupt_time < debounce_delay) {
    return;
  }
  last_interrupt_time = current_time;
  InterruptData* interrupt_data = (InterruptData*)arg;
  Serial.printf("Interrution raised by touch on pin %d\n", interrupt_data->touch_pin);
  xQueueSend(interrupt_queue, &interrupt_data->task_handle, portMAX_DELAY);  // Send to queue
  int num_messages = (int) uxQueueMessagesWaiting(interrupt_queue);
  Serial.printf("Number of Messages in the queue = %d\n", num_messages);
}

void TimerCallback(TimerHandle_t xTimer) {
  Serial.println("Time is up! Going to sleep after the current task finishes.");
  // Clean up
  std::ranges::for_each(touch_pins, [](InterruptData& p) { touchDetachInterrupt(p.touch_gpio); });
  int num_messages = (int) uxQueueMessagesWaiting(interrupt_queue);
  Serial.println("Waiting queue to be empty...");
  while(num_messages > 0) {
    Serial.printf("Number of messages still in the queue = %d\n", num_messages);
    vTaskDelay(pdMS_TO_TICKS(300));
    num_messages = (int) uxQueueMessagesWaiting(interrupt_queue);
  }
  Serial.println("Queue empty. Proceeding...");
  Serial.println("Waiting any pending task to complete...");
  while(xSemaphoreGetMutexHolder(task_semaphore) != NULL) {
    vTaskDelay(pdMS_TO_TICKS(300));
  }
  Serial.println("No task running. Proceeding...");  
  vTaskDelete(trigger_leds_handle);
  vQueueDelete(interrupt_queue);
  std::ranges::for_each(touch_pins, [](InterruptData& p) { vTaskDelete(p.task_handle); });
  vSemaphoreDelete(task_semaphore);
  xTimerDelete(go_to_sleep_timer, portMAX_DELAY);
  matrix_processor.clear();

  Serial.println("Going to sleep now.");

  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  print_wakeup_reason(wakeup_reason);
  uint16_t local_touch_pin = touch_pad_t::TOUCH_PAD_MAX;
  if(wakeup_reason == esp_sleep_wakeup_cause_t::ESP_SLEEP_WAKEUP_TOUCHPAD) {
    local_touch_pin = esp_sleep_get_touchpad_wakeup_status();
    local_touch_pin = log(local_touch_pin)/log(2);
    print_wakeup_touchpad(local_touch_pin);
    Serial.printf("Waking up due to touch on pin = %d\n", local_touch_pin);
  }

  interrupt_queue = xQueueCreate(queue_size, sizeof(TaskHandle_t));
  if (interrupt_queue == NULL) {
    Serial.println("Failed to create interrupt_queue queue!");
    while (1);
  }

  task_semaphore = xSemaphoreCreateMutex();
  if (task_semaphore == NULL){
    Serial.println("xSemaphoreCreateMutex returned NULL");
    while(1);
  }

  go_to_sleep_timer = xTimerCreate("go_to_sleep_timer", pdMS_TO_TICKS(timer_delay), pdFALSE, NULL, TimerCallback);
  if (go_to_sleep_timer == NULL) {
    Serial.println("Failed to create timer!");
    while (1);
  }

  matrix_processor.begin();
  delay(500);
  
  xTaskCreatePinnedToCore(fill_led, "fill_led", 4000, NULL, 1, &(touch_pins[0].task_handle), 0);
  xTaskCreatePinnedToCore(move_dot, "move_dot", 4000, NULL, 1, &(touch_pins[1].task_handle), 0);
  xTaskCreatePinnedToCore(fill_led_2, "fill_led_2", 4000, NULL, 1, &(touch_pins[2].task_handle), 0);
  xTaskCreatePinnedToCore(random_fill, "random_fill", 4000, NULL, 1, &(touch_pins[3].task_handle), 1);
  
  xTaskCreatePinnedToCore(trigger_leds, "trigger_leds", 4000, NULL, 1, &trigger_leds_handle, 1);
  delay(100); // Ideally we would check to see when the tasks are ready
  
  std::ranges::for_each(touch_pins, [](InterruptData& p) { touchAttachInterruptArg(p.touch_gpio, handleTouch, &p, threshold); });
  std::ranges::for_each(touch_pins, [](InterruptData& p) { touchSleepWakeUpEnable(p.touch_gpio, threshold); });

  if(local_touch_pin != touch_pad_t::TOUCH_PAD_MAX) {
    auto it = std::find_if(touch_pins.begin(), touch_pins.end(), [&](InterruptData p) { return (p.touch_pin == local_touch_pin); });
    if (it != touch_pins.end()) {
      Serial.printf("Waking with touch pin  %d\n", local_touch_pin);
      xQueueSend(interrupt_queue, &(*it).task_handle, portMAX_DELAY);
    }
  }

  xTimerStart(go_to_sleep_timer, portMAX_DELAY);
}

void loop() {}

void trigger_leds(void *parameter) {
  for (;;) {
    TaskHandle_t task_handle;
    if (xQueueReceive(interrupt_queue, &task_handle, portMAX_DELAY)) {
        xTimerReset(go_to_sleep_timer, portMAX_DELAY);
        TickType_t expiry_time = xTimerGetExpiryTime(go_to_sleep_timer);
        TickType_t current_time = xTaskGetTickCount();
        Serial.printf("Current time = %d, Expiry time = %d\n", current_time, expiry_time);
        return_value = xTaskNotifyGive(task_handle);
    }
    return_value = ulTaskNotifyTake(0, portMAX_DELAY);
  }
}

void fill_led(void *parameter) {
  for(;;) {
    Serial.println("Task fill_led waiting green light");
    return_value = ulTaskNotifyTake(0, portMAX_DELAY);
    Serial.println("Task fill_led notified");
    xSemaphoreTake(task_semaphore, portMAX_DELAY);
    fill_move_leds(false);
    vTaskDelay(pdMS_TO_TICKS(1000));
    matrix_processor.clear();
    xSemaphoreGive(task_semaphore);
    return_value = xTaskNotifyGive(trigger_leds_handle);
  }
}

void move_dot(void *parameter) {
  for(;;) {
    Serial.println("Task move_dot waiting green light");
    return_value = ulTaskNotifyTake(0, portMAX_DELAY);
    Serial.println("Task move_dot notified");
    xSemaphoreTake(task_semaphore, portMAX_DELAY);
    fill_move_leds(true);
    vTaskDelay(pdMS_TO_TICKS(1000));
    matrix_processor.clear();
    xSemaphoreGive(task_semaphore);
    return_value = xTaskNotifyGive(trigger_leds_handle);
  }
}

void fill_led_2(void *parameter) {
  for(;;) {
    Serial.println("Task fill_led_2 waiting green light");
    return_value = ulTaskNotifyTake(0, portMAX_DELAY);
    Serial.println("Task fill_led_2 notified");
    xSemaphoreTake(task_semaphore, portMAX_DELAY);
    matrix_processor.clear();
    for(short k = 0; k < 2; k++) {
      uint8_t col = 0;
      while(col < 8) {
        for(short row = 0; row < 8; row++) {
          matrix_processor.setLedState(row, col, (k == 0));
          vTaskDelay(pdMS_TO_TICKS(task_delay));
        }
        col++;
        for(short row = 7; row >= 0; row--) {
          matrix_processor.setLedState(row, col, (k == 0));
          vTaskDelay(pdMS_TO_TICKS(task_delay));
        }
        col++;
      }
    }
    matrix_processor.clear();
    vTaskDelay(pdMS_TO_TICKS(1000));
    xSemaphoreGive(task_semaphore);
    return_value = xTaskNotifyGive(trigger_leds_handle);
  }
}

void random_fill(void *parameter) {
  for(;;) {
    Serial.println("Task random_fill waiting green light");
    return_value = ulTaskNotifyTake(0, portMAX_DELAY);
    Serial.println("Task random_fill notified");
    xSemaphoreTake(task_semaphore, portMAX_DELAY);
    matrix_processor.clear();
    uint32_t attempts = 0;
    while(!matrix_processor.isLedMatrixFull() && attempts < 1000) {
      uint8_t row = random(8);
      uint8_t col = random(8);
      matrix_processor.setLedState(row, col, true);
      vTaskDelay(pdMS_TO_TICKS(task_delay));
    }
    for(short j = 0; j < 3; j++) {
      vTaskDelay(pdMS_TO_TICKS(700));
      matrix_processor.off();
      vTaskDelay(pdMS_TO_TICKS(700));
      matrix_processor.on();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    attempts = 0;
    while(!matrix_processor.isLedMatrixClear() && attempts < 1000) {
      uint8_t row = random(8);
      uint8_t col = random(8);
      matrix_processor.setLedState(row, col, false);
      vTaskDelay(pdMS_TO_TICKS(task_delay));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    matrix_processor.clear();
    xSemaphoreGive(task_semaphore);
    return_value = xTaskNotifyGive(trigger_leds_handle);
  }
}

void fill_move_leds(bool erase_previous_led) {
  uint8_t current_row = 0, current_col = 0;
  uint8_t prev_row = 0xFF, prev_col = 0xFF;
  uint8_t v_delta = 1, h_delta = 0;
  uint8_t num_rows = 8, num_cols = 8;    
  while(num_rows > 0 || num_cols > 0 ) {
    for(short i = 0; i < num_rows; i++) {
      if(prev_row != 0xFF && prev_col != 0xFF) {
        matrix_processor.setLedState(prev_row, prev_col, false);
      }          
      matrix_processor.setLedState(current_row, current_col, true);
      if(erase_previous_led) {
        prev_row = current_row;
        prev_col = current_col;
      }
      current_row = current_row + v_delta;
      vTaskDelay(pdMS_TO_TICKS(task_delay));
    }
    current_row = current_row - v_delta;
    h_delta = (h_delta == 0) ? 1 : -h_delta;
    current_col = current_col + h_delta;
    num_cols--;
    for(short i = 0; i < num_cols; i++) {
      if(prev_row != 0xFF && prev_col != 0xFF) {
        matrix_processor.setLedState(prev_row, prev_col, false);
      }          
      matrix_processor.setLedState(current_row, current_col, true);
      if(erase_previous_led) {
        prev_row = current_row;
        prev_col = current_col;
      }
      current_col = current_col + h_delta;
      vTaskDelay(pdMS_TO_TICKS(task_delay));
    }
    current_col = current_col - h_delta;
    v_delta = (v_delta == 0) ? 1 : -v_delta;
    current_row = current_row + v_delta;
    num_rows--;
  }
}


void print_wakeup_reason(esp_sleep_wakeup_cause_t reason) {
  switch (reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", reason); break;
  }
}

void print_wakeup_touchpad(uint16_t pin) {
  switch (pin) {
    case 0:  Serial.println("Touch detected on GPIO 4"); break;
    case 1:  Serial.println("Touch detected on GPIO 0"); break;
    case 2:  Serial.println("Touch detected on GPIO 2"); break;
    case 3:  Serial.println("Touch detected on GPIO 15"); break;
    case 4:  Serial.println("Touch detected on GPIO 13"); break;
    case 5:  Serial.println("Touch detected on GPIO 12"); break;
    case 6:  Serial.println("Touch detected on GPIO 14"); break;
    case 7:  Serial.println("Touch detected on GPIO 27"); break;
    case 8:  Serial.println("Touch detected on GPIO 33"); break;
    case 9:  Serial.println("Touch detected on GPIO 32"); break;
    default: Serial.println("Wakeup not by touchpad"); break;
  }
}
