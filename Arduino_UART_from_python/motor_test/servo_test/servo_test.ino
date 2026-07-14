/*
 * servo_test.ino
 * ---------------------------------------------------------------
 * 最單純的伺服馬達測試程式：只測「一顆」馬達，訊號線接在 pin 4。
 * 上電後就自動讓馬達在 0 度 <-> 180 度之間反覆來回轉。
 *
 * 用途：當馬達突然不會動時，用這支程式排除「主程式邏輯」的問題，
 *       單純確認「馬達 + 接線 + 電源 + Arduino 腳位」是否正常。
 *
 * 接線：
 *   - 馬達訊號線(通常是橘/黃色) -> pin 4
 *   - 馬達電源(紅色) -> 外部 5V 電源，不要只靠 Arduino 的 5V
 *   - 馬達 GND(棕/黑色) -> 外部電源 GND，且務必與 Arduino 的 GND 共地
 *
 * 觀察 Serial Monitor（115200 baud）會印出目前角度，
 * 若馬達不動但序列埠有持續印出角度 -> 多半是接線/電源/馬達本身問題。
 * ---------------------------------------------------------------
 */

#include <Servo.h>

Servo testServo;        // 要測試的馬達
const int SERVO_PIN = 9; // 訊號線接在 pin 4

const int posClose = 0;    // 一端角度
const int posOpen  = 180;  // 另一端角度

void setup() {
  Serial.begin(115200);
  Serial.println("=========================================");
  Serial.println(" Servo Test Start (single motor)");
  Serial.println(" Signal -> pin 4");
  Serial.println(" Sweeping 0 <-> 180 repeatedly...");
  Serial.println("=========================================");

  testServo.attach(SERVO_PIN);

  // 先回到 0 度
  testServo.write(posClose);
  delay(1000);
}

void loop() {
  // ----- 0 -> 180 -----
  Serial.println("Sweeping 0 -> 180...");
  for (int a = posClose; a <= posOpen; a += 5) {
    testServo.write(a);
    Serial.print("  angle = ");
    Serial.println(a);
    delay(20);
  }
  delay(1000); // 停在 180 度 1 秒

  // ----- 180 -> 0 -----
  Serial.println("Sweeping 180 -> 0...");
  for (int a = posOpen; a >= posClose; a -= 5) {
    testServo.write(a);
    Serial.print("  angle = ");
    Serial.println(a);
    delay(20);
  }
  delay(1000); // 停在 0 度 1 秒
}
