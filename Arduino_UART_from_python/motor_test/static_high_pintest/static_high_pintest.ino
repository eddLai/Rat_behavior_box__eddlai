// 靜態拉 High 測試（非 PWM）—— 判 Arduino 數位腳輸出級生死
//
// 用途：診斷 reward_R(pin9) / reward_L(pin10) 那支腳是否被短路灌壞。
// 原理：把腳直接 digitalWrite HIGH（不是 PWM），這樣電表才量得到「穩定 5V」，
//       看得出真正的輸出振幅；PWM 只給平均值（0.2~0.5V）看不出腳好不好。
//
// 量法：腳 ↔ GND
//   ≈ 5V    → 腳健康
//   0~1V    → 輸出級燒了
//   1~3V    → 輸出級弱掉/受損
// 對照：把 TEST_PIN 改成一支從沒用過的腳（例如 8）應該量到 ≈5V。
//
// ⚠️ 接訊號線前先量該線對 GND 不通（高阻抗），確認沒短路再接，否則會把腳灌壞。

const int TEST_PIN = 9;   // 要測的腳：10=reward_L、9=reward_R、8=對照腳

void setup() {
  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, HIGH);   // 靜態拉 High，量 TEST_PIN ↔ GND
}

void loop() {}
