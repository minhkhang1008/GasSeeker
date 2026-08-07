# Sơ đồ đấu nối — GasSeeker

Tài liệu này mô tả **đúng cấu hình phần cứng thực tế đang dùng để build xe**.
Nguồn chân firmware vẫn lấy từ `namespace cfg::pin` trong `src/core/config.h`.
Nếu đổi GPIO thì sửa `config.h` trước, sau đó cập nhật lại tài liệu này.

## Cấu hình phần cứng đã chốt

- ESP32-S3 DevKitC-1 trên xe
- 4 × động cơ vàng V1 + bánh 65 mm
- 2 × TB6612FNG
- 1 × encoder quang HC-020K + 1 đĩa encoder
- 1 × MQ-3
- 1 × công tắc hành trình V156 làm bumper phía trước
- 1 × SX1262 / Ra-01SH LoRa trên xe
- Pin 2S: 2 × 18650 nối tiếp
- XL4015 #1: hạ xuống 6,0 V cho motor
- LM2596: hạ xuống 5,0 V cho ESP32 + MQ-3
- Cầu chì 5 A + công tắc KCD1
- **Không lắp MPU6050**

> Lưu ý: firmware gốc ban đầu được viết cho 2 encoder và có hỗ trợ MPU6050.
> Với phần cứng hiện tại chỉ có 1 encoder và không có MPU6050, phần odometry/motion
> phải chạy theo cấu hình single-encoder tương ứng. Không được để firmware hiểu
> GPIO2 là encoder phải trong khi chân này thực tế không nối gì.

---

## Chân KHÔNG được dùng trên ESP32-S3

| Chân | Lý do |
|---|---|
| GPIO 0 | Nút BOOT (strapping) — dùng làm nút bấm onboard, chấp nhận được |
| GPIO 3, 45, 46 | Strapping, mức lúc khởi động ảnh hưởng cách boot |
| GPIO 19, 20 | USB D− / D+ |
| GPIO 26–32 | SPI flash trong |
| GPIO 33–37 | Octal PSRAM trên bản N16R8 |
| GPIO 43, 44 | UART0 TX/RX (cổng debug) |

---

# 1. Nguồn tổng

## Sơ đồ nguồn

```text
2S 18650 (6,0–8,4 V)
        +
        │
   Cầu chì 5 A
        │
   Công tắc KCD1
        │
        ├──────────────────► XL4015 #1
        │                       │
        │                     6,0 V
        │                       │
        │              ┌────────┴────────┐
        │              │                 │
        │          TB6612 #1 VM      TB6612 #2 VM
        │              │                 │
        │             FL, FR            RL, RR
        │
        └──────────────────► LM2596
                                │
                              5,0 V
                                │
                         ┌──────┴──────┐
                         │             │
                    ESP32 5V/VIN     MQ-3 VCC

ESP32 3V3
   │
   ├── TB6612 #1 VCC
   ├── TB6612 #2 VCC
   ├── LoRa VCC
   └── HC-020K VCC (ưu tiên chạy 3,3 V)
```

**Tất cả các khối phải dùng chung GND.**

### Trước khi cắm bất kỳ module nào

1. Tháo ESP32, MQ-3, LoRa và TB6612 khỏi đường nguồn.
2. Bật pin.
3. Chỉnh XL4015 #1 tới **6,00 V ± 0,1 V**.
4. Chỉnh LM2596 tới **5,00 V ± 0,1 V**.
5. Đo lại tại đúng đầu dây sẽ cắm vào module.
6. Tắt nguồn rồi mới đấu board.

### Tụ nguồn / chống nhiễu

Nếu có đúng các giá trị sau trong bộ tụ hiện có thì lắp:

- 100 nF trực tiếp qua hai cực **mỗi motor**.
- 470–1000 µF gần đường VM của hai TB6612.
- 220–470 µF gần ESP32.
- 220–470 µF + 100 nF sát nguồn LoRa.
- 100 nF từ GPIO4 xuống GND tại mạch đọc MQ-3.

Không cần cố dùng một giá trị không có sẵn chỉ để đúng con số trong tài liệu;
ưu tiên tụ phù hợp gần nhất trong bộ linh kiện hiện có.

---

# 2. Hai TB6612FNG điều khiển bốn motor

Xe dùng điều khiển vi sai:

```text
LEFT  = Front Left + Rear Left
RIGHT = Front Right + Rear Right
```

Mỗi motor dùng một cầu H riêng. **Không ghép hai motor vào chung một output cầu H.**

## Phân motor

### TB6612 #1 — hai motor phía trước

| Kênh | Motor |
|---|---|
| A01/A02 | Front Left (FL) |
| B01/B02 | Front Right (FR) |

### TB6612 #2 — hai motor phía sau

| Kênh | Motor |
|---|---|
| A01/A02 | Rear Left (RL) |
| B01/B02 | Rear Right (RR) |

## Tín hiệu điều khiển

Hai TB6612 nhận **chung tín hiệu logic** từ ESP32:

| ESP32-S3 | TB6612 #1 | TB6612 #2 | Chức năng |
|---|---|---|---|
| GPIO 5 | PWMA | PWMA | PWM bên trái |
| GPIO 6 | AIN1 | AIN1 | chiều bên trái |
| GPIO 7 | AIN2 | AIN2 | chiều bên trái |
| GPIO 15 | PWMB | PWMB | PWM bên phải |
| GPIO 16 | BIN1 | BIN1 | chiều bên phải |
| GPIO 17 | BIN2 | BIN2 | chiều bên phải |
| GPIO 18 | STBY | STBY | LOW = tắt motor |
| 3V3 | VCC | VCC | nguồn logic |
| 6,0 V | VM | VM | nguồn motor |
| GND | GND | GND | mass chung |

Như vậy firmware vẫn chỉ cần phát hai lệnh `LEFT` và `RIGHT`, nhưng cả bốn motor
đều được điều khiển.

### Kiểm tra chiều motor

Kê xe để bánh không chạm đất rồi test lần lượt:

```text
mot 80 0      → cả FL và RL phải quay theo chiều làm xe tiến
mot 0 80      → cả FR và RR phải quay theo chiều làm xe tiến
mot 80 80     → cả 4 bánh tiến
mot -80 -80   → cả 4 bánh lùi
mot -80 80    → quay trái tại chỗ
mot 80 -80    → quay phải tại chỗ
```

Nếu chỉ **một motor** quay ngược, đảo hai dây motor tại output của chính motor đó.
Không đảo AIN/BIN nếu motor còn lại cùng bên đang quay đúng.

---

# 3. Encoder quang HC-020K — chỉ một encoder

Phần cứng hiện tại có **1 module HC-020K + 1 đĩa encoder**.

Khuyến nghị lắp tại **bánh sau trái (RL)** vì dễ đi dây và tránh MQ-3/bumper phía trước.
Nếu cơ khí thuận lợi hơn ở bánh khác thì có thể đổi, nhưng firmware phải biết encoder
đang đại diện cho bên nào.

| HC-020K | ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| OUT | GPIO 1 |

**GPIO2 không nối encoder.**

Ưu tiên cấp HC-020K bằng **3,3 V** để OUT không vượt mức an toàn của GPIO ESP32.
Nếu module cụ thể chỉ hoạt động ổn ở 5 V thì OUT phải được chia áp về 3,3 V trước
khi nối vào ESP32.

### Đĩa encoder

- Đĩa phải đi xuyên giữa khe quang của HC-020K.
- Đĩa không được cọ vào cảm biến.
- Đếm **số khe thực tế** trên chính đĩa đang dùng rồi cập nhật `ENCODER_SLOTS`.
- Không mặc định 20 khe nếu chưa kiểm tra.

### Ý nghĩa khi chỉ có một encoder

- Khi đi thẳng: dùng encoder để ước lượng quãng đường đã đi.
- Khi quay tại chỗ: dùng quãng đường bánh encoder để ước lượng góc quay sau hiệu chuẩn.
- Không thể dùng hiệu `encoder trái - encoder phải` để xác định heading như cấu hình hai encoder.
- Vì không có MPU6050, giữ hướng khi đi thẳng phải dựa vào cân chỉnh motor/PWM thực nghiệm.

---

# 4. Cảm biến khí MQ-3

| ESP32-S3 | Nối tới |
|---|---|
| GPIO 4 (ADC1_CH3) | điểm giữa mạch chia áp |

```text
MQ-3 AO ──[ 10 kΩ ]──┬── GPIO4
                     │
                 [ 20 kΩ ]
                     │
                    GND

GPIO4 ── 100 nF ── GND   (nếu có tụ phù hợp)
```

- MQ-3 VCC → **5,0 V từ LM2596**.
- MQ-3 GND → GND chung.
- Không nối AO trực tiếp vào GPIO4 nếu AO có thể lên tới khoảng 5 V.
- Hệ số chia áp: `20 / (10 + 20) = 0,667`, nên 5 V được hạ về khoảng 3,33 V.
- Nếu đổi giá trị điện trở thì phải đổi `DIV_R_TOP_OHM` và `DIV_R_BOT_OHM` trong `config.h`.
- Đo lại điện trở tải thật trên module và cập nhật `MQ3_RL_OHM` trước khi dùng ppm để hiển thị.

Đặt MQ-3 ở phía trước xe, tương đối thấp và tránh luồng khí nóng trực tiếp từ mạch nguồn.
Dây AO nên đi xa dây motor và đường VM.

---

# 5. LoRa SX1262 / Ra-01SH

| ESP32-S3 | SX1262 / Ra-01SH |
|---|---|
| GPIO 11 | MOSI |
| GPIO 13 | MISO |
| GPIO 12 | SCK |
| GPIO 10 | NSS / CS |
| GPIO 14 | RST |
| GPIO 21 | BUSY |
| GPIO 47 | DIO1 |
| 3V3 | VCC |
| GND | GND |

### Bắt buộc

- LoRa chỉ cấp **3,3 V**, tuyệt đối không cấp 5 V.
- Gắn antenna **trước khi cấp nguồn**.
- Đặt tụ 220–470 µF + 100 nF sát chân nguồn nếu có.
- Đặt module/antenna xa motor, TB6612 và dây nguồn công suất nhất có thể.
- Antenna ưu tiên dựng thẳng đứng khi chạy thử ngoài thực tế.

Firmware hiện dùng DIO2 nội bộ của SX1262 để điều khiển RF switch; không cần nối
TXEN/RXEN riêng vào ESP32 theo sơ đồ này.

---

# 6. Công tắc va chạm — một bumper

Phần cứng hiện chỉ có **1 công tắc hành trình V156**.
Dùng nó làm bumper chung ở giữa đầu xe.

| Công tắc | ESP32-S3 |
|---|---|
| COM | GND |
| NO | GPIO 38 |

Firmware dùng `INPUT_PULLUP`:

```text
không va chạm → GPIO38 = HIGH
va chạm       → GPIO38 = LOW
```

**GPIO39 không sử dụng.**

Nếu switch đang có ký hiệu `COM / NO / NC`, dùng `COM + NO`, không dùng `NC`.

---

# 7. Giao diện onboard

| ESP32-S3 | Chức năng |
|---|---|
| GPIO 0 | nút BOOT onboard — nhấn ngắn Start/Stop, giữ để đổi thuật toán |
| GPIO 48 | LED RGB onboard |
| GPIO 42 | buzzer tùy chọn nếu thực tế có lắp |
| GPIO 40, 41 | LED rời tùy chọn, mặc định không dùng |

Không có buzzer/LED rời trong bộ linh kiện hiện tại thì để các chân tương ứng trống.

---

# 8. Các chân đang dùng trên xe

| GPIO | Thiết bị | Ghi chú |
|---:|---|---|
| 0 | BOOT button | onboard |
| 1 | HC-020K OUT | encoder duy nhất |
| 2 | Không dùng | không có encoder phải |
| 4 | MQ-3 AO | qua divider 10k/20k |
| 5 | TB6612 PWMA ×2 | LEFT PWM |
| 6 | TB6612 AIN1 ×2 | LEFT direction |
| 7 | TB6612 AIN2 ×2 | LEFT direction |
| 8 | Không dùng | MPU6050 không lắp |
| 9 | Không dùng | MPU6050 không lắp |
| 10 | LoRa NSS | SPI |
| 11 | LoRa MOSI | SPI |
| 12 | LoRa SCK | SPI |
| 13 | LoRa MISO | SPI |
| 14 | LoRa RST | |
| 15 | TB6612 PWMB ×2 | RIGHT PWM |
| 16 | TB6612 BIN1 ×2 | RIGHT direction |
| 17 | TB6612 BIN2 ×2 | RIGHT direction |
| 18 | TB6612 STBY ×2 | dùng chung |
| 21 | LoRa BUSY | |
| 38 | Bumper | switch xuống GND |
| 39 | Không dùng | không có bumper thứ hai |
| 47 | LoRa DIO1 | |
| 48 | RGB LED | onboard |

---

# 9. Bố trí dây khuyến nghị

Tách thành hai nhóm:

```text
DÂY CÔNG SUẤT
- pin
- fuse
- switch
- XL4015
- VM TB6612
- dây motor

DÂY TÍN HIỆU
- MQ-3 AO
- encoder OUT
- LoRa SPI
- bumper
```

Không bó dây tín hiệu chung với dây motor nếu có thể tránh được.
Đặc biệt MQ-3 AO và encoder OUT nên cách xa dây motor.

PCB 7×9 cm có thể dùng làm điểm phân phối:

- BAT+
- GND
- 6 V motor
- 5 V logic/MQ-3
- 3V3 logic
- mạch chia áp MQ-3
- domino kết nối các cụm tháo rời

---

# 10. Checklist trước khi cấp điện lần đầu

## Cơ khí

- [ ] 4 bánh quay tự do, không cạ khung.
- [ ] Đĩa encoder không chạm HC-020K.
- [ ] Bumper có thể bị nhấn khi đầu xe đụng vật cản.
- [ ] Antenna LoRa đã gắn.

## Nguồn

- [ ] Fuse 5 A nằm nối tiếp ngay sau cực dương pack pin.
- [ ] KCD1 cắt được nguồn toàn xe.
- [ ] XL4015 đo được 6,00 ± 0,1 V trước khi cắm TB6612.
- [ ] LM2596 đo được 5,00 ± 0,1 V trước khi cắm ESP32/MQ-3.
- [ ] Không có LoRa nào nối 5 V.
- [ ] GND giữa pin, buck, ESP32, TB6612, MQ-3, encoder và LoRa thông nhau.

## Tín hiệu

- [ ] GPIO1 nối HC-020K OUT.
- [ ] GPIO2 để trống.
- [ ] GPIO38 nối NO của bumper; COM xuống GND.
- [ ] GPIO39 để trống.
- [ ] MQ-3 AO qua 10k/20k trước GPIO4.
- [ ] Không có dây nào dùng GPIO26–37.

## Motor

- [ ] TB6612 #1 điều khiển FL/FR.
- [ ] TB6612 #2 điều khiển RL/RR.
- [ ] Hai TB6612 dùng chung các tín hiệu GPIO5/6/7/15/16/17/18.
- [ ] Mỗi motor nằm trên một output cầu H riêng.
- [ ] Có tụ 100 nF qua hai cực motor nếu có sẵn đúng loại.

---

# 11. Trình tự test sau khi đấu xong

Kê xe lên cao để bánh không chạm đất.

```bash
pio run -e robot -t upload
pio device monitor
```

Sau đó test theo thứ tự:

```text
bump
enc
gas
mot 80 0
mot 0 80
mot 80 80
mot -80 80
ping
```

Chỉ khi từng khối đã hoạt động đúng mới đặt xe xuống sàn để hiệu chuẩn quãng đường,
góc quay và độ lệch khi đi thẳng.

> `selftest` của firmware phải phản ánh đúng cấu hình **1 encoder / không MPU6050 /
> 1 bumper**. Nếu vẫn dùng logic selftest cũ dành cho 2 encoder + MPU6050 thì kết quả
> `HONG` ở các phần không lắp không có nghĩa wiring bị sai; firmware cần được cập nhật.
