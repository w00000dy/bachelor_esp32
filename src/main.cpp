#include <Arduino.h>
#include <BleCombo.h>

#define LOG_TAG_SC "Serial Controller"
#define TASK_STACK_SIZE 4096

bool connected = false;
TaskHandle_t serialController;

void serialControl(void* parameter) {
    String command = "";
    for (;;) {
        if (Serial.available()) {
            int c = Serial.read();
#if CORE_DEBUG_LEVEL >= 3
            Serial.write(c);
#endif
            command += (char)c;
            if (c == '\x1B') {  // ESC
                Serial.println("Restarting...");
                ESP.restart();
            } else if (c == ';') {
                ESP_LOGI(LOG_TAG_SC, "Core: %d", xPortGetCoreID());
                ESP_LOGI(LOG_TAG_SC, "Stack: %d / %d (%d%%)", uxTaskGetStackHighWaterMark(NULL), TASK_STACK_SIZE, uxTaskGetStackHighWaterMark(NULL) * 100 / TASK_STACK_SIZE);
                ESP_LOGI(LOG_TAG_SC, "portTICK_PERIOD_MS: %d", portTICK_PERIOD_MS);
            }
            if (command[0] == '`') {
                if (c == '\n') {
                    // count the number of commas in the command
                    size_t commaCount = 0;
                    for (size_t i = 0; i < command.length(); i++) {
                        if (command[i] == ',') {
                            commaCount++;
                        }
                    }

                    command = command.substring(1);
                    command.replace("\r", "");
                    command.replace("\n", "");
                    command.trim();
                    ESP_LOGI(LOG_TAG_SC, "Command: %s", command.c_str());

                    const int commaIndex = command.indexOf(',');
                    if (commaCount == 0) {
                        if (command == "KEY_LEFT_SHIFT_DOWN") {
                            Keyboard.press(KEY_LEFT_SHIFT);
                        } else if (command == "KEY_LEFT_SHIFT_UP") {
                            Keyboard.release(KEY_LEFT_SHIFT);
                        } else if (command == "KEY_LEFT_CTRL_DOWN") {
                            Keyboard.press(KEY_LEFT_CTRL);
                        } else if (command == "KEY_LEFT_CTRL_UP") {
                            Keyboard.release(KEY_LEFT_CTRL);
                        } else if (command == "KEY_LEFT_ALT_DOWN") {
                            Keyboard.press(KEY_LEFT_ALT);
                        } else if (command == "KEY_LEFT_ALT_UP") {
                            Keyboard.release(KEY_LEFT_ALT);
                        } else if (command == "KEY_RIGHT_ALT_DOWN") {
                            Keyboard.press(KEY_RIGHT_ALT);
                        } else if (command == "KEY_RIGHT_ALT_UP") {
                            Keyboard.release(KEY_RIGHT_ALT);
                        } else if (command == "KEY_DELETE") {
                            Keyboard.write(KEY_DELETE);
                        } else if (command == "KEY_END") {
                            Keyboard.write(KEY_END);
                        } else if (command == "KEY_UP_ARROW") {
                            Keyboard.write(KEY_UP_ARROW);
                        } else if (command == "KEY_DOWN_ARROW") {
                            Keyboard.write(KEY_DOWN_ARROW);
                        } else if (command == "KEY_LEFT_ARROW") {
                            Keyboard.write(KEY_LEFT_ARROW);
                        } else if (command == "KEY_RIGHT_ARROW") {
                            Keyboard.write(KEY_RIGHT_ARROW);
                        } else if (command == "MOUSE_LEFT_DOWN") {
                            Mouse.press(MOUSE_LEFT);
                        } else if (command == "MOUSE_LEFT_UP") {
                            Mouse.release(MOUSE_LEFT);
                        } else if (command == "MOUSE_RIGHT_DOWN") {
                            Mouse.press(MOUSE_RIGHT);
                        } else if (command == "MOUSE_RIGHT_UP") {
                            Mouse.release(MOUSE_RIGHT);
                        } else {
                            ESP_LOGE(LOG_TAG_SC, "Invalid Command");
                        }
                    } else if (commaCount == 1) {
                        String xStr = command.substring(0, commaIndex);
                        String yStr = command.substring(commaIndex + 1);
                        int x = 0;
                        int y = 0;
                        ESP_LOGD(LOG_TAG_SC, "X: '%s', Y: '%s'", xStr.c_str(), yStr.c_str());
                        if (xStr != "0") {
                            x = xStr.toInt();
                            if (!x) {
                                ESP_LOGE(LOG_TAG_SC, "%s (X) is not an integer", xStr.c_str());
                            }
                        }
                        if (yStr != "0") {
                            y = yStr.toInt();
                            if (!y) {
                                ESP_LOGE(LOG_TAG_SC, "%s (Y) is not an integer", yStr.c_str());
                            }
                        }

                        if (x >= -128 && x <= 127 && y >= -128 && y <= 127) {
                            ESP_LOGI(LOG_TAG_SC, "Moving mouse to (%d, %d)", x, y);
                            Mouse.move(x, y);
                        } else {
                            Serial.println("One of the mouse values is out of range. Received command: " + command);
                        }
                    } else if (commaCount == 2) {
                        const int comma2Index = command.indexOf(',', commaIndex + 1);
                        String xStr = command.substring(0, commaIndex);
                        String yStr = command.substring(commaIndex + 1, comma2Index);
                        String wheelStr = command.substring(comma2Index + 1);
                        int x = 0;
                        int y = 0;
                        int wheel = 0;
                        ESP_LOGD(LOG_TAG_SC, "X: '%s', Y: '%s', Wheel: '%s'", xStr.c_str(), yStr.c_str(), wheelStr.c_str());
                        if (xStr != "0") {
                            x = xStr.toInt();
                            if (!x) {
                                ESP_LOGE(LOG_TAG_SC, "%s (X) is not an integer", xStr.c_str());
                            }
                        }
                        if (yStr != "0") {
                            y = yStr.toInt();
                            if (!y) {
                                ESP_LOGE(LOG_TAG_SC, "%s (Y) is not an integer", yStr.c_str());
                            }
                        }
                        if (wheelStr != "0") {
                            wheel = wheelStr.toInt();
                            if (!wheel) {
                                ESP_LOGE(LOG_TAG_SC, "[Wheel] is not an integer");
                            }
                        }

                        if (x >= -128 && x <= 127 && y >= -128 && y <= 127 && wheel >= -128 && wheel <= 127) {
                            ESP_LOGI(LOG_TAG_SC, "Moving mouse to (%d, %d, %d)", x, y, wheel);
                            Mouse.move(x, y, wheel);
                        } else {
                            Serial.println("One of the mouse values is out of range. Received command: " + command);
                        }
                    } else {
                        ESP_LOGE(LOG_TAG_SC, "Invalid Mouse Command");
                    }

                    command = "";
                }
            } else {
                Keyboard.write(c);
                command = "";
            }
        }
        vTaskDelay(1);
    }
}

void initializeBluetoothPairing() {
    Keyboard.begin();
    Mouse.begin();

    Serial.println("The Bluetooth Device Is Ready To Pair");
}

void setup() {
    Serial.begin(115200);

    ESP_LOGD("Setup", "Core: %d", xPortGetCoreID());

    xTaskCreate(serialControl, "Serial Contr.", TASK_STACK_SIZE, NULL, 1, &serialController);

    initializeBluetoothPairing();
}

void loop() {
    if (Keyboard.isConnected() && !connected) {
        connected = true;
        Serial.println("Connected");
    } else if (!Keyboard.isConnected() && connected) {
        connected = false;
        Serial.println("Disconnected");
        Keyboard.end();
        Mouse.end();
        initializeBluetoothPairing();
    }

    delay(1);
}