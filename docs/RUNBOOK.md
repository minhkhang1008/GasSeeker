# Hướng dẫn thực hiện — từ đống linh kiện đến bảng số liệu

Đọc theo thứ tự. Mỗi giai đoạn có **tiêu chí đạt**; chưa đạt thì đừng sang giai đoạn sau —
đó là cách duy nhất để không phải gỡ rối cả hệ cùng một lúc.

Tổng thời gian ước tính: **12–16 giờ làm việc**, chia được thành 4–5 buổi.

| Giai đoạn | Nội dung | Thời gian |
|---|---|---|
| [0](#giai-đoạn-0--phần-mềm-trước-khi-động-vào-linh-kiện) | Phần mềm, chạy mô phỏng | 30 phút |
| [1](#giai-đoạn-1--lắp-cơ-khí) | Lắp cơ khí | 2–3 giờ |
| [2](#giai-đoạn-2--mạch-nguồn-làm-riêng-đo-riêng) | Mạch nguồn (làm riêng, đo riêng) | 1–2 giờ |
| [3](#giai-đoạn-3--đấu-tín-hiệu) | Đấu tín hiệu | 1–2 giờ |
| [4](#giai-đoạn-4--cấp-điện-lần-đầu-và-selftest) | Cấp điện lần đầu + `selftest` | 1 giờ |
| [5](#giai-đoạn-5--hiệu-chuẩn-chuyển-động) | Hiệu chuẩn chuyển động | 1–2 giờ |
| [6](#giai-đoạn-6--hiệu-chuẩn-mq-3-quan-trọng-nhất) | Hiệu chuẩn MQ-3 | 2–3 giờ |
| [7](#giai-đoạn-7--chạy-thử-từng-thuật-toán) | Chạy thử ba thuật toán | 1–2 giờ |
| [8](#giai-đoạn-8--đo-chính-thức) | Đo chính thức 30 lần | 3–4 giờ |
| [9](#giai-đoạn-9--phân-tích-và-báo-cáo) | Phân tích + báo cáo | 2 giờ |

---

## Ba nguyên tắc an toàn — vi phạm là cháy linh kiện

1. **Không bao giờ cắm module LoRa vào 5 V.** Nó là 3,3 V. Cắm nhầm là hỏng vĩnh viễn.
2. **Không cấp điện cho LoRa khi chưa gắn ăng-ten.** Phát mà không có ăng-ten làm hỏng
   tầng khuếch đại công suất.
3. **Đo điện áp đầu ra của buck converter *trước khi* nối vào board.** Buck rẻ tiền hay
   xuất xưởng ở mức 12 V. Nối thẳng vào ESP32 là mất luôn board.

Thêm một nguyên tắc mềm: **kê xe lên cao** (đặt trên hộp, bánh không chạm đất) trong suốt
giai đoạn 4 và phần đầu giai đoạn 5. Xe chạy mất kiểm soát trên bàn sẽ rơi.

---

## GIAI ĐOẠN 0 — Phần mềm, trước khi động vào linh kiện

```bash
cd ~/Downloads/GasSeeker
python3 -m pip install -r tools/requirements.txt
pio run -e test && ./.pio/build/test/program     # phải ra 86/86
pio run -e sim  && ./.pio/build/sim/program --trials 10 --traj
python3 tools/analyze.py --sim
```

Mở `data/figures/fig_quydao_*.png` xem quỹ đạo ba thuật toán. Hiểu chúng làm gì **trước khi**
lắp xe thì lúc chạy thật bạn mới phân biệt được đâu là hành vi bình thường, đâu là hỏng.

**Tiêu chí đạt:** 86/86 test, có biểu đồ trong `data/figures/`.

---

## GIAI ĐOẠN 1 — Lắp cơ khí

Thứ tự lắp (làm ngược lại sẽ phải tháo ra lắp lại):

1. **Gắn motor vào gá, gá vào khung.** Siết vừa tay, chưa siết chặt.
2. **Hàn dây vào motor NGAY BÂY GIỜ**, trước khi lắp bánh — lát nữa không với tới được.
   Hàn luôn **tụ gốm 100 nF qua hai cực mỗi motor**. Bỏ bước này thì nhiễu chổi than sẽ phá
   tín hiệu encoder, và bạn sẽ mất nhiều giờ mới tìm ra nguyên nhân.
3. **Lắp đĩa encoder lên trục**, rồi **gá cảm biến encoder quang** sao cho đĩa đi lọt qua khe.
   Khe phải ôm đĩa mà không chạm. Đây là chi tiết hay sai nhất của cả bộ khung.
4. **Lắp bánh xe và bánh caster.**
5. **Lắp tầng trên bằng trụ đồng M3** (25–40 mm).
6. **Gắn MQ-3 ở đầu xe**, càng thấp càng tốt — hơi ethanol nặng hơn không khí nên tích tụ ở
   lớp sát nền.
7. **Gắn hai micro switch ở đầu xe**, có thanh cản nhẹ chạm được cả hai.
8. **Ăng-ten LoRa dựng đứng**, tránh xa motor và dây nguồn.

Đo và ghi vào sổ bốn số sau (lát nữa nhập vào `config.h`):

| Đại lượng | Cách đo |
|---|---|
| `WHEEL_DIAMETER_MM` | đường kính bánh, đo bằng thước kẹp |
| `WHEEL_BASE_MM` | khoảng cách giữa **tâm mặt tiếp xúc** của hai bánh dẫn động |
| `SENSOR_OFFSET_CM` | tâm trục hai bánh → đầu dò MQ-3 |
| `ENCODER_SLOTS` | **đếm số khe trên đĩa** (thường 20, nhưng phải đếm) |

**Tiêu chí đạt:** đẩy xe bằng tay thì hai bánh quay trơn, đĩa encoder không cọ vào khe.

---

## GIAI ĐOẠN 2 — Mạch nguồn (làm riêng, đo riêng)

Làm toàn bộ giai đoạn này **khi chưa nối board nào vào**.

1. Lắp cầu chì và công tắc chính nối tiếp với cực dương của pin.
2. Nối **hai buck converter** song song vào sau công tắc.
3. Bật nguồn. Dùng đồng hồ và **chỉnh biến trở** trên từng buck:
   - Buck 1 → **6,0 V** (cho motor, chân VM của TB6612)
   - Buck 2 → **5,0 V** (cho ESP32 và MQ-3)
4. Đo lại sau 1 phút để chắc điện áp không trôi.
5. Tắt nguồn. Hàn tụ: **1000 µF** gần VM của TB6612, **470 µF** gần TB6612,
   **220–470 µF** gần chỗ sẽ cắm ESP32 và gần chỗ sẽ cắm LoRa, **100 nF** rải khắp.

> Bốn viên pin AA đi kèm khung chỉ dùng để test motor lúc đầu. Nguồn cuối cùng là pack 2S.

**Tiêu chí đạt:** đồng hồ đọc 6,00 ± 0,1 V và 5,00 ± 0,1 V, đo ở **đúng đầu dây sẽ cắm vào
board**. Sơ đồ tổng thể xem [WIRING.md](WIRING.md) mục "Nguồn".

---

## GIAI ĐOẠN 3 — Đấu tín hiệu

Bảng chân đầy đủ ở [WIRING.md](WIRING.md). Vài điểm dễ sai:

- **Tất cả các khối phải chung GND.** Thiếu một dây GND là toàn bộ tín hiệu thành rác.
- **LoRa lấy 3V3**, không phải 5 V.
- **Encoder**: nếu module chạy 5 V thì **phải chia áp về 3,3 V** trước khi vào GPIO.
- **MQ-3**: sợi đốt cần **5 V**; chân AO đi qua mạch chia áp 10 kΩ / 20 kΩ rồi mới vào GPIO4.
- **Tuyệt đối không dùng GPIO 26–37** — flash và PSRAM của ESP32-S3 chiếm dải này.
- Dây tín hiệu của MQ-3 và encoder **đi tách xa dây motor**. Chạy song song là rước nhiễu.

Trước khi cấp điện, chạy hết checklist ở cuối [WIRING.md](WIRING.md).

---

## GIAI ĐOẠN 4 — Cấp điện lần đầu và `selftest`

### 4.1. Nạp firmware (cắm USB, chưa bật nguồn pin)

```bash
pio run -e robot -t upload
pio device monitor
```

Màn hình phải in banner `GasSeeker`, kèm dòng `MPU6050:` và `LoRa:`.

### 4.2. Kê xe lên cao rồi chạy tự kiểm tra

Bật nguồn pin. Trong cửa sổ monitor gõ:

```
selftest
```

Robot lần lượt kiểm tra I2C, gyro, ADC của MQ-3, công tắc va chạm, LoRa, rồi quay **từng
bánh một** 0,8 giây và đếm xung encoder. Kết quả in ra dạng bảng `DAT / CANH / HONG`.

Sửa **hết** dòng `HONG` rồi mới đi tiếp. Bảng sự cố ở [cuối trang](#phụ-lục--sự-cố-thường-gặp).

### 4.3. Kiểm tra chiều quay

```
mot 120 120     → cả hai bánh phải quay VỀ PHÍA TRƯỚC
mot 120 -120    → xe phải quay tại chỗ
```

- Một bánh quay ngược → **đảo AIN1 với AIN2** (hoặc BIN1 với BIN2) của bánh đó.
- Bánh rung mà không quay → tăng `PWM_MIN_MOVE` trong `config.h` (mỗi lần 10 đơn vị).

### 4.4. Thử LoRa

Nạp trạm thu vào ESP32 thứ hai (`pio run -e base -t upload`), mở cửa sổ monitor thứ hai,
rồi trên xe gõ `ping`. Trạm phải in 5 dòng `PING` kèm RSSI.

**Tiêu chí đạt:** `selftest` không còn dòng `HONG`, xe tiến/quay đúng chiều, trạm nhận được `ping`.

---

## GIAI ĐOẠN 5 — Hiệu chuẩn chuyển động

### 5.1. Nhập số đo cơ khí

Mở `src/core/config.h`, sửa bốn hằng số đã đo ở giai đoạn 1 (`WHEEL_DIAMETER_MM`,
`WHEEL_BASE_MM`, `ENCODER_SLOTS`, `SENSOR_OFFSET_CM`), rồi:

```bash
pio run -e test && ./.pio/build/test/program    # phải vẫn 86/86
pio run -e robot -t upload
```

> **Chạy bộ test sau mỗi lần sửa `config.h`.** Một ngưỡng đặt sai có thể làm thuật toán
> không bao giờ dừng, và bạn chỉ phát hiện ra sau khi đã mất cả buổi đo.

### 5.2. Hiệu chuẩn quãng đường

Đặt xe xuống sàn phẳng, đánh dấu vị trí đầu xe bằng băng dính.

```
drive 100
```

Đo **quãng đường thật** bằng thước. Firmware in sẵn công thức sửa `WHEEL_DIAMETER_MM`.
Sửa → nạp lại → làm lại. Lặp cho tới khi **lệch dưới 3 %**.

### 5.3. Hiệu chuẩn góc quay

```
turn 360
```

Xe phải về đúng hướng ban đầu.

- Lệch nhiều và **luôn cùng một chiều** → bias gyro sai. Khởi động lại và **giữ xe yên hoàn
  toàn** trong 2 giây đầu (lúc đó firmware đang đo bias).
- Quay quá đà rồi lắc qua lắc lại → giảm `TURN_KP`.
- Quay thiếu rồi dừng sớm → giảm `TURN_TOLERANCE_DEG` hoặc tăng `TURN_KP`.

### 5.4. Kiểm tra đi thẳng

```
drive 200
```

Xe lệch sang một bên quá 10 cm → tăng `HEADING_KP`, mỗi lần 0,5.

**Tiêu chí đạt:** `drive 100` sai dưới 3 cm · `turn 360` sai dưới 10° · `drive 200` lệch
ngang dưới 10 cm.

---

## GIAI ĐOẠN 6 — Hiệu chuẩn MQ-3 (**quan trọng nhất**)

Giai đoạn này quyết định robot có chạy được hay không. Bốn bước, đúng thứ tự.

### 6.1. Đo điện trở tải `RL` trên module

Tắt điện. Dùng đồng hồ đo điện trở giữa chân **AO** và **GND** của module MQ-3.
Ghi vào `MQ3_RL_OHM`. Bỏ bước này thì `ppm` sai cả bậc độ lớn.

### 6.2. Sấy nóng rồi đo baseline

Cấp điện, để **5–10 phút** ở nơi thoáng, không có cồn. Gõ `cal`.
Firmware in `Baseline = ... , R0 = ... ohm`. Ghi vào sổ.

> MQ-3 cần sấy nóng thật sự. Đo khi chưa đủ nóng thì mọi số sau đó đều sai.

### 6.3. Fit hằng số `A`, `B`

Mở datasheet module bạn mua, tìm đồ thị ethanol/alcohol, đọc **hai điểm** (nồng độ, tỉ số Rs/R0):

```bash
python3 tools/mq3_fit.py fit --p1 0.1 2.6 --p2 10 0.22 --unit mgl
```

Dán hai dòng nó in ra vào `config.h`.

### 6.4. Chốt ba ngưỡng điều khiển — việc quan trọng nhất của cả dự án

1. Đặt nguồn ethanol cố định trên sân (đĩa nhỏ, **cùng thể tích cho mọi lần thử**, ví dụ 5 ml).
2. Đặt robot lần lượt cách nguồn **150, 100, 70, 50, 30, 15 cm**. Ở mỗi vị trí chờ khoảng
   30 giây cho cảm biến ổn định rồi gõ:

   ```
   sniff
   ```

   Firmware in một dòng `norm / adc / ppm`. Ghi lại kèm khoảng cách.
   (Muốn xem cả chuỗi liên tục thì dùng `gas` — nó in các cột cách nhau bằng TAB, dán thẳng
   được vào bảng tính.)

3. Nhìn bảng vừa ghi rồi đặt trong `config.h`:

| Hằng số | Đặt bằng |
|---|---|
| `DETECT_DELTA` | `norm` ở khoảng cách xa nhất mà nó còn **nhô hẳn** khỏi nhiễu nền |
| `STOP_HIGH_DELTA` | `norm` đo được ở khoảng **30–40 cm** |
| `PLATEAU_EPS` | 2–3 lần biên độ dao động của `norm` khi robot đứng yên |

4. Vẽ đường đặc tuyến (khoảng cách ↔ `norm` ↔ `ppm`) để đưa vào báo cáo. Không cần chính xác
   tuyệt đối — chỉ cần chứng minh **đơn điệu và đúng xu hướng**.

5. Nạp lại rồi **chạy lại bộ test**:

```bash
pio run -e test && ./.pio/build/test/program
pio run -e robot -t upload
```

> Không `ppm` nào được đưa vào ba ngưỡng trên. `DETECT_DELTA`, `STOP_HIGH_DELTA`,
> `PLATEAU_EPS` đều tính bằng **đếm ADC** (Lớp 1). `T1/T2/T3` mới tính bằng ppm và
> **chỉ để hiển thị**.

**Tiêu chí đạt:** có bảng khoảng cách↔`norm`, ba ngưỡng đã nhập, test vẫn 86/86.

---

## GIAI ĐOẠN 7 — Chạy thử từng thuật toán

### 7.1. Bố trí sân

```
        y
        ↑
  200cm ┌───────────────────────────────┐
        │                               │
        │            ★ nguồn            │ ← quạt đặt ở cạnh này,
        │          ethanol              │   thổi theo chiều -X
        │                               │
      0 └─●─────────────────────────────┘ → x
          robot xuất phát            200cm
          ô (0,0), hướng +X
```

- Robot **luôn** xuất phát ở ô (0,0), quay mặt hướng +X.
- Nguồn ethanol ở nửa phải sân. **Ghi lại toạ độ (x, y) tính bằng cm** — bắt buộc, thiếu nó
  thì không tính được sai số.
- Môi trường "không quạt": tắt quạt, đóng cửa, chờ 2 phút cho trường nồng độ ổn định.
- Môi trường "có quạt": quạt ở cạnh +X thổi về gốc. **Không đổi vị trí quạt** giữa các lần thử.
- Đổi kích thước sân: sửa `ARENA_W_CM`, `ARENA_H_CM`, `CELL_CM` trong `config.h`.
  Muốn thử nhanh nhiều cỡ mà không sửa file:

  ```bash
  PLATFORMIO_BUILD_FLAGS="-DGS_ARENA_W_CM=250.0f -DGS_ARENA_H_CM=250.0f" pio run -e sim
  ```

### 7.2. Chạy thử

Chạy **theo thứ tự này**, mỗi thuật toán ít nhất một lần trước khi đo chính thức:

```
algo 0   →  start        quét toàn bộ — dễ nhất, xác nhận cơ cấu chạy ổn
algo 1   →  start        gradient
algo 2   →  start        surge-casting
```

Quan sát và đối chiếu với quỹ đạo mô phỏng ở giai đoạn 0. Dấu hiệu **bình thường**:

- Robot **dừng hẳn** trước mỗi phép đo — đó là "dừng ngửi", không phải treo máy.
- Gradient đi thẳng khi nồng độ còn tăng, chỉ ngoái trái/phải khi nồng độ giảm.
- Khi kết luận, robot **quay lại** điểm đo cao nhất rồi mới kêu còi. Đây là hành vi đúng,
  không phải lỗi.
- LED chuyển xanh lá → vàng → cam → đỏ theo mức khí.

Dấu hiệu **phải sửa**:

| Hiện tượng | Xử lý |
|---|---|
| Dừng ngay khi vừa xuất phát | `STOP_HIGH_DELTA` quá thấp — xem lại 6.4 |
| Chạy hết 8 phút mới dừng | `DETECT_DELTA` quá cao, không bao giờ "bắt" được luồng khí |
| Quét xong mà kết luận sai chỗ hẳn | odometry trôi — làm lại 5.2 và 5.3 |
| `norm` nhảy loạn khi motor chạy | thiếu tụ 100 nF trên motor, hoặc dây MQ-3 chạy sát dây motor |

**Tiêu chí đạt:** cả ba thuật toán chạy hết một lần và dừng lại **có lý do**, không phải hết giờ.

---

## GIAI ĐOẠN 8 — Đo chính thức

Quy mô theo đề cương: **3 thuật toán × 2 môi trường × 5 lần = 30 lần chạy**.
Hụt thời gian thì bỏ surge-casting trước (đề cương xếp nó ở mức "Should").

Với **mỗi** lần thử:

1. Mở cửa sổ ghi dữ liệu (thay tham số cho đúng lần chạy):

   ```bash
   python3 tools/receiver.py --algo GRADIENT --env khong-quat --trial 1
   ```

2. Đưa robot về đúng ô xuất phát, đúng hướng.
3. Bấm nút BOOT trên xe (hoặc gõ `start` ở cửa sổ trạm thu).
4. Đợi robot dừng. **Đo bằng thước** khoảng cách từ đầu dò tới nguồn thật, ghi vào sổ giấy.
5. Ctrl-C ở cửa sổ receiver. File CSV nằm trong `data/runs/`.

Giữa các lần thử:

- **Thoáng khí ít nhất 2 phút.** MQ-3 hồi phục mất ~30 giây; vội là lần sau đo sai.
- Gõ `cal` nếu đã nghỉ lâu hoặc vừa thay pin.
- Ghi lại toạ độ nguồn nếu có thay đổi.

Quay video ít nhất một lần chạy của mỗi thuật toán — đề cương yêu cầu video demo.

**Tiêu chí đạt:** đủ 30 file CSV trong `data/runs/`, sổ giấy có toạ độ nguồn của từng lần.

---

## GIAI ĐOẠN 9 — Phân tích và báo cáo

```bash
python3 tools/analyze.py
```

Lần đầu nó tạo `data/runs/meta.csv`. Mở file đó, điền `src_x`, `src_y` (cm) của nguồn thật
cho từng lần chạy, rồi chạy lại lệnh trên.

Kết quả nằm ở `data/figures/`:

| File | Dùng cho mục nào của báo cáo |
|---|---|
| `bang_ketqua.csv` | bảng số liệu chính |
| `fig_thoigian.png` | chỉ số chính — thời gian định vị |
| `fig_quangduong.png` | năng lượng tiêu thụ |
| `fig_saiso.png` | sai số định vị |
| `fig_thanhcong.png` | tỉ lệ thành công |
| `fig_quydao_*.png` | minh hoạ hành vi từng thuật toán |
| `fig_bandonhiet_*.png` | bản đồ nhiệt nồng độ thô — sản phẩm phụ, đề cương mục 11.4 |

Bốn điều **phải** nêu trong báo cáo để trung thực:

1. `ppm` là **ước lượng bậc độ lớn**, quy đổi theo đường đặc tuyến của nhà sản xuất, không
   phải phép đo nồng độ chính xác (đề cương mục 11.1d).
2. Hướng gió là **khai báo trước** theo bố trí sân; robot **không** tự đo (mục 11.2).
3. Giai đoạn SEEK dùng **chung** cho gradient và surge-casting, nên bảng so sánh đo hành vi
   **sau khi** bắt được luồng khí (xem `DECISIONS.md` mục D3).
4. Ethanol là **chất mô phỏng an toàn**, không phải khí độc thật; kết quả tương đồng về động
   học phát tán, không phải kiểm chứng hoá học (mục 17).

Nếu có dùng số liệu mô phỏng, để **mục riêng**, không trộn với số liệu đo thật.

---

## Sân rộng tối đa bao nhiêu?

Đo trên mô phỏng, thuật toán **quét toàn bộ** (nặng nhất), 6 lần thử mỗi cỡ:

| Sân | Ô lưới | Lưới | Thời gian | Quãng đường | Sai số | Thành công |
|---|---|---|---|---|---|---|
| 1,5 × 1,5 m | 25 cm | 6×6 | 158 s | 9,5 m | 19 cm | 6/6 |
| **2,0 × 2,0 m** | 25 cm | 8×8 | **279 s** | 17,1 m | 23 cm | **6/6** |
| 2,5 × 2,5 m | 25 cm | 10×10 | 429 s | 26,2 m | 26 cm | 3/6 |
| 3,0 × 3,0 m | 25 cm | 12×12 | 611 s | 37,4 m | 29 cm | 2/6 |
| 3,0 × 3,0 m | 50 cm | 6×6 | 218 s | 19,6 m | 28 cm | 3/6 |
| 4,0 × 4,0 m | 50 cm | 8×8 | 373 s | 33,7 m | 40 cm | 1/6 |
| 5,0 × 5,0 m | 50 cm | 10×10 | 583 s | 53,3 m | 35 cm | 1/6 |

**Công thức xấp xỉ:** `thời gian ≈ (diện tích / ô²) × (2,9 + ô/18) giây`.

Ba trần khác nhau, cái nào chạm trước thì cái đó quyết định:

1. **Timeout 8 phút** (`MISSION_TIMEOUT_MS`): ô 25 cm → ~114 ô → **2,6 × 2,6 m**;
   ô 50 cm → ~78 ô → **4,4 × 4,4 m**.
2. **Sai số dead-reckoning — trần thật.** Độ trôi khoảng **0,5–0,8 % quãng đường**
   (10 cm sau 17 m, 50 cm sau 64 m). Bán kính thành công 30 cm → quãng đường phải dưới ~40 m
   → **khoảng 3 × 3 m với ô 25 cm**.
3. **Tầm phát hiện thật của MQ-3 — chưa đo được.** SEEK lấy mẫu cách nhau 50 cm, trường hợp
   xấu nhất luồng khí cách điểm đo 35 cm. Đo ở bước 6.4 sẽ biết, và đây có thể là trần thấp
   nhất trong ba cái.

**Khuyến nghị:** giữ **2 × 2 m, ô 25 cm**. Gradient và surge-casting không bị giới hạn theo
diện tích như vậy vì không quét hết sân (76 s ở sân 1,5 m, 143 s ở sân 4 m).

---

## Phụ lục — sự cố thường gặp

| Hiện tượng | Nguyên nhân hay gặp |
|---|---|
| `selftest`: MPU6050 KHONG THAY | nhầm SDA/SCL · module chưa hàn chân · chưa cấp 3V3 |
| `selftest`: MQ-3 ADC ~0 | đứt dây AO · mạch chia áp lắp ngược |
| `selftest`: MQ-3 bão hoà (>4000) | chia áp sai tỉ số · AO chạm thẳng 5 V |
| `selftest`: motor không có xung | motor không quay (kiểm tra VM, STBY) · encoder lệch khe · encoder thiếu nguồn |
| `selftest`: quay bánh trái mà bánh phải cũng có xung | hai dây encoder cắm lẫn nhau |
| `LoRa: KHONG DUNG DUOC` | sai chân SPI · thiếu tụ gần module · chưa gắn ăng-ten · nhầm 5 V vào VCC |
| Robot đi cong dù lệnh đi thẳng | bias gyro sai — giữ **yên hoàn toàn** lúc khởi động · hoặc tăng `HEADING_KP` |
| Quay quá đà rồi dao động | giảm `TURN_KP` hoặc tăng `TURN_TOLERANCE_DEG` |
| `norm` nhảy loạn | thiếu tụ 100 nF trên motor · dây MQ-3 sát dây motor · nguồn 5 V sụt khi motor khởi động |
| Robot dừng ngay khi vừa chạy | `STOP_HIGH_DELTA` quá thấp — xem lại 6.4 |
| Robot chạy hết giờ mới dừng | `DETECT_DELTA` quá cao, không bắt được luồng khí |
| Số xung chỉ tăng một bên | encoder lệch khe · nguồn encoder yếu · đĩa bám bụi |
| Sửa `config.h` xong test trượt | đọc dòng test trượt — thường là ngưỡng bị đặt ngược thứ tự |

---

## Bảng lệnh đầy đủ

Gõ vào Serial của xe (hoặc gõ ở trạm thu để gửi lên xe qua LoRa):

| Lệnh | Tác dụng |
|---|---|
| `selftest` | kiểm tra lần lượt mọi khối phần cứng, in bảng PASS/FAIL |
| `start` / `stop` | chạy / dừng khẩn |
| `algo 0` `1` `2` | quét toàn bộ / gradient / surge-casting |
| `cal` | đo lại baseline |
| `info` | trạng thái cảm biến, pose, số xung encoder |
| `mot <L> <R>` | cấp PWM thô 1,5 s — kiểm tra chiều dây motor |
| `drive <cm>` | đi thẳng rồi báo odometry — hiệu chuẩn `WHEEL_DIAMETER_MM` |
| `turn <deg>` | quay rồi báo kết quả |
| `enc` / `bump` | in xung encoder / trạng thái công tắc, 20 s |
| `gas` | bảng ADC/Rs/ppm 60 s, cột cách nhau bằng TAB |
| `sniff` | một phép dừng-ngửi, in một dòng |
| `ping` | gửi 5 gói LoRa thử |
| `bench off` | dừng chế độ kiểm tra |

Nút BOOT trên board: **nhấn ngắn** = Start/Stop · **nhấn giữ ~1 s** = đổi thuật toán.

Màu LED RGB: xanh dương nhấp nháy = đang hiệu chuẩn · trắng/lam/tím = sẵn sàng (theo thuật
toán đang chọn) · xanh lá → vàng → cam → đỏ = mức khí khi đang chạy · xanh lá nhấp nháy =
đã kết luận.
