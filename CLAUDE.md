# CLAUDE.md — GasSeeker

Robot tự hành dò tìm nguồn rò rỉ khí bằng cảm biến đơn. Đề tài cuối khoá PIFKID.
File này là bộ nhớ làm việc giữa các phiên. **Đọc hết mục 1–5 trước khi sửa bất cứ thứ gì.**

---

## 1. Ngữ cảnh dự án

- **Đề cương gốc:** `~/Downloads/Robot tự hành dò tìm nguồn rò rỉ khí độc - Đề tài ...md`
  (ngoài repo). Mọi thứ trong repo phải nhất quán với file đó.
- **Deadline:** dưới 2 tuần. Ưu tiên tinh gọn, chạy được, có số liệu — không mở rộng phạm vi.
- **Phân công:** người dùng lo linh kiện + lắp ráp. Claude lo **toàn bộ phần code**.
- **Ngôn ngữ:** trao đổi và bình luận trong code bằng tiếng Việt **không dấu**
  (tránh lỗi encoding trên Arduino/Serial). Tài liệu `.md` thì có dấu bình thường.

### Ba giả thuyết cần số liệu chứng minh
- **H1** — môi trường khuếch tán thuần: **gradient** nhanh nhất.
- **H2** — môi trường phát tán đứt quãng: **surge-casting** nhanh nhất.
- **H3** — **quét toàn bộ** luôn thành công nhưng chậm nhất (baseline).

---

## 2. Kiến trúc — bất biến, không được phá

```
src/core/   THUẦN C++, KHÔNG include Arduino.h.  <- thuật toán sống ở đây
src/robot/  firmware ESP32-S3 trên xe
src/base/   firmware ESP32-S3 trạm thu LoRa
src/lora/   driver SX1262, dùng chung robot + base
src/sim/    mô phỏng chạy native trên macOS
src/test/   bộ test tự động cho src/core/, chạy native
tools/      Python: receiver.py, analyze.py, mq3_fit.py
docs/       DECISIONS.md, WIRING.md, RUNBOOK.md
```

**Quy tắc kiến trúc (vi phạm là hỏng cả hệ):**

1. `src/core/` **không được** include `Arduino.h` hay bất cứ thứ gì của ESP32.
   Nhờ vậy cùng một file `.cpp` thuật toán chạy được trên cả xe lẫn simulator.
2. Thuật toán chỉ nói chuyện với phần cứng qua `gs::IRobot` (`src/core/irobot.h`).
   Muốn thêm cảm biến → thêm hàm vào `IRobot` và hiện thực ở **cả hai** phía
   (`RobotIO` và `SimRobot`).
3. `update()` của thuật toán **không được chặn**. Không `delay()`, không vòng `while`.
4. **Hai lớp dữ liệu khí tách biệt** (đề cương mục 11.1):
   - Lớp 1 `raw` / `normalized` → thuật toán dùng.
   - Lớp 2 `ppm` / `level` → **chỉ** để hiển thị.
   `ppm` **tuyệt đối không** xuất hiện trong điều kiện dừng hay chọn hướng.
5. Mọi hằng số nằm trong `src/core/config.h`. Không rải magic number.
6. Robot phải chạy được khi mất LoRa. LoRa chỉ để giám sát.
7. `enum class AlarmLevel` viết `Safe/Detected/High/Critical` — **không** viết hoa
   toàn bộ, vì `Arduino.h` định nghĩa macro `HIGH`.

---

## 3. Lệnh hay dùng

```bash
pio run -e test && ./.pio/build/test/program                    # 86 test, CHẠY SAU MỖI LẦN SỬA config.h
pio run -e sim && ./.pio/build/sim/program --trials 10 --traj   # mô phỏng
python3 tools/analyze.py --sim                                  # bảng + biểu đồ
pio run -e robot -t upload && pio device monitor                # nạp xe
pio run -e base  -t upload                                      # nạp trạm thu
python3 tools/receiver.py --algo GRADIENT --env khong-quat --trial 1
```

Sim có cờ: `--algo exh|gra|sur|all --env diff|inter|all --trials N --seed S --traj --verbose`.

---

## 4. Trạng thái hiện tại

| Hạng mục | Trạng thái |
|---|---|
| Thuật toán 1 — quét toàn bộ | Xong, chạy tốt trên sim |
| Thuật toán 2 — bám gradient | Xong, chạy tốt trên sim |
| Thuật toán 3 — surge-casting | Xong, chạy tốt trên sim |
| Firmware xe | Xong, **build sạch, CHƯA thử trên phần cứng** |
| Chế độ kiểm tra phần cứng (`bench`) | Xong (`mot/drive/turn/enc/bump/gas/sniff/ping`) |
| Bộ test tự động `src/core/` | Xong, 86/86 đạt |
| Firmware trạm thu | Xong, **build sạch, CHƯA thử trên phần cứng** |
| Simulator | Xong, có số liệu |
| Tools Python | Xong. `analyze.py` đã thử cả nhánh sim và nhánh dữ liệu thực; `receiver.py` chưa chạy với phần cứng thật |
| Hiệu chuẩn MQ-3 thật | **Chưa** — việc của Ngày 4 |
| Số liệu thực nghiệm | **Chưa có** |

Mọi hằng số đánh dấu `[DO]` trong `config.h` **đều đang là giá trị phỏng đoán**
và phải đo lại trên phần cứng. Xem `docs/RUNBOOK.md` mục "Ngày 4".

---

## 5. Những cái bẫy đã gặp (đừng lặp lại)

- **Trễ của MQ-3 là vấn đề trung tâm, không phải chi tiết phụ.** Recovery time
  ~30 s theo datasheet. Hệ quả kéo theo ba thiết kế bắt buộc:
  1. **Dừng ngửi** (`SNIFF_*`): mọi phép đo đều dừng hẳn xe rồi mới lấy mẫu.
  2. **Gradient đi thẳng khi còn tăng**, chỉ quét 3 hướng khi giảm. Quét 3 hướng
     liên tục làm ba phép đo lệch theo *thứ tự đo* chứ không theo không gian.
  3. **`RETURN_TO_BEST`**: robot luôn vượt qua đỉnh rồi mới biết → phải quay lại
     điểm đo cao nhất. Bỏ tính năng này thì sai số tăng gấp 2–3 lần (đã đo).
- **Cảm biến phải gắn lệch về phía trước** (`SENSOR_OFFSET_CM`). Gắn ngay tâm trục
  thì quay tại chỗ không làm đầu dò đổi chỗ → quét 3 hướng vô nghĩa.
- **Surge-casting từng kẹt vòng lặp vô hạn ở góc sân**: không tiến ngược gió được
  → cast, cast lại phát hiện khí → reset biên độ → lặp mãi. Đã sửa bằng
  (a) buộc tăng biên độ khi hướng ngược gió bị chặn, (b) `STALL_LIMIT_SNIFFS`.
- **Ngưỡng dừng phải tính theo dải động thật của cảm biến.** Đặt `STOP_HIGH_DELTA`
  quá thấp thì robot dừng ngay khi vừa ngửi thấy mùi, cách nguồn cả mét.
- **Pha `RETURN` phải là pha CUỐI, không được quay lại tìm kiếm.** Lỗi đã gặp:
  robot kết luận → quay về điểm cao nhất → đụng tường → `BumpRecovery` quẳng nó
  về pha SURGE → kết luận lại → lặp mãi tới hết giờ. Cả ba thuật toán đều mắc.
  Nay va chạm trong pha `RETURN` → dừng luôn tại chỗ (kết luận là **điểm đã ghi**,
  về được tận nơi chỉ là cố gắng thêm).
- **"Kỷ lục mới" không đồng nghĩa với "tiến triển".** `StallGuard` từng reset bộ
  đếm mỗi khi có giá trị cao hơn kỷ lục cũ dù chỉ 1 đếm ADC → trong trường nhiễu
  nó gần như không bao giờ kích hoạt. Nay chỉ reset khi vượt quá `PLATEAU_EPS`.
- **Công cụ phân tích không được âm thầm bỏ dữ liệu.** `analyze.py` từng gộp nhóm
  theo danh sách môi trường viết cứng, nên `SURGE_CAST` (tên có dấu `_`) bị lệch
  khi tách tên file và **mất hẳn khỏi bảng**. Và nó in sai số `0.0` khi thiếu
  `meta.csv` — trông như chính xác hoàn hảo. Cả hai đã sửa.

---

## 6. Nhật ký phiên làm việc

### Phiên 1 — 2026-07-28

**Đã làm**
- Dựng repo từ số 0: `platformio.ini` 3 target (robot / base / sim), khung thư mục.
- `src/core/`: `config.h`, `irobot.h`, `geometry.h`, `gas.*`, `search_common.*`,
  `search_algorithm.*`, ba thuật toán, `mission.*`, `telemetry_fmt.*`.
- `src/sim/`: mô hình plume hai môi trường (khuếch tán / puff đứt quãng),
  mô hình trễ bất đối xứng của MQ-3, động học xe vi sai, sai số dead-reckoning.
- `src/robot/`: TB6612, MPU6050 (đọc thanh ghi trực tiếp), encoder + odometry,
  motion vòng kín, ADC MQ-3, nút/LED RGB/còi, `RobotIO`, `main.cpp`.
- `src/lora/`: SX1262 qua RadioLib, truyền **không chặn**, tự thử TCXO 1.6 V rồi 0 V.
- `src/base/`: nhận, kiểm checksum, in hai dạng máy-đọc và người-đọc, gửi lệnh lên xe.
- `tools/`: `receiver.py`, `analyze.py`, `mq3_fit.py`, `requirements.txt`.
- Chạy 10 lần thử × 3 thuật toán × 2 môi trường trên sim, ra bảng + biểu đồ.

**Quyết định lớn trong phiên** → chi tiết ở `docs/DECISIONS.md`
- Dùng chung code C++ giữa firmware và simulator (người dùng chọn).
- LoRa SPI/RadioLib, PlatformIO, sân 2×2 m ô 25 cm (người dùng chọn).
- Viết lại gradient theo kiểu "thẳng khi tăng, quét khi giảm" — lý do ở mục 5.
- Thêm `RETURN_TO_BEST` cho **cả ba** thuật toán.
- Thêm `STALL_LIMIT_SNIFFS` chống kẹt.
- Gộp giai đoạn SEEK dùng chung cho gradient và surge-casting để so sánh công bằng.

**Đã cài vào máy người dùng:** `matplotlib` (qua pip) để kiểm chứng `analyze.py`.

**Còn nợ / phiên sau**
- Thử toàn bộ trên phần cứng thật; nhiều khả năng phải chỉnh `PWM_MIN_MOVE`,
  `HEADING_KP`, `TURN_KP` theo motor thực tế.
- Hiệu chuẩn `MQ3_RL_OHM`, `R0`, `A`/`B`, `T1..T3`, `DETECT_DELTA`, `STOP_HIGH_DELTA`.
- Đo lại `WHEEL_DIAMETER_MM`, `WHEEL_BASE_MM`, `ENCODER_SLOTS`, `SENSOR_OFFSET_CM`.
- Chưa có bộ test tự động cho `src/core/` (cân nhắc nếu còn thời gian).

### Phiên 2 — 2026-07-30

Bối cảnh: người dùng đang chờ linh kiện giao tới, hỏi còn phải làm gì.

**Đã làm**
- Kiểm thử nhánh **dữ liệu thực** của `analyze.py` (trước đó chỉ chạy `--sim`) bằng
  file CSV giả lập → phát hiện và sửa 2 lỗi: mất dữ liệu `SURGE_CAST`, và in sai số
  `0.0` khi thiếu `meta.csv`. `receiver.py` nay thay `_` thành `-` trong tên file.
- Thêm `src/robot/bench.h/.cpp`: chế độ kiểm tra phần cứng và hiệu chuẩn
  (`mot`, `drive`, `turn`, `enc`, `bump`, `gas`, `sniff`, `ping`). Lấp lỗ hổng:
  `RUNBOOK` yêu cầu đo quãng đường và bảng khoảng cách↔`norm` nhưng firmware
  chưa có lệnh nào làm được.
- Thêm target `env:test` + `src/test/main.cpp`: 86 phép kiểm tra, dùng lại `SimRobot`
  làm robot giả. Nhóm 7 chạy 3 thuật toán × 2 môi trường × 4 seed và bắt buộc mọi
  lần đều phải **kết thúc**.
- Bộ test **bắt được 2 lỗi thật** mà 10 lần chạy mô phỏng trước đó không thấy:
  (a) `StallGuard` reset sai (xem mục 5), (b) pha `RETURN` bị `BumpRecovery` phá
  (xem mục 5). Sửa cả hai → surge-casting ổn định hơn rõ rệt:
  `inter` từ 167±126 s xuống 132±63 s, `diff` từ 146±59 s xuống 103±23 s,
  không còn lần nào chạy hết giờ.
- Sửa lệnh `ping` (trước đó cố tình gửi checksum sai — vô lý); trạm thu nay nhận
  biết gói `$PING` và in RSSI/SNR.
- `RUNBOOK.md`: thêm mục 0.5 "làm gì trong lúc chờ linh kiện", bảng lệnh `bench`,
  mục bộ test; mục 3 và 4.4 nay dùng lệnh có sẵn thay vì mô tả suông.

- Cho phép ghi đè `ARENA_W_CM` / `ARENA_H_CM` / `CELL_CM` / `MISSION_TIMEOUT_S` bằng
  `-D` lúc build (mặc định không đổi), rồi quét thử 7 cỡ sân để trả lời câu hỏi
  "robot quét được vùng tối đa bao nhiêu". Bảng kết quả ở `RUNBOOK.md` mục 5.
  Kết luận: trần thật là **sai số dead-reckoning (~0,5–0,8 % quãng đường)**, không
  phải thời gian — khoảng **3 × 3 m** với ô 25 cm. Mặc định 2 × 2 m vẫn là lựa chọn tốt.

**Ghi chú cho phiên sau**
- Số liệu mô phỏng ở mục 4 đã đổi sau khi sửa 2 lỗi trên. Nếu đã trích số cũ vào
  báo cáo thì phải chạy lại `./.pio/build/sim/program --trials 10 --traj`.
- Vẫn chưa có gì chạy trên phần cứng thật. Mọi hằng số `[DO]` vẫn là phỏng đoán.
- `receiver.py` vẫn chưa test với cổng Serial thật (cần có board).

---

### Phiên 3 — 2026-07-31

Bối cảnh: người dùng đã có đủ linh kiện, xin hướng dẫn đầy đủ từ lắp xe tới báo cáo.

**Đã làm**
- Thêm lệnh `selftest` vào `src/robot/bench.cpp`: kiểm tra lần lượt I2C/MPU6050, gyro
  lúc đứng yên, ADC của MQ-3, công tắc va chạm, LoRa, rồi quay **từng bánh một** và
  đếm xung encoder → in bảng `DAT/CANH/HONG`. Bắt được cả trường hợp hai dây encoder
  cắm lẫn nhau (quay bánh trái mà bánh phải cũng có xung).
- Viết lại `docs/RUNBOOK.md` thành hướng dẫn tuyến tính 10 giai đoạn (0→9), mỗi giai
  đoạn có **tiêu chí đạt**. Bổ sung phần lắp cơ khí và mạch nguồn (trước đây thiếu hẳn,
  vì ban đầu người dùng nói tự lo phần lắp ráp).
- Ba nguyên tắc an toàn đặt lên đầu: LoRa không cắm 5 V · không cấp điện LoRa khi chưa
  gắn ăng-ten · đo buck trước khi nối vào board.

**Ghi chú cho phiên sau**
- Người dùng đang ở giai đoạn lắp. Việc hay hỏng nhất sẽ là: encoder lệch khe, thiếu
  tụ 100 nF trên motor, và ba ngưỡng ở mục 6.4.
- Khi có số đo thật, nhớ cập nhật `config.h` **rồi chạy lại `pio run -e test`**.

---

### Phiên 4 — 2026-08-14

Bối cảnh: người dùng đang lắp xe, yêu cầu dò lại repo GitHub (đã có commit mới) và
soạn nội dung báo cáo giữa kỳ.

**Phát hiện quan trọng — phần cứng thật KHÁC thiết kế ban đầu**

Kéo `origin/main` về thấy thư mục `docs/wiring/` mới (sơ đồ Mermaid theo từng cụm).
Đọc kỹ thì cấu hình thật là:

| | Firmware giả định | Phần cứng thật |
|---|---|---|
| Motor | 2 motor, 1 TB6612 | **4 motor, 2 TB6612 dùng chung dây điều khiển** |
| Encoder | 2 (GPIO1 + GPIO2) | **1 (HC-020K, GPIO1, bánh sau trái)** |
| Bumper | 2 (GPIO38 + 39) | **1 (V156, GPIO38)** |

Phần motor không ảnh hưởng firmware (hai driver song song cùng tín hiệu → vẫn là xe
vi sai hai bên). Nhưng **một encoder** gây hai lỗi nếu để nguyên:
1. `d = 0.5*(dl+dr)` với `dr` luôn bằng 0 → **quãng đường bị chia đôi**.
2. Khi quay tại chỗ, bánh trái vẫn quay → robot **tưởng mình vừa đi lùi** một đoạn.

**Đã sửa**
- `config.h`: thêm `ENCODER_COUNT` / `BUMPER_COUNT` / `IMU_REQUIRED` (=1/1/true).
- `odometry.cpp`: chỉ gắn ngắt cho encoder có thật; quãng đường lấy trực tiếp từ
  encoder trái; **`dir_l_ != dir_r_` (quay tại chỗ) → `d = 0`** — sửa đúng lỗi 2 ở trên,
  và cách sửa này đúng cho cả cấu hình 1 lẫn 2 encoder.
- `hw_io.cpp`: không đọc chân bumper phải khi không lắp.
- `bench.cpp`: `selftest` và báo cáo `drive` thích ứng với cấu hình một encoder;
  bước kiểm tra motor phải chuyển thành "xác nhận bằng mắt".
- `main.cpp`: MPU6050 thành **bắt buộc** (một encoder thì không suy được góc);
  chặn `start` và in cảnh báo lớn nếu thiếu.
- Cho phép ghi đè kích thước sân bằng `-D` lúc build.

**Đã soạn** `docs/BaoCaoGiuaKy.md` — nội dung báo cáo giữa kỳ đầy đủ: giới thiệu,
lý do, vấn đề giải quyết, sơ đồ khối, sơ đồ nguồn, ba giải thuật (Mermaid), tiến độ,
hạn chế, hướng phát triển. Hình minh hoạ copy vào `docs/img/` (đã bỏ khỏi `.gitignore`).

**Đã soạn thêm (cùng phiên)** — người dùng báo thời gian sắp cạn, reviewer hỏi sâu:
- `docs/GIAI_THICH_KY_THUAT.md` — 22 mục, giải thích **vì sao** từng quyết định:
  vì sao ESP32-S3 (ADC 12 bit là điểm quyết định), vì sao 2 buck riêng (sụt áp làm
  sợi đốt MQ-3 hạ nhiệt → tương quan giả giữa chuyển động và nồng độ), nguyên lý SnO2,
  vì sao ppm khuếch đại nhiễu 1,87 lần, và **giải thích surge-casting thật kỹ**
  (sợi khí rời rạc → gradient vô nghĩa → nhưng "ngửi thấy khí thì nguồn ở đầu gió"
  luôn đúng vì là ràng buộc nhân quả).
- `docs/PHAN_BIEN.md` — 22 câu hỏi xoáy + câu trả lời, kèm "ba câu cần thuộc lòng".
  Câu khó nhất đã trả lời thẳng: "mô hình mô phỏng do chính em viết thì chứng minh
  được gì" → thừa nhận nó KHÔNG chứng minh được ba giả thuyết.
- `docs/SLIDES.md` — dàn ý 16 slide + 6 slide dự phòng + prompt đầy đủ cho
  claude.com/design (khuyến nghị hơn Canva vì hình vẽ ở đây LÀ nội dung).

**Ghi chú cho phiên sau**
- Nếu sau này lắp thêm encoder thứ hai: chỉ cần đổi `ENCODER_COUNT = 2`, mọi chỗ khác
  tự thích ứng.
- Vì chỉ có encoder bên TRÁI, nếu bánh trái trượt thì quãng đường sai hệ thống. Lúc
  hiệu chuẩn `drive 100` nhớ chạy trên đúng mặt sàn sẽ dùng khi đo thật.

---

## 7. Quy ước làm việc với người dùng

1. Bất cứ giá trị/hướng nào Claude **tự chọn** đều phải ghi vào `docs/DECISIONS.md`
   và đánh dấu `[CHON]` ngay tại chỗ trong `config.h`.
2. Mỗi phiên làm việc thêm một mục vào **mục 6** của file này.
3. Có câu hỏi thì **hỏi ngay trong lượt đang làm** (dùng công cụ hỏi), không dùng
   giá trị giả rồi hỏi ở lượt sau.
