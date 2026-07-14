/*
 * servo_sweep_pintest.ino
 * ---------------------------------------------------------------
 * 用途：單一 pin 腳的伺服「乾淨」測試，用來診斷某支腳是否正常。
 *
 * 特性（刻意做到最乾淨，排除 v7 的所有干擾）：
 *   - 只有「一個」attach，全程不 detach
 *   - 沒有模式選擇、沒有 LED、沒有 UART 指令邏輯
 *   - 連續來回掃動，伺服有沒有在動「一眼就看得出來」
 *
 * 使用方式：
 *   1. 把伺服「從機構上拆下來」空轉測（排除三向閥/齒輪卡住）。
 *   2. 改下面的 SERVO_PIN 成你要驗的腳（例如 9 / 4 / 6），燒錄。
 *   3. 確認訊號線「確實插緊」在對應的 header 腳。
 *   4. 觀察伺服會不會「連續來回掃」：
 *        - 會順順掃            → 這支腳正常。
 *        - 嗡嗡叫、抖、掃不動   → 這支腳送出的脈衝有問題（腳受損或接線）。
 *        - 完全不動、軟趴趴     → 沒收到訊號（線沒接好 / 腳沒輸出）。
 *
 * 診斷邏輯：
 *   - 若 pin9 嗡叫、但 pin4 或 pin6 會正常掃
 *       → pin9 這支輸出腳壞了 → v7 把 reward_R 改用健康的腳即可。
 *   - 若這支乾淨 sketch 在 pin9 反而「正常掃」
 *       → pin9 沒壞！問題出在 v7 的 attach/detach 或 manual mode 邏輯 → 改韌體。
 *
 * 註：pin4/pin6 不是硬體 PWM 腳，但 Servo 函式庫底層用 Timer1，
 *     可以驅動「任何」數位腳，所以拿來當對照腳完全沒問題。
 */

#include <Servo.h>

const int SERVO_PIN = 9;   // <<<<< 改這裡：要驗哪支腳就改成 9 / 4 / 6 ...
const int STEP_DEG  = 5;   // 每步幾度（和 v7 的 moveServoSmooth 一致）
const int STEP_MS   = 20;  // 每步間隔毫秒（越大掃越慢、越好觀察）

Servo s;

void setup() {
  Serial.begin(115200);
  s.attach(SERVO_PIN);
  Serial.print("Servo sweep test running on pin ");
  Serial.println(SERVO_PIN);
}

void loop() {
  // 0 -> 180
  for (int a = 0; a <= 180; a += STEP_DEG) {
    s.write(a);
    delay(STEP_MS);
  }
  Serial.println("reached 180, sweeping back...");

  // 180 -> 0
  for (int a = 180; a >= 0; a -= STEP_DEG) {
    s.write(a);
    delay(STEP_MS);
  }
  Serial.println("reached 0, sweeping up...");
}
