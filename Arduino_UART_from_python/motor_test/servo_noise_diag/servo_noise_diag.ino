// `    `/*
//  * servo_noise_diag.ino
//  * -------------------------------------------------------------
//  * 伺服馬達異音診斷 sketch（不用拆機構就能測）
//  *
//  * 目的：在馬達黏死、無法拆離機構的情況下，用「軟體隔離 + 避開極端角度」
//  *       來判斷異音是【機構頂死/崩齒】還是【電源/馬達本身】的問題。
//  *
//  * 接線（沿用主程式）：
//  *   reward_L -> pin 10
//  *   reward_R -> pin 9
//  *
//  * 用法：燒錄後打開 Serial Monitor (115200)，耳朵貼近馬達，
//  *       依 A / B / C 三段分別聽聲音，對照下面判讀表。
//  *
//  * 判讀：
//  *   A(detach) 也叫        -> 外部電源/干擾（少見）
//  *   A 靜、B(90°) 叫        -> 電源不穩 或 馬達受損
//  *   A/B 靜、只有極端角叫   -> 機構頂死 或 塑膠齒被磨花（改角度範圍即可，不用拆）
//  *
//  * 注意：本測試「刻意避開」0°/180° 兩個極端，避免再度堵轉傷害機構。
//  * -------------------------------------------------------------
//  */

#include <Servo.h>

Servo reward_L; // pin 10
Servo reward_R; // pin 9

const int PIN_L = 10;
const int PIN_R = 9;

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Servo Noise Diagnostic ==="));
  Serial.println(F("Listen close to each stage: A / B / C"));
}

// 已知：A(detach)不叫、B(兩顆90°)叫、C(掃描)叫
// 目標：分開「供電不足」與「機構/馬達」→ 一次只驅動一顆做比較
void loop() {
  // ---- D) 只驅動 L，保持 90°（R 完全斷開）----
  reward_R.detach();
  reward_L.attach(PIN_L);
  reward_L.write(90);
  Serial.println(F("D: L only @90 (R detached) -> listen (4s)"));
  delay(4000);

  // ---- E) 只驅動 R，保持 90°（L 完全斷開）----
  reward_L.detach();
  reward_R.attach(PIN_R);
  reward_R.write(90);
  Serial.println(F("E: R only @90 (L detached) -> listen (4s)"));
  delay(4000);

  // ---- F) 兩顆一起，保持 90°：對照 D/E 有沒有明顯變吵 ----
  reward_L.attach(PIN_L);
  reward_R.attach(PIN_R);
  reward_L.write(90);
  reward_R.write(90);
  Serial.println(F("F: BOTH @90 -> compare with D/E (4s)"));
  delay(4000);

  // ---- G) 全部 detach 靜音一下，方便分辨段落 ----
  reward_L.detach();
  reward_R.detach();
  Serial.println(F("G: all detached (silence gap 2s)"));
  delay(2000);

  Serial.println(F("--- loop restart ---"));
}
