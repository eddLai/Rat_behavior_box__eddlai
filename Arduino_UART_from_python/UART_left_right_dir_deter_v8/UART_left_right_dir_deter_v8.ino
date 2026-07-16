#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>

//----- 定義模式一的pin腳 -----
const int LED_R1 = 6; // 用於 Mode 1 的右側提示燈
const int LED_R2 = 7; 
const int LED_L1 = 12;
const int LED_L2 = 13;

// 伺服馬達
Servo reward_L; // 10 
Servo reward_R; // 9

// 三向筏角度控制
const int posClose = 0;   // 關門角度
const int posOpen = 180;  // 開門角度

int currentPosL = posClose; // 記錄左門當前角度 (initial)
int currentPosR = posClose; // 記錄右門當前角度 (initial)

//----- 定義模式二的pin腳 -----
const int pumpPin = 5;
const int ledPin = 7; // 用於 Mode 2 和 Mode 3 的給水提示燈(6,7)

//----- 通用變數 -----
char receivedChar; // 宣告字符變量
int mode = 0; // 模式選擇變量
int lastMode = 0; // 記錄上一次的模式，用於偵測模式切換
int trial = 0; // 計算當前的trial次數

bool isObstacle_M = false;  // IR sensor(中路)初始狀態
bool isObstacle_L = false;  // true when left correct response
bool isObstacle_R = false;  // true when right correct response
int ans = 0;  // 暫存此次trial應給水的位置
int reward_time = 300;
int reward_time_R = 450;
int reward_time_L = 150;
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
void left_right();
// void stand_up();
void manual_mode();
void drain_mode();
void long_locomotion();
void resetState();
void processResponse(char response);
void moveServoSmooth(Servo &s, int from, int to);

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

  // 初始化為非激活狀態（改為 180 度伺服馬達的初始關門位置）
  currentPosL = posClose; 
  currentPosR = posClose;
  reward_L.write(currentPosL);
  reward_R.write(currentPosR);
  digitalWrite(pumpPin, HIGH);
  digitalWrite(ledPin, LOW);

  // Serial.println(F("Ready"));
  // Serial.println(F("========================================="));
  Serial.println(F(" System Ready! Please select a mode: "));
  Serial.println(F(" Enter '1' : Camera detection mode"));
  Serial.println(F(" Enter '2' : Standup (Auto Interval) mode (Right Door)"));
  Serial.println(F(" Enter '3' : Manual Control mode"));
  Serial.println(F(" Enter '4' : Drain mode"));
  Serial.println(F(" Enter '5' : Long locomotion mode (L/R point -> reward)"));
  // Serial.println(F("========================================="));
  
  // 等待用戶輸入模式選擇
  // before Serial.read, Serial.available()=0
  while (Serial.available() == 0) {
    // 等待用戶輸入
  }
  receivedChar = Serial.read();
  if (receivedChar == '1') {
    mode = 1;
    Serial.println(F("-> [Mode 1] Camera detection mode selected"));

  } else if (receivedChar == '2') {
    mode = 2;
    Serial.println(F("-> [Mode 2] Standup mode selected"));
    Serial.println(F("    Random Time Interval (Auto), open only Right Door"));

  } else if (receivedChar == '3') {
    mode = 3;
    Serial.println(F("-> [Mode 3] Manual Control mode selected"));
    Serial.println(F("   Tutorial: Enter '1' to OPEN/BLINK, '0' to CLOSE/STOP)"));
  
  } else if (receivedChar == '4') {
    mode = 4;
    Serial.println(F("-> [Mode 4] Drain mode selected"));

  } else if (receivedChar == '5') {
    mode = 5;
    Serial.println(F("-> [Mode 5] Long locomotion mode selected"));
    Serial.println(F("    L point -> left reward, R point -> right reward"));

  } else {
    Serial.println(F("Invalid mode selected. Defaulting to Camera detection mode."));
    mode = 1;
  }
}

void loop() {
  // 偵測模式切換，在切換時處理伺服馬達
  if (mode != lastMode) {
    if (mode == 1 || mode == 5) {
      // 切換到需要雙門的模式時，重新連接伺服馬達並設為當前角度
      reward_L.attach(10);
      reward_R.attach(9);
      reward_L.write(currentPosL);
      reward_R.write(currentPosR);
    } else {
      // 切換到其他模式時 detach 伺服馬達以減少雜訊
      reward_L.detach();
      reward_R.detach();
    }
    lastMode = mode;
  }

  if (mode == 1) {
    left_right();
  // } 
  // else if (mode == 2) {
    // stand_up();
  } else if (mode == 3) {
    manual_mode();
  } else if (mode == 4) {
    drain_mode();
  } else if (mode == 5) {
    long_locomotion();
  }
}

///////////////// left right mode (Mode 1) /////////////////////
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
          Serial.println(F("LED blinking finished. Waiting for 'L' or 'R'."));
        }
      }
    }
  }
}

// 獨立處理每個指令 避免訊號重疊
void processResponse(char response){
  // 如果收到 'D'，立即停止所有操作
  if (response == 'D') {
    // op1-平滑關閉兩側的門
    moveServoSmooth(reward_L, currentPosL, posClose);
    moveServoSmooth(reward_R, currentPosR, posClose);
    // op2-瞬間關門
    // reward_L.write(posClose);
    // reward_R.write(posClose);

    // 更新當前角度紀錄
    currentPosL = posClose;
    currentPosR = posClose;

    Serial.println(F("Sudden stop received: Doors closed"));
    blinking = false;
    resetState();
    return;
  }

  // 如果碰到紅點 M 隨機決定任務方向
  if (response == 'M') {
    if (taskInProgress) {
      Serial.println(F("Task already in progress. Ignoring duplicate 'M'."));
      return; // 如果已有任務正在進行，忽略重複的 M
    }

    resetState();
    isObstacle_M = true; // 更改狀態
    taskInProgress = true;
    ans = random(1, 3); // 隨機決定方向: 1 是左邊、2 是右邊
    Serial.print(F("Red point detected. Task assigned: "));
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
      Serial.println(F("Right signal received: Correct side"));
      isObstacle_R = true;
      blinking = false;

      // 平滑開啟右門
      Serial.println(F("right door opening..."));
      moveServoSmooth(reward_R, currentPosR, posOpen);
      currentPosR = posOpen;
      
      delay(reward_time); // 等待給水/獎勵時間
      
      // 平滑關閉右門
      Serial.println(F("right door closing..."));
      moveServoSmooth(reward_R, currentPosR, posClose);
      currentPosR = posClose;
      
      Serial.println(F("right reward finish"));
      resetState(); // 重置狀態

    } else if (response == 'L' && ans == 1 && !isObstacle_L) {
      // 正確走左邊
      Serial.println(F("Left signal received: Correct side"));
      isObstacle_L = true;
      blinking = false;

      // 平滑開啟左門
      Serial.println(F("left door opening..."));
      moveServoSmooth(reward_L, currentPosL, posOpen);
      currentPosL = posOpen;
      
      delay(reward_time); // 等待給水/獎勵時間
      
      // 平滑關閉左門
      Serial.println(F("left door closing..."));
      moveServoSmooth(reward_L, currentPosL, posClose);
      currentPosL = posClose;
      
      Serial.println(F("left reward finish"));
      resetState();

    } else if (response == 'R' && ans == 1) {
      // 走錯了，應該走左邊
      Serial.println(F("Right signal received: Wrong side, expected Left"));
      blinking = false;
      resetState();
    } else if (response == 'L' && ans == 2) {
      // 走錯了，應該走右邊
      Serial.println(F("Left signal received: Wrong side, expected Right"));
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

// 平滑移動函式：讓馬達以指定的步進角度緩慢轉動，避免瞬間全速扯動機構
void moveServoSmooth(Servo &s, int from, int to) {
  if (from == to) {
    return; // 如果起點和終點一樣（門已經在目標位置），直接結束函式
  }
  
  if (from < to) { 
    // 若目標角度大於當前角度 (正轉)
    for (int a = from; a <= to; a += 5) {
      s.write(a);
      delay(10); 
    }
  } else { 
    // 若目標角度小於當前角度 (反轉)
    for (int a = from; a >= to; a -= 5) {
      s.write(a);
      delay(10);
    }
  }
}

///////////////// long locomotion mode (Mode 5) /////////////////////
// 碰到 L / R 點就直接給水（開對應側的門），不需要 M 觸發、不判斷對錯。
// 防重複機制：給一次水後上鎖(disarm)，必須走過黑點('D')才會重新武裝(rearm)。
// armed 為 L/R 共用的單一鎖；初始為 true，所以第一次碰 L/R 就會給水。
void long_locomotion() {
  static bool armed = true; // true=可觸發給水；false=已給水，等待黑點 'D' 解鎖

  while (MySerial.available() > 0) {
    char rat_response = MySerial.read();

    if (rat_response == 'D') {
      // 走過任一黑點(MD/LD/RD)：重新武裝，允許下一次 L/R 給水
      if (!armed) {
        armed = true;
        Serial.println(F("[Mode 5] Black point passed: rearmed"));
      }

    } else if (rat_response == 'L' && armed) {
      // 碰到左邊點且處於武裝狀態：開左門給水，然後上鎖
      armed = false; // 給水前先上鎖，阻塞期間湧入的重複 L/R 會被忽略
      Serial.println(F("[Mode 5] Left point reached: left reward (disarmed)"));
      moveServoSmooth(reward_L, currentPosL, posOpen);
      currentPosL = posOpen;

      delay(reward_time_L); // 給水/獎勵時間

      moveServoSmooth(reward_L, currentPosL, posClose);
      currentPosL = posClose;
      Serial.println(F("left reward finish"));

    } else if (rat_response == 'R' && armed) {
      // 碰到右邊點且處於武裝狀態：開右門給水，然後上鎖
      armed = false;
      Serial.println(F("[Mode 5] Right point reached: right reward (disarmed)"));
      moveServoSmooth(reward_R, currentPosR, posOpen);
      currentPosR = posOpen;

      delay(reward_time_R); // 給水/獎勵時間

      moveServoSmooth(reward_R, currentPosR, posClose);
      currentPosR = posClose;
      Serial.println(F("right reward finish"));
    }
    // 其他情況（已 disarmed 的 L/R、或未定義字元）：讀掉丟棄，不作動
  }
}

///////////////// stand up mode (Mode 2) /////////////////////
// void stand_up() {
//   reward_R.attach(9);

//   Serial.println(F("Stand-up: Right door opening..."));
//   moveServoSmooth(reward_R, currentPosR, posOpen);
//   currentPosR = posOpen;

//   // 閃爍LED燈, 持續6秒, 每次閃爍間隔500毫秒 (這 6 秒期間，右門會維持開啟狀態給水)
//   Serial.println(F("Blink every 500ms for 6sec"));
//   blinkLed(6, 500);            

//   Serial.println(F("Stand-up: Right door closing..."));
//   moveServoSmooth(reward_R, currentPosR, posClose);
//   currentPosR = posClose;

//   digitalWrite(ledPin, LOW);

//   // 5. 斷開右邊馬達以減少待機時的雜訊與抖動
//   reward_R.detach();

//   // 6. 隨機間隔時間
//   long interval = random(20000, 35000);  // 從20秒到35秒的隨機間隔
//   Serial.print(F("Stand-up: Waiting for next trial ("));
//   Serial.print(interval / 1000);
//   Serial.println(F(" s)..."));
//   delay(interval);
// }

///////////////// Manual Control mode (Mode 3) /////////////////////
void manual_mode() {
  // 使用靜態變數來記憶當前的狀態，這些變數在迴圈中不會被重置
  static bool isManualActive = false;     // 記錄目前是否處於「正在給水/閃燈」的狀態
  static unsigned long lastBlinkTime = 0; // 記錄上一次切換燈號的時間
  static bool ledState = LOW;             // 記錄目前的燈號是亮還是暗

  // 1. 處理你的鍵盤輸入
  while (Serial.available() > 0) {
    char c = Serial.read();

    // 忽略換行與空白
    if (c == '\n' || c == '\r' || c == ' ' || c == '\t') continue;

    if (c == '1') {
      // 只有在「本來是關閉狀態」時按 1 才作動，避免重複按 1 造成馬達抽動
      if (!isManualActive) {
        Serial.println(F("Manual Switch: Right door OPEN & Blinking START"));
        isManualActive = true;
        
        // 重新連接馬達並開門
        reward_R.attach(9);
        moveServoSmooth(reward_R, currentPosR, posOpen);
        currentPosR = posOpen;
        
        // 初始化閃爍狀態 (馬上亮燈)
        ledState = HIGH;
        digitalWrite(ledPin, ledState);
        lastBlinkTime = millis();
      }

    } else if (c == '0') {
      // 只有在「本來是開啟狀態」時按 0 才作動
      if (isManualActive) {
        Serial.println(F("Manual Switch: Right door CLOSE & Blinking STOP"));
        isManualActive = false;
        
        // 強制關閉 LED 燈
        ledState = LOW;
        digitalWrite(ledPin, LOW);
        
        // 確保馬達有連線，然後平滑關門
        reward_R.attach(9);
        moveServoSmooth(reward_R, currentPosR, posClose);
        currentPosR = posClose;
        
        // 關門後斷開馬達以減少雜訊
        reward_R.detach();
      }

    } else {
      // 非 0/1 指令：忽略並回報
      Serial.print(F("Ignored input: "));
      Serial.println(c);
    }
  }

  // 2. 非阻塞式持續閃燈邏輯 (只要狀態是啟動的，就會自動在背景閃爍)
  if (isManualActive) {
    // 檢查是否已經過了 500 毫秒
    if (millis() - lastBlinkTime >= 500) { 
      lastBlinkTime = millis();            // 更新時間
      ledState = !ledState;                // 反轉燈號狀態 (亮變暗、暗變亮)
      digitalWrite(ledPin, ledState);      // 寫入燈號
    }
  }
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

///////////////// drain mode (Mode 4) /////////////////////
void drain_mode() {
  // 使用 static 變數，確保提示選單只會印出一次
  static bool menuPrinted = false;

  if (!menuPrinted) {
    Serial.println(F("========================================="));
    Serial.println(F(" Drain mode activated."));
    Serial.println(F(" Enter 'O' to OPEN doors and start draining."));
    Serial.println(F(" Enter 'C' to CLOSE doors and detach motors."));
    Serial.println(F("========================================="));
    menuPrinted = true; // 標記為已列印
  }

  // 持續監聽鍵盤輸入
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // 忽略換行與空白符號
    if (cmd == '\n' || cmd == '\r' || cmd == ' ' || cmd == '\t') {
      return; 
    }

    // 輸入 'O' 或 'o' 開門
    if (cmd == 'O' || cmd == 'o') {
      // 重新連接馬達
      reward_L.attach(10);
      reward_R.attach(9);
      
      Serial.println(F("-> Doors OPENING... Water is draining."));
      moveServoSmooth(reward_L, currentPosL, posOpen);
      moveServoSmooth(reward_R, currentPosR, posOpen);
      currentPosL = posOpen;
      currentPosR = posOpen;
    } 
    // 輸入 'C' 或 'c' 關門
    else if (cmd == 'C' || cmd == 'c') {
      Serial.println(F("-> Doors CLOSING..."));
      moveServoSmooth(reward_L, currentPosL, posClose);
      moveServoSmooth(reward_R, currentPosR, posClose);
      currentPosL = posClose;
      currentPosR = posClose;
      
      // 關門後斷開馬達以減少雜訊
      reward_L.detach();
      reward_R.detach();
      Serial.println(F("-> Doors are closed and motors detached. Safe to idle."));
    } 
    // 輸入其他無效字元
    else {
      Serial.print(F("Invalid command: "));
      Serial.println(cmd);
    }
  }
}