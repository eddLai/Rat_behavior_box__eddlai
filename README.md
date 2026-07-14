## Now Operating Version...
- **Arduino for Cue and Reward**: [`UART_left_right_dir_deter_v8.ino`](Arduino_UART_from_python/UART_left_right_dir_deter_v8/UART_left_right_dir_deter_v8.ino)
- **Camera for Rat Position**: [`python_to_detect_rat_at_point/two_camera_videos_diff_Uart_blah.py`](python_to_detect_rat_at_point/two_camera_videos_diff_Uart_blah.py)
---

# Arduino UART 版本演進說明

## 版本功能比較總表

| 版本 | 主要功能 | 獎勵控制方式 | LED閃爍方式 | 重要修正 |
|------|---------|------------|-----------|---------|
| **v2** | 基礎左右/直立模式 | 繼電器 (relay) | 阻塞式 (blocking) | 基礎版本 |
| **v3** | 新增正確回應判斷 | 繼電器 (relay) | 阻塞式 (blocking) | 修正獎勵邏輯 |
| **v4** | 非阻塞式處理 | 繼電器 (relay) | 非阻塞式 (non-blocking) | 解決延遲問題 |
| **v5** | 伺服馬達版本 | 360伺服馬達 (servo) | 非阻塞式 (non-blocking) | 硬體更新 |
| **v6** | 純左右轉模式 | 360伺服馬達 (servo) | 非阻塞式 (non-blocking) | 移除模式選擇 |
| **v7** | 三向閥與純手動控制 | 180伺服馬達 (servo) | 非阻塞式 (non-blocking) | (1)平滑步進控制三向閥+齒輪開關 (2)新增修改起身模式、手動控制模式、排水模式 |
| **v8** | 新增長距離移動/純轉向模式 | 180伺服馬達 (servo) | 非阻塞式 (non-blocking) | (1)新增 Mode 5：碰 L/R 直接給水，黑點 `D` 作為 rearm 防重複 (2)全部字串改用 `F()` 巨集釋放 SRAM (3)記憶體不足，暫時註解停用 Mode 2 |

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

### v6 - 純左右模式版本 (為排除 50Hz 雜訊)(但實測無效:P)
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

---

### v7 - 三向閥開關門與純手動控制版本
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v7/`

**核心改進與硬體更新**:
- ✅ **180度三向門控制**: 取代原本的 360 度連續旋轉馬達，改用 180 度伺服馬達控制實體三向門 (`posClose = 0`, `posOpen = 180`)。
- ✅ **平滑步進控制 (`moveServoSmooth`)**: 為了減少機構震動與噪音，所有開關門動作皆改為每 10ms 移動 2 度的平滑步進，大幅提升硬體作動穩定性與動物實驗品質。
- ✅ **馬達防抖優化**: `attach()` 與 `detach()` 邏輯完美整合於模式切換與各功能中，並加入起訖點判斷，消除待機時的雜訊與抖動。
- ✅ **緊急停止 (`D`) 安全升級**: 收到 `D` 時，兩側門會平滑強制關閉，並確實重置當前位置記憶 (`currentPosL/R`)。

**重構&更新模式**:
- **Mode 1 (左右轉任務)**: 保留核心邏輯，走對邊時平滑開門給水後關門。
- **Mode 2 (自動站立)**: 移除水泵控制，改為僅開啟「右門」並維持 6 秒，隨機間隔 20~35 秒。
- **Mode 3 (改成手動控制開關)**: 徹底移除舊版不直覺的秒數輸入。改為「純手動發球機」：
  - 輸入 `1`: 右門打開，LED 持續閃爍 (給水)。
  - 輸入 `0`: 右門關閉，LED 立即熄滅。
- **Mode 4 (新增：Drain Mode)**: 實驗後的專屬管路清洗/排水模式。
  - 輸入 `O`: 雙門全開。
  - 輸入 `C`: 雙門關閉。
  - 提示可隨時重啟序列埠來回到主選單。

---

### v8 - 長距離移動模式與記憶體 (SRAM) 優化版本
**檔案位置**: `Arduino_UART_from_python/UART_left_right_dir_deter_v8/`

**核心改進**:
- ✅ **新增 Mode 5 (Long locomotion / pure turning)**: 適用於長距離移動與純轉向任務。
  - 碰到 `L` 點 → 平滑開左門給水後關門；碰到 `R` 點 → 平滑開右門給水後關門。
  - **與 Mode 1 的差異**: 不需 `M` 觸發、不指定方向、不判斷對錯 —— 碰到就給。
- ✅ **Mode 5 防重複機制 (`armed` 狀態鎖)**: 解決「老鼠只觸發一次，馬達卻反覆開關多次」的問題。
  - 根因：Python 端是逐幀送訊號，老鼠停在 L/R 點時會連續灌入大量 `L`/`R`，每個字元都觸發一次給水。
  - 解法：用單一共用鎖 `armed`。給一次水後立即上鎖 (`armed = false`)，阻塞給水期間湧入的重複訊號全部忽略；**必須走過任一黑點（收到 `D`）才重新武裝** (`armed = true`)，才能再給下一次。
  - 初始 `armed = true`，第一次碰 L/R 即給水。此設計逼老鼠實際跑完路線（黑點→L/R）才有獎勵，符合 locomotion 實驗本意。
  - Arduino 端相容既有 Python：三個黑點 (MD/LD/RD) 都送 `D`，任一個都可 rearm。
- ✅ **SRAM 記憶體優化（關鍵修正）**: 所有 `Serial.print/println` 字串改用 `F()` 巨集包裝。
  - 問題：加入 Mode 5 後全域變數用到 **2047/2048 bytes (99%)**，開機時 stack 與全域變數相撞 → **選單完全印不出來、系統無反應**。
  - 解法：`F("...")` 讓字串留在 Flash（32KB，充裕）而非複製到 SRAM（僅 2KB）。行為與輸出完全不變，只是搬移儲存位置。

**⚠️ Mode 2 (自動站立) 暫時停用**:
- 因 ATmega328P 的 SRAM 僅 2KB，為釋放記憶體空間，先將 `stand_up()` 函式及 `loop()` 內的 Mode 2 分支**註解掉**。
- 開機選單仍列出 `Enter '2'`，但該模式功能尚未啟用。

**Mode 對照表 (v8)**:
| Mode | 功能 | 說明 |
|------|------|------|
| 1 | 攝影機偵測（左右轉） | 需 `M` 觸發、判斷對錯，走對邊才給水 |
| 2 | ~~自動站立~~ | ⚠️ 暫時停用（記憶體不足） |
| 3 | 手動控制 | 輸入 `1` 開右門+閃燈、`0` 關門 |
| 4 | 排水 (Drain) | 輸入 `O` 雙門全開、`C` 雙門關閉 |
| 5 | 長距離移動 / 純轉向 | 碰 `L`/`R` 直接給水，黑點 `D` 作為 rearm |

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
