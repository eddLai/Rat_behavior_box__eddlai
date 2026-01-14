#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>


// 定義模式一的pin腳
const int LED_R1 = 6;
const int LED_R2 = 7;
const int LED_L1 = 12;
const int LED_L2 = 13;

// 伺服馬達
Servo reward_L; // 10
Servo reward_R; // 9

// 定義模式二的pin腳
const int pumpPin = 5;
const int ledPin = 7;

// 通用變數
char receivedChar; // 宣告字符變量
String time_duration;
long f_interval;
int mode = 0; // 模式選擇變量
int trial = 0; // 計算當前的trial次數

bool isObstacle_M = false;  // IR sensor(中路)初始狀態
bool isObstacle_L = false;  // true when left correct response
bool isObstacle_R = false;  // true when right correct response
int ans = 0;  // 暫存此次trial應給水的位置
int reward_time = 300;
bool taskInProgress = false;

// 閃爍時仍可能延遲太久 導致下一個 L/R 指令在 loop 中被讀走但無法處理
// 非阻塞閃爍相關變數
bool blinking = false;
unsigned long blinkLastToggle = 0;
int blinkCount = 0;
int blinkState = 0;
int led1 = 0, led2 = 0;
const int maxBlinks = 15;

SoftwareSerial MySerial(2, 3); // RX, TX

// 函數宣告
void blinkLed(int duration, int blinkDelay);
long calculateInterval(String input);
bool isNumeric(String str);
void left_right();
void stand_up();
void stand_up_fixed(long fixed_time);
void resetState();
void processResponse(char response);

void setup() {
  MySerial.begin(115200);
  Serial.begin(115200); // 初始化USB串口

  // 設置模式一的pin腳模式
  reward_L.attach(10);  // left signal from pin 10
  reward_R.attach(9); // right signal from pin 9
  pinMode(LED_L1, OUTPUT);
  pinMode(LED_L2, OUTPUT);
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_R2, OUTPUT);

  // 設置模式二的pin腳模式
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // 初始化為非激活狀態
  reward_L.write(90); 
  reward_R.write(90); 
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
  // 累積指令進 buffer
  while (MySerial.available() > 0) {
    char rat_response = MySerial.read();
    processResponse(rat_response);  // 在閃爍中也會立即處理指令
  }

  // 處理 LED 閃爍狀態
  if (blinking) {
    unsigned long currentMillis = millis();
    if (currentMillis - blinkLastToggle >= 167) {
      blinkLastToggle = currentMillis;
      blinkState = !blinkState;

      if (blinkState) {
        digitalWrite(led1, HIGH);
        digitalWrite(led2, LOW);
      } else {
        digitalWrite(led1, LOW);
        digitalWrite(led2, HIGH);
        blinkCount++;

        if (blinkCount >= maxBlinks) {
          digitalWrite(led1, LOW);
          digitalWrite(led2, LOW);
          blinking = false;
          Serial.println("LED blinking finished. Waiting for 'L' or 'R'.");
        }
      }
    }
  }
}

// 獨立處理每個指令 避免訊號重疊
void processResponse(char response){
  // 如果收到 'D'，立即停止所有操作
  if (response == 'D') {
    reward_L.write(90); 
    reward_R.write(90); 
    Serial.println("Sudden stop received");
    blinking = false;
    resetState();
    return;
  }

  // 如果碰到紅點 M，隨機決定任務方向
  if (response == 'M') {
    if (taskInProgress) {
      Serial.println("Task already in progress. Ignoring duplicate 'M'.");
      return; // 如果已有任務正在進行，忽略重複的 M
    }

    resetState();
    isObstacle_M = true; // 更改狀態
    taskInProgress = true;
    ans = random(1, 3); // 隨機決定方向：1 是左邊，2 是右邊
    Serial.print("Red point detected. Task assigned: ");
    Serial.println((ans == 1) ? "Left" : "Right");

    // LED 閃爍 (啟動非阻塞閃爍)
    blinking = true;
    blinkLastToggle = millis();
    blinkCount = 0;
    blinkState = 0;
    led1 = (ans == 1) ? LED_L1 : LED_R1;
    led2 = (ans == 1) ? LED_L2 : LED_R2;
    return;
  }

  // 接收到 L / R 回應
  if (isObstacle_M) {
    if (response == 'R' && ans == 2 && !isObstacle_R) {
      // 正確走右邊
      Serial.println("Right signal received: Correct side");
      isObstacle_R = true;
      blinking = false;
      reward_R.write(60);
	    Serial.println("right reward start"); 
	    delay(reward_time);
	    reward_R.write(90); 
      Serial.println("right reward finish");
      resetState(); // 重置狀態
    } else if (response == 'L' && ans == 1 && !isObstacle_L) {
      // 正確走左邊
      Serial.println("Left signal received: Correct side");
      isObstacle_L = true;
      blinking = false;
      reward_L.write(114);
	    Serial.println("left reward start"); 
	    delay(reward_time);
	    reward_L.write(90); 
      Serial.println("left reward finish");
      resetState(); // 重置狀態
    } else if (response == 'R' && ans == 1) {
      // 走錯了，應該走左邊
      Serial.println("Right signal received: Wrong side, expected Left");
      blinking = false;
      resetState();
    } else if (response == 'L' && ans == 2) {
      // 走錯了，應該走右邊
      Serial.println("Left signal received: Wrong side, expected Right");
      blinking = false;
      resetState();
    }
  }
}

// 重置狀態
void resetState() {
  isObstacle_M = false;
  isObstacle_L = false;
  isObstacle_R = false;
  ans = 0;
  taskInProgress = false;
  blinking = false;
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
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
  // digitalWrite(pumpPin, LOW);  // 啟動水泵
  // blinkLed(6, 500);            // 閃爍LED燈, 持續6秒, 每次閃爍間隔500毫秒

  // // 停止水泵和LED燈
  // // digitalWrite(pumpPin, HIGH); // 停止水泵
  // digitalWrite(ledPin, LOW);   // 確保LED燈關閉

  // // 間隔時間
  // delay(fixed_time);

  while (Serial.available() > 0) {
    char c = Serial.read();

    // 忽略換行與空白
    if (c == '\n' || c == '\r' || c == ' ' || c == '\t') continue;

    if (c == '1') {
      // digitalWrite(ledPin, HIGH);
      Serial.println("LED ON");
      blinkLed(10, 500);            // 閃爍LED燈, 持續10秒, 每次閃爍間隔500毫秒
    } else if (c == '0') {
      digitalWrite(ledPin, LOW);
      Serial.println("LED OFF");
    } else {
      // 非 0/1 指令：忽略並回報
      Serial.print("Ignored input: ");
      Serial.println(c);
    }
  }

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