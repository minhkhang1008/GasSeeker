# Những lựa chọn tôi tự chốt

Theo quy tắc làm việc: **mọi thứ tự ý lựa chọn đều phải note lại**. File này liệt
kê đầy đủ, kèm lý do và cách đổi lại nếu bạn không đồng ý.

Ba mức đánh dấu dùng trong `src/core/config.h`:

| Nhãn | Nghĩa |
|---|---|
| `[CHOT]` | Đã có trong đề cương, tôi không tự đổi |
| `[CHON]` | **Tôi tự chọn** — liệt kê trong file này |
| `[DO]` | Bắt buộc đo/hiệu chuẩn lại trên phần cứng thật |

---

## A. Bạn đã chốt (không phải tôi chọn)

| Hạng mục | Lựa chọn |
|---|---|
| Giao tiếp LoRa | SPI, chip SX1262, dùng thư viện RadioLib |
| Môi trường build | PlatformIO |
| Simulator | Có, dùng chung code C++ với firmware |
| Kích thước sân | Chưa biết → mặc định 2×2 m, ô 25 cm |

---

## B. Kiến trúc

### B1. Tách `src/core/` thành lớp thuần C++ không phụ thuộc Arduino
Thuật toán chỉ gọi `gs::IRobot`. Nhờ vậy **cùng một file `.cpp`** chạy trên ESP32 và
trên máy tính. Không có chuyện "bản sim khác bản thật".
*Đổi lại:* không, đây là nền của cả dự án.

### B2. Ba thuật toán là ba lớp con của `SearchAlgorithm`, cấp phát tĩnh
`makeAlgorithm()` trả về con trỏ tới đối tượng `static` — không dùng heap trên MCU
(tránh phân mảnh bộ nhớ trong chương trình chạy dài).

### B3. Lớp `Mission` đo thời gian và quãng đường, dùng chung xe + sim
Đảm bảo hai bên đo **giống hệt nhau**, không lệch định nghĩa chỉ số.

### B4. Driver MPU6050 viết tay ~80 dòng thay vì dùng thư viện
Robot chỉ cần **một** đại lượng: gyro Z. Kéo cả thư viện về cho một thanh ghi là
thừa và thêm rủi ro phụ thuộc.
*Đổi lại:* thay `src/robot/hw_imu.cpp`, giữ nguyên 4 hàm public.

### B5. LoRa truyền **không chặn**
`radio.transmit()` của RadioLib chặn ~200 ms ở SF9. Xe chạy 18 cm/s sẽ đi lố ~3,6 cm
mỗi gói tin. Dùng `startTransmit()` + cờ ngắt DIO1.

### B6. Firmware tự thử TCXO 1,6 V rồi 0 V khi khởi tạo LoRa
Ra-01SH có bản dùng TCXO, có bản dùng thạch anh thường. Thử cả hai để không phải
đoán, và in ra kết quả.

---

## C. Xử lý tín hiệu khí

### C1. Baseline đo **một lần** lúc khởi động, 5 giây, không tự trôi theo
Baseline trôi (adaptive) sẽ "nuốt" mất tín hiệu khi robot ở lâu trong vùng có khí.
Đổi lại: `cal` trên Serial để đo lại giữa buổi. `BASELINE_MS = 5000`.

### C2. `R0` **tự hiệu chuẩn mỗi lần bật máy** từ chính giai đoạn baseline
`MQ3_R0_OHM = -1` → firmware tính `R0 = Rs_khong_khi_sach / 60`. Datasheet nói `R0`
trôi theo thời gian, hiệu chuẩn lại mỗi buổi là đúng khuyến cáo.
*Đổi lại:* đặt `MQ3_R0_OHM` thành số dương đo được để cố định.  

### C3. Hằng số đường đặc tuyến `A = 315.2`, `B = -1.865`
Fit từ **hai điểm đọc thô** trên đồ thị ethanol của datasheet MQ-3:
(0,1 mg/L, Rs/R0 = 2,6) và (10 mg/L, Rs/R0 = 0,22), quy đổi 1 mg/L ≈ 531 ppm ở 25 °C.
**Đây là ước lượng, không phải số liệu từ datasheet module bạn mua.**
*Bắt buộc fit lại:* `python3 tools/mq3_fit.py fit --p1 <ppm> <ratio> --p2 <ppm> <ratio>`

### C4. Ngưỡng cảnh báo mặc định `T1/T2/T3 = 100 / 500 / 1500 ppm`
Chọn tạm để có gì đó hiển thị. Ngày 4 phải đo lại: `tools/mq3_fit.py levels --csv ...`.
Trong sản phẩm thật với khí độc, ba ngưỡng này phải đặt theo TLV-TWA / IDLH.

### C5. "Dừng ngửi" — mọi phép đo đều dừng hẳn xe
`SNIFF_SETTLE_MS = 1500` (bỏ đoạn quá độ) + `SNIFF_AVG_MS = 800` (lấy trung bình).
Đọc trong lúc xe đang chạy thì giá trị ứng với vị trí *đã đi qua*, không ứng với
vị trí nào cả. Đây là hệ quả trực tiếp của trễ MQ-3.
*Đánh đổi:* mỗi phép đo tốn 2,3 s → quét toàn bộ 64 ô mất ~4,5 phút.

### C6. Thời gian giữ điều kiện dừng `STOP_HOLD_MS = 6000`
Tương đương khoảng **2 chu kỳ đo liên tiếp** không cải thiện. Ngắn hơn thì một lần
đọc nhiễu cũng làm robot dừng sớm.

---

## D. Thuật toán

### D1. Gradient: **đi thẳng khi còn tăng, chỉ quét 3 hướng khi giảm**
Đề cương nêu hai biến thể; bản cài đặt ghép cả hai. Lý do là vật lý chứ không phải
thẩm mỹ: quét 3 hướng liên tục thì ba phép đo cách nhau vài giây trong khi cảm biến
hồi phục mất ~30 s → ba số liệu lệch theo **thứ tự đo** chứ không theo không gian.
Đi thẳng thì hai phép đo cách nhau 30 cm, chênh lệch không gian át được trễ.

### D2. `RETURN_TO_BEST = true` cho **cả ba** thuật toán
Do trễ cảm biến, robot luôn vượt qua đỉnh rồi mới nhận ra. Khi thoả điều kiện dừng,
robot quay lại điểm đo cao nhất rồi mới báo "đã tìm thấy".
*Đo được trên sim:* tắt tính năng này, sai số gradient tăng từ ~22 cm lên ~52 cm.
Thời gian định vị có tính cả quãng quay lại — đúng và trung thực.

### D3. Giai đoạn SEEK dùng **chung** cho gradient và surge-casting
Khi chưa hề phát hiện khí, cả hai đều quét thô zig-zag cách `SEEK_STRIDE = 2` ô
cho tới khi bắt được tín hiệu đầu tiên. Lý do: nếu để mỗi thuật toán tự xoay xở
lúc chưa có tín hiệu thì bảng so sánh sẽ đo lẫn cả "khả năng dò mù", trong khi
điều H1/H2 nói tới là hành vi **sau khi** bắt được luồng khí.
*Phải ghi rõ điều này trong báo cáo.*

### D4. `STALL_LIMIT_SNIFFS = 12` — chống kẹt
Sau 12 phép đo liên tiếp không lập kỷ lục mới, robot kết luận bằng điểm cao nhất
đã đo thay vì chạy tới hết giờ. Không có nó, surge-casting kẹt vòng lặp vô hạn ở
góc sân (đã gặp thật khi chạy mô phỏng).

### D4b. `StallGuard` chỉ tính là "tiến triển" khi vượt kỷ lục quá `PLATEAU_EPS`
Ban đầu tôi reset bộ đếm chống kẹt mỗi khi có giá trị cao hơn kỷ lục cũ. Trong
trường nhiễu, giá trị cao hơn 1 đếm ADC xuất hiện liên tục nên bộ đếm không bao
giờ đầy → robot chạy tới hết giờ. Nay dùng chung ngưỡng `PLATEAU_EPS` với mọi chỗ
khác. Việc *ghi lại* điểm cao nhất vẫn dùng ngưỡng 0 (chính xác tuyệt đối) — hai
khái niệm này tách biệt, xem chú thích của `StallGuard`.

### D4c. Pha `RETURN` là pha cuối — va chạm không được đưa robot về tìm kiếm
Nếu đụng tường trên đường quay về điểm cao nhất, robot dừng luôn tại chỗ và giữ
kết luận đã ghi. Trước khi sửa, `BumpRecovery` đưa nó về pha SURGE và tạo vòng lặp
kết luận↔tìm lại cho tới hết giờ.

### D5. Quét toàn bộ **không** dừng sớm theo nồng độ
Nó luôn quét hết lưới. Cho nó dừng sớm thì không còn là baseline nữa và mất ý nghĩa
so sánh cho H3.

### D6. Hướng gió khai báo cứng `WIND_FROM_DEG = 0` (gió thổi từ phía +X về gốc)
Đúng theo đề cương mục 11.2 — robot **không** tự đo hướng gió. Đặt quạt ở cạnh +X
của sân, nguồn ethanol ở giữa sân về phía +X.

### D7. Phản ứng va chạm: lùi 12 cm rồi quay ±75°, đổi bên mỗi lần
Đơn giản, không cần bản đồ vật cản (đề cương xếp tránh vật cản vào mục "Không làm").

---

## E. Chuyển động và định vị

### E1. Hướng lấy từ **gyro MPU6050**, quãng đường lấy từ **encoder**
Lấy hướng bằng hiệu số xung hai bánh sai nhiều vì bánh trượt khi quay tại chỗ.
Nếu không tìm thấy MPU6050, firmware **tự động** quay về dùng hiệu số encoder và
in cảnh báo.

### E2. Encoder một kênh — chiều quay suy từ lệnh đang cấp cho motor
Đúng như đề cương mục 19.2. Hệ quả: nếu bánh bị kẹt và quay ngược do quán tính,
odometry sẽ sai. Chấp nhận được ở quy mô đề tài.

### E3. Chống nhiễu xung encoder: bỏ qua xung cách nhau dưới 1,5 ms
Ở tốc độ làm việc, hai xung thật cách nhau > 20 ms. Ngưỡng 1,5 ms lọc nhiễu điện
từ motor mà không mất xung thật.

### E4. `PWM_DRIVE = 150`, `PWM_TURN = 130`, `PWM_MIN_MOVE = 70`
Giá trị khởi điểm an toàn cho motor TT với nguồn 6 V. **Phải chỉnh lại** sau khi lắp:
tăng `PWM_MIN_MOVE` cho tới khi xe chớm quay được từ trạng thái đứng yên.

---

## F. Truyền thông

### F1. Tần số 923,0 MHz, SF9, BW 125 kHz, CR 4/5, công suất 14 dBm
Băng SRD Việt Nam là **920–925 MHz**, giới hạn ~25 mW EIRP = 14 dBm. Tôi chọn giữa
băng và đúng mức công suất cho phép. SF9 để có dự phòng cự ly; đổi xuống SF7 nếu
muốn gói tin nhanh hơn (~85 ms thay vì ~205 ms).

### F2. Gói tin gửi dạng CSV có checksum, trạm thu in ra **cả hai** dạng
Trên sóng gửi `$GS,...*HH` cho máy đọc. Trạm in thêm dòng người-đọc đúng mẫu đề
cương mục 11.4. Checksum XOR kiểu NMEA để phát hiện gói hỏng.

### F3. Cho phép trạm gửi lệnh **lên** xe (`ENABLE_UPLINK = true`)
Đề cương nói LoRa chỉ để giám sát. Tôi vẫn thêm đường lệnh lên vì hai lý do thực tế:
bấm chạy từ xa khi quay video, và **dừng khẩn từ xa**. Robot vẫn chạy hoàn toàn độc
lập nếu mất sóng. Đặt `ENABLE_UPLINK = false` để tắt hẳn.

---

## G0. Chế độ kiểm tra phần cứng (`bench`)

Tôi tự thêm `src/robot/bench.h/.cpp` — không có trong đề cương. Lý do: đề cương
Ngày 2–4 yêu cầu đo quãng đường thật, kiểm tra encoder, và dựng bảng
khoảng cách↔`norm`, nhưng nếu chỉ có firmware chạy thuật toán thì không có cách
nào làm những việc đó ngoài việc nạp code tạm rồi xoá. Chế độ `bench` biến các
bước hiệu chuẩn thành lệnh gõ được, và tách hẳn khỏi đường chạy thuật toán
(hai bên không bao giờ cùng giành motor).

Lệnh `gas` in ra các cột **cách nhau bằng TAB** để dán trực tiếp vào bảng tính —
đường đặc tuyến mà đề cương yêu cầu ở Ngày 4 vẽ được ngay từ đó.

---

## G. Sơ đồ chân và phần cứng

### G1. Dùng nút BOOT (GPIO0) làm nút điều khiển
Nhấn ngắn = Start/Stop, nhấn giữ = đổi thuật toán. Tiết kiệm 2 chân GPIO và không
phải hàn thêm nút. Bù lại: không bấm được trong lúc board đang khởi động.

### G2. Dùng LED RGB có sẵn trên DevKitC-1 (GPIO48) làm đèn cảnh báo
Hiển thị đúng bảng màu đề cương mục 11.1c. Nếu board của bạn không có LED này, đặt
`UI_USE_RGB_LED = false` và `UI_USE_DISCRETE_LEDS = true` rồi hàn LED vào GPIO40/41.

### G3. Tránh GPIO 26–37 hoàn toàn
Bản ESP32-S3 N16R8 dùng octal PSRAM chiếm GPIO33–37, flash chiếm 26–32. Sơ đồ chân
trong `docs/WIRING.md` né hết. Đây là lỗi rất hay gặp và rất khó tìm ra.

---

## H0. Bộ test tự động (`env:test`)

Tôi tự thêm, không có trong đề cương. 86 phép kiểm tra chạy native trong ~1 giây,
dùng lại `SimRobot` làm robot giả nên không cần viết mock riêng.

Lý do đáng làm dù đang gấp: Ngày 3–4 bạn sẽ sửa rất nhiều hằng số trong `config.h`
theo số đo thật. Một ngưỡng đặt sai có thể làm thuật toán **không bao giờ dừng** —
và bạn chỉ phát hiện ra sau khi đã mất một buổi đo. Nhóm test số 7 chạy cả ba thuật
toán trên 24 tình huống và bắt buộc chúng phải kết thúc; chính nó đã bắt được hai
lỗi thật mà 10 lần chạy mô phỏng không thấy (xem D4b và D4c).

---

## H. Mô phỏng

### H1. Mọi tham số trong `src/sim/sim_config.h` là **giả định của tôi**
Chúng được chọn để tạo ra một trường nồng độ có dạng hình học và động học hợp lý,
đủ để so sánh **hành vi** ba thuật toán. Chúng **không** phải số liệu đo được.
Báo cáo phải để kết quả mô phỏng ở mục riêng, không trộn với số liệu thực nghiệm.

### H2. Mô hình cảm biến có trễ **bất đối xứng**: lên 2,5 s, xuống 8 s
Datasheet MQ nói response ≤ 10 s, recovery ≤ 30 s. Đây là chi tiết quan trọng nhất
của mô hình — chính nó buộc phải có "dừng ngửi" và `RETURN_TO_BEST`.

### H3. Sim mô phỏng cả **sai số dead-reckoning**
Giữ song song hai vị trí: vị trí thật (để chấm điểm) và vị trí robot *tin* (dead
reckoning, có sai số tỉ lệ ±1,5 % và trôi gyro ±0,06 °/s). Thuật toán chỉ thấy cái
thứ hai. Nhờ vậy con số sai số trên sim không bị đẹp giả tạo.

### H4. Cùng một seed → cùng vị trí nguồn cho cả ba thuật toán
So sánh công bằng: ba thuật toán gặp đúng cùng một bài toán.
