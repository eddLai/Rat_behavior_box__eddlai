#include <Arduino.h>
#include <SoftwareSerial.h>

const int LED_R1 = 13;
const int LED_R2 = 12;
const int LED_L1 = 11;
const int LED_L2 = 10;
const int reward_L = 6;
const int reward_R = 7;

bool loopRunning = false;
int i = 1;

void setup() {
  Serial.begin(115200);
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_R2, OUTPUT);
  pinMode(reward_R, OUTPUT);
}

void loop() {
  // 檢查 Serial 是否有輸入
  if (Serial.available() > 0) {
    char inputChar = Serial.read();
    if (inputChar == '\n') { // 如果按下 Enter 鍵，改變迴圈狀態
      loopRunning = !loopRunning; // 切換運行狀態
      Serial.println(loopRunning);
      i = 1; // 重置計數器
    }
  }

  // 如果 loopRunning 為 true，進行動作
  if (loopRunning) {
    Serial.println("MOTOR ON");
    digitalWrite(reward_R, LOW);
    if (i < 15) {
      digitalWrite(LED_R1, HIGH);
      digitalWrite(LED_R2, LOW);
      digitalWrite(reward_R, HIGH);
      delay(167/2);
      digitalWrite(LED_R1, LOW);
      digitalWrite(LED_R2, HIGH);
      digitalWrite(reward_R, LOW);
      delay(167/2);
      digitalWrite(LED_R1, LOW);

      i++; // 增加計數器
    }
  }
  else {
      Serial.println("CLOSE");
      digitalWrite(reward_R, HIGH); // 關閉繼電器
      loopRunning = false; // 迴圈完成後停止
    }
}