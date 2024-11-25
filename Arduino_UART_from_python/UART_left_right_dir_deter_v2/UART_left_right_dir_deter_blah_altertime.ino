#include <Arduino.h>
#include <SoftwareSerial.h>

// 定義模式一的pin腳
const int LED_R1 = 13;
const int LED_R2 = 12;
const int LED_L1 = 11;
const int LED_L2 = 10;
const int reward_L = 6;
const int reward_R = 7;

// 定義模式二的pin腳
const int pumpPin = 5;
const int ledPin = 9;

// 通用變數
char receivedChar; // 宣告字符變量
String time_duration;
long f_interval;
int trial = 0; // 計算當前的trial次數
int ans = 0;  // 暫存此次trial應給水的位置
bool isObstacle_M = false;  // IR sensor(中路)初始狀態
int mode = 0; // 模式選擇變量

SoftwareSerial MySerial(2, 3); // RX, TX

// 函數宣告
void blinkLed(int duration, int blinkDelay);
long calculateInterval(String input);
bool isNumeric(String str);
void left_right();
void stand_up();
void stand_up_fixed(long fixed_time);

void setup() {
  MySerial.begin(115200);
  Serial.begin(115200); // 初始化USB串口

  // 設置模式一的pin腳模式
  pinMode(reward_L, OUTPUT);
  pinMode(reward_R, OUTPUT);
  pinMode(LED_L1, OUTPUT);
  pinMode(LED_L2, OUTPUT);
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_R2, OUTPUT);

  // 設置模式二的pin腳模式
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // 初始化為非激活狀態
  digitalWrite(reward_L, HIGH);
  digitalWrite(reward_R, HIGH);
  digitalWrite(pumpPin, HIGH);
  digitalWrite(ledPin, LOW);

  Serial.println("Ready");
  Serial.println("Ready, enter '1' for Camera detection mode, '2' for Standup mode, '3' for Standup time fixed mode");

  // 等待用戶輸入模式選擇
  // before Serial.read, Serial.available()=0
  while (Serial.available() == 0) {
    // 等待用戶輸入
  }
  receivedChar = Serial.read();
  if (receivedChar == '1') {
    mode = 1;
    Serial.println("Camera detection mode selected");

  } else if (receivedChar == '2') {
    mode = 2;
    Serial.println("Standup mode selected");

  } else if (receivedChar == '3') {
    Serial.println("Enter time duration in seconds");
    // after Serial.read, Serial.available()=1
    while (Serial.read() >= 0) {
        // clear serialbuffer
    }
    // after clean up, Serial.available()=0
    while (Serial.available() == 0){

    }
    time_duration = Serial.readStringUntil('\n'); // 讀取完整字串
    time_duration.trim(); // 去除空白和換行符號
    f_interval = calculateInterval(time_duration); // 計算間隔時間
    if(f_interval != -1){
      mode = 3;
      Serial.println("Standup time fixed mode selected, ");
      Serial.print("Interval set to ");
      Serial.print(f_interval / 1000);
      Serial.println(" s");
    } else {
      mode = 2;
      Serial.println("Invalid input. Using default random interval.");
    }
  } else {
    Serial.println("Invalid mode selected. Defaulting to Camera detection mode.");
    mode = 1;
  }
}

void loop() {
  if (mode == 1) {
    left_right();
  } else if (mode == 2) {
    stand_up();
  } else if (mode == 3) {
    stand_up_fixed(f_interval);
  }
}

///////////////// left right mode /////////////////////
void left_right() {
  if (MySerial.available() > 0) {
    receivedChar = MySerial.read();

    if (receivedChar == 'R' || receivedChar == 'L' || receivedChar == 'D') {
      digitalWrite(reward_L, HIGH); // 關閉左側繼電器
      digitalWrite(reward_R, HIGH); // 關閉右側繼電器
      Serial.println("Sudden stop");
      return; // 立即退出函數
    }

    if (receivedChar == 'M' && !isObstacle_M) { // 只有在狀態由LOW變為HIGH時才處理
      isObstacle_M = true; // 更改狀態
      ans = random(1, 3); // 隨機選擇獎勵
      Serial.print("Ans: ");
      Serial.println(ans);
      trial += 1;
    } else {
      isObstacle_M = false; // 確保狀態能夠重置
    }

    if (isObstacle_M && (ans == 1 || ans == 2)) {
      int led1 = (ans == 1) ? LED_L1 : LED_R1;
      int led2 = (ans == 1) ? LED_L2 : LED_R2;
      int reward_pin = (ans == 1) ? reward_L : reward_R;

      digitalWrite(reward_pin, LOW); // 激活對應繼電器
      Serial.println((ans == 1) ? "Left LED On" : "Right LED On");

      for (int i = 1; i < 15; i++) {
        if (MySerial.available() > 0) {
          char tempChar = MySerial.read();
          if (tempChar == 'R' || tempChar == 'L' || tempChar == 'D') {
            break; // 如果在循環中收到'R', 'L', 'D'，立即中斷循環
          }
        }
        digitalWrite(led1, HIGH);
        digitalWrite(led2, LOW);
        delay(167);
        digitalWrite(led1, LOW);
        digitalWrite(led2, HIGH);
        delay(167);
        digitalWrite(led2, LOW);
      }

      Serial.println((ans == 1) ? "Left LED Off" : "Right LED Off");
      digitalWrite(reward_pin, HIGH); // 關閉繼電器
      isObstacle_M = false; // 重置狀態
    }
  }
}

///////////////// stand up mode /////////////////////
void stand_up() {
  // 啟動水泵和LED燈
  digitalWrite(pumpPin, LOW);  // 啟動水泵
  blinkLed(6, 500);            // 閃爍LED燈, 持續6秒, 每次閃爍間隔500毫秒

  // 停止水泵和LED燈
  digitalWrite(pumpPin, HIGH); // 停止水泵
  digitalWrite(ledPin, LOW);   // 確保LED燈關閉

  // 隨機間隔時間
  long interval = random(20000, 35000);  // 從20秒到35秒的隨機間隔
  delay(interval);
}

void stand_up_fixed(long fixed_time) {
  // 啟動水泵和LED燈
  digitalWrite(pumpPin, LOW);  // 啟動水泵
  blinkLed(6, 500);            // 閃爍LED燈, 持續6秒, 每次閃爍間隔500毫秒

  // 停止水泵和LED燈
  digitalWrite(pumpPin, HIGH); // 停止水泵
  digitalWrite(ledPin, LOW);   // 確保LED燈關閉

  // 間隔時間
  delay(fixed_time);
}

// 間隔時間
long calculateInterval(String input) {
  Serial.print("calc");
  if (isNumeric(input)) {
    return input.toInt() * 1000; // 將秒轉換為毫秒
  } else {
    return -1; // 無效輸入返回 -1
  }
}

// 檢查字串是否為數字
bool isNumeric(String str) {
  Serial.print("isNumeric");
  for (int i = 0; i < str.length(); i++) {
    if (!isDigit(str[i])) {
      return false;
    }
  }
  return true;
}

// 閃爍LED燈函數
void blinkLed(int duration, int blinkDelay) {
  long endTime = millis() + duration * 1000;
  while (millis() < endTime) {
    digitalWrite(ledPin, HIGH);  // 打開LED燈
    delay(blinkDelay);            // 等待
    digitalWrite(ledPin, LOW);   // 關閉LED燈
    delay(blinkDelay);            // 等待
  }
}
