# Arduino UART 版本演進說明

## 版本功能比較總表

| 版本 | 主要功能 | 獎勵控制方式 | LED閃爍方式 | 重要修正 |
|------|---------|------------|-----------|---------|
| **v2** | 基礎左右/直立模式 | 繼電器 (relay) | 阻塞式 (blocking) | 基礎版本 |
| **v3** | 新增正確回應判斷 | 繼電器 (relay) | 阻塞式 (blocking) | 修正獎勵邏輯 |
| **v4** | 非阻塞式處理 | 繼電器 (relay) | 非阻塞式 (non-blocking) | 解決延遲問題 |
| **v5** | 伺服馬達版本 | 伺服馬達 (servo) | 非阻塞式 (non-blocking) | 硬體更新 |
| **v6** | 純左右模式 | 伺服馬達 (servo) | 非阻塞式 (non-blocking) | 移除模式選擇 |

---

## 詳細版本說明

### v2 - 基礎版本
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v2/`

**主要功能**:
- Mode 1: 攝影機偵測模式 (Camera detection mode) - 左右轉任務
- Mode 2: 站立模式 (Standup mode) - 隨機間隔給水 (20-35秒)

**硬體配置**:
- 獎勵控制: 繼電器 (reward_L: pin 6, reward_R: pin 7)
- LED: L1/L2 (pin 11/10), R1/R2 (pin 13/12)

**已知問題**:
- LED閃爍期間會阻塞其他指令處理
- 不論走對走錯都給水

**獨立衍生檔案**:
- `blah_altertime.ino`: 新增模式3 (固定時間間隔的站立模式)
- `blah_correct_reward.ino`: 新增正確回應判斷 (只有走對邊才給水)

---

### v3 - 正確回應判斷版本
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v3/`

**新增功能**:
- Mode 1: 攝影機偵測模式 (Camera detection mode) - 左右轉任務
- Mode 2: 站立模式 (Standup mode) - 隨機間隔給水 (20-35秒)
- Mode 3: 固定時間間隔站立模式 (Standup fixed mode) - 可手動設定固定時間間隔給水
- ✅ 判斷大鼠是否走對邊，只有正確才給水
- ✅ 新增狀態變數: `isObstacle_L`, `isObstacle_R`, `taskInProgress`
- ✅ 新增獎勵時間控制: `reward_time = 3500ms`

**核心改進**:
- 接收 'L' 或 'R' 訊號時會檢查是否與任務方向 (`ans`) 一致
- 走錯邊會重置狀態但不給水
- 新增 `resetState()` 函數統一管理狀態重置

**仍存在的問題**:
- LED閃爍仍為阻塞式，可能導致指令延遲

---

### v4 - 非阻塞式處理版本
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v4/`

**核心改進**:
- ✅ **非阻塞式LED閃爍**: 使用 `millis()` 和狀態機實現
- ✅ **訊號緩衝處理**: 即使在閃爍中也能立即處理指令
- ✅ 新增 `processResponse()` 函數獨立處理每個指令

**技術細節**:
```cpp
// 非阻塞閃爍相關變數
bool blinking = false;
unsigned long blinkLastToggle = 0;
int blinkCount = 0;
int blinkState = 0;
const int maxBlinks = 15;
```

**執行流程改善**:
1. `left_right()` 中先累積所有指令進 buffer
2. 使用 `processResponse()` 逐一處理
3. LED閃爍與指令處理並行，不會互相阻塞

**解決的問題**:
- ✅ 解決 "如果在紅點下待太久會出現觸發LED CUE錯誤的問題"
- ✅ 避免閃爍時 L/R 指令被讀走但無法處理

---

### v5 - 伺服馬達版本
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v5/`

**硬體更新**:
- ✅ **獎勵系統**: 從繼電器改為伺服馬達 (因直流抽水馬達對電壓值敏感)
  - `reward_L`: Servo on pin 10 (角度 114° 給水, 90° 停止)
  - `reward_R`: Servo on pin 9 (角度 60° 給水, 90° 停止)
- ✅ **獎勵時間縮短**: `reward_time = 300ms` (v3/v4 為 3500ms)
- ✅ **LED pin腳調整**: L1/L2 (pin 12/13), R1/R2 (pin 6/7)

**Mode 3 功能更新**:
- Mode 3 改為手動控制LED測試模式
- 接收 '1' 指令: LED閃爍10秒
- 接收 '0' 指令: LED關閉

**程式碼變化**:
```cpp
// v4使用繼電器
digitalWrite(reward_L, LOW);  // 激活
digitalWrite(reward_L, HIGH); // 停止

// v5使用伺服馬達
reward_L.write(114);  // 激活
reward_L.write(90);   // 停止
```

---

### v6 - 純左右模式版本 (為避免 50Hz 雜訊)
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v6/`

**核心改進**:
- ✅ **移除模式選擇**: 開機自動進入 Mode 1（左右轉任務）
- ✅ **即插即用**: 按 reset 鍵或接上行動電源立即啟動
- ✅ **精簡程式碼**: 移除 Mode 2/3 相關程式碼
- ✅ **獨立供電友善**: 不需連接電腦選擇模式，適合使用行動電源

**使用情境**:
- Arduino 改用行動電源供電，避免電腦 USB 雜訊干擾
- 不需要透過 MobaXterm 選擇模式
- 專注於左右轉任務實驗

**與 v5 的差異**:
```cpp
// v5: 需要等待模式選擇
while (Serial.available() == 0) {
  // 等待用戶輸入 '1', '2', '3'
}

// v6: 直接啟動
Serial.println("Camera detection mode (left-right task) started automatically");
```

**功能保留**:
- 伺服馬達控制 (reward_L: pin 10, reward_R: pin 9)
- 非阻塞式 LED 閃爍
- 正確回應判斷 (只有走對邊才給水)
- 獎勵時間: 300ms
- LED pin: L1/L2 (pin 12/13), R1/R2 (pin 6/7)

---

# Others
3D tracker
- official doc when behavior box be setup: [[3DTracker Doc for v180924 (under refinement).pdf]]
- [3DTracker-FAB documentation — 3DTrackerFAB-doc documentation](https://3dtrackerfab.readthedocs.io/en/latest/)

---
# 《需求列表》

### 直立式給水器｛站立｝
1. 針對每次水量限制問題+鼠鼠會扒上去/電線被3D tracker抓到難濾掉的問題
  - sol1. 把馬達架高、深度變深、上面也要做高擋起來
2. 瀑布自助餐問題
  - sol2. 中繼出水孔（鼠鼠舔的地方）深度挖深一點（？）可以小小裝水、累積到某個水量再流掉
  - sol3. 換材質

3. 給水時間random除錯困難的問題
  - sol4. 新增輸入mode 2時間，改Arduino

### 掛勾式給水平台｛左右｝
1. 鼠鼠頭上線會勾到方形轉角的問題
  - sol5. 把平台改半圓（包含底部也是弧狀）
  - sol6. 把平台改高一點，增加鼠鼠底下通過空間
2. 鼠鼠選錯邊依舊會給reward的殘水問題
  - sol7. 直接改code，把紅點以外的其他點作為判斷是否走對邊的檢查點，判斷正確才出水
3. 出水針頭因為各種水壓阻塞底座不穩導致需一直調整的問題
  - sol8. 印一個針頭固定座

### 銅網上的線路
1. 線路時常接觸不良的問題
  - sol9. 焊起來一勞永逸（嗎
  - sol10. 應歪歪要求封裝起來
  - sol11. 更新乖乖、改善風水佈局
  - sol12. 3D列印鼠鼠女神像祭壇

### 其他
1. LED燈會不定時左右亂閃
  - sol13. 改善code
  - sol14. 買更多乖乖
2. 忘記這是指什麼了XD👇
  - 可以定義順序，改python
