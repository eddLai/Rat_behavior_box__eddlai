#include <Arduino.h>
#include <SoftwareSerial.h>
// 對於馬達繼電器，LOW是開啟
// 面向兩個LED，右側為右側

char receivedChar; // 宣告字符变量
int trial = 0; // 计算当前的trial次数
int ans = 0;  // 暂存此次trial应给水的位置
bool isObstacle_M = false;  // IR sensor(中路)初始状态

SoftwareSerial MySerial(2, 3); // RX, TX
int LED_R1 = 13;
int LED_R2 = 12;
int LED_L1 = 11;
int LED_L2 = 10;
int reward_L = 6;
int reward_R = 7;

void setup() {
  MySerial.begin(115200);
  Serial.begin(115200); // 初始化USB串口
  pinMode(reward_L, OUTPUT);
  pinMode(reward_R, OUTPUT);
  pinMode(LED_L1, OUTPUT);
  pinMode(LED_L2, OUTPUT);
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_R2, OUTPUT);

  digitalWrite(reward_L, HIGH); // 初始化为非激活状态
  digitalWrite(reward_R, HIGH); // 初始化为非激活状态

  Serial.println("Ready");
  Serial.println("Ready, enter '1' for Camera detection mode, '2' for Standup mode");
}

void loop() {
  if (MySerial.available() > 0) {
    receivedChar = MySerial.read();

    if (receivedChar == 'R' || receivedChar == 'L' || receivedChar == 'D') {
      digitalWrite(reward_L, HIGH); // 关闭左侧继电器
      digitalWrite(reward_R, HIGH); // 关闭右侧继电器
      Serial.println("Sudden stop");
      return; // 立即退出loop()函数
    }

    if (receivedChar == 'M' && !isObstacle_M) { // 只有在状态由LOW变为HIGH时才处理
      isObstacle_M = true; // 更改状态
      ans = random(1, 3); // 随机选择奖励
      Serial.print("Ans: ");
      Serial.println(ans);
      trial += 1;
    } else {
      isObstacle_M = false; // 确保状态能够重置
    }

    if (isObstacle_M && (ans == 1 || ans == 2)) {
      int led1 = (ans == 1) ? LED_L1 : LED_R1;
      int led2 = (ans == 1) ? LED_L2 : LED_R2;
      int reward_pin = (ans == 1) ? reward_L : reward_R;

      digitalWrite(reward_pin, LOW); // 激活对应继电器
      Serial.println((ans == 1) ? "Left LED On" : "Right LED On");

      for (int i = 1; i < 15; i++) {
        if (MySerial.available() > 0) {
          char tempChar = MySerial.read();
          if (tempChar == 'R' || tempChar == 'L' || tempChar == 'D') {
            break; // 如果在循环中收到'R', 'L', 'D'，立即中断循环
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
      digitalWrite(reward_pin, HIGH); // 关闭继电器
      isObstacle_M = false; // 重置状态
    }
  }
}