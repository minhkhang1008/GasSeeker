# Sơ đồ chân — ESP32-S3 DevKitC-1

Nguồn chân duy nhất đúng là `namespace cfg::pin` trong `src/core/config.h`.
File này giải thích **vì sao** chọn như vậy. Sửa chân thì sửa ở `config.h`, rồi
cập nhật lại bảng dưới.

## Chân KHÔNG được dùng trên ESP32-S3

| Chân | Lý do |
|---|---|
| GPIO 0 | Nút BOOT (strapping) — ta dùng làm **nút bấm**, chấp nhận được |
| GPIO 3, 45, 46 | Strapping, mức lúc khởi động ảnh hưởng cách boot |
| GPIO 19, 20 | USB D− / D+ |
| GPIO 26–32 | SPI flash trong |
| GPIO 33–37 | **Octal PSRAM** trên bản N16R8 — đây là bẫy hay gặp nhất |
| GPIO 43, 44 | UART0 TX/RX (cổng debug) |

---

## Bảng đấu nối

### Cảm biến khí MQ-3
| ESP32-S3 | Nối tới |
|---|---|
| GPIO 4 (ADC1_CH3) | điểm giữa mạch chia áp |

```
MQ3 AO ──[ 10 kΩ ]──┬── GPIO4
                    │
                [ 20 kΩ ]
                    │
                   GND        thêm tụ gốm 100 nF từ GPIO4 xuống GND
```
MQ-3 VCC → **5 V** (sợi đốt cần 5 V), GND chung.
Hệ số chia 20/(10+20) = 0,667 → AO 5 V ra 3,33 V ở GPIO. Khai báo trong `config.h`
(`DIV_R_TOP_OHM`, `DIV_R_BOT_OHM`) — đổi điện trở thì phải sửa hai hằng này.

### MPU6050 (GY-521), I²C
| ESP32-S3 | MPU6050 |
|---|---|
| GPIO 8 | SDA |
| GPIO 9 | SCL |
| 3V3 | VCC |
| GND | GND |

### LoRa SX1262 / Ra-01SH, SPI
| ESP32-S3 | Module |
|---|---|
| GPIO 11 | MOSI |
| GPIO 13 | MISO |
| GPIO 12 | SCK |
| GPIO 10 | NSS / CS |
| GPIO 14 | RST |
| GPIO 21 | BUSY |
| GPIO 47 | DIO1 |
| 3V3 | VCC — **cấp 3,3 V, không phải 5 V** |

Đặt tụ hoá 220–470 µF + tụ gốm 100 nF **sát chân nguồn module**. Đỉnh dòng lúc
phát tới ~120 mA; thiếu tụ là ESP32 reset.
Nhớ gắn ăng-ten **trước khi** cấp điện.

### TB6612FNG
| ESP32-S3 | TB6612 | |
|---|---|---|
| GPIO 5 | PWMA | bánh trái |
| GPIO 6 | AIN1 | |
| GPIO 7 | AIN2 | |
| GPIO 15 | PWMB | bánh phải |
| GPIO 16 | BIN1 | |
| GPIO 17 | BIN2 | |
| GPIO 18 | STBY | LOW = tắt motor |
| 3V3 | VCC | phía logic |
| — | VM | 6 V từ buck, **không** lấy từ ESP32 |

Hàn tụ gốm 100 nF thẳng qua hai cực **mỗi motor** — chống nhiễu chổi than làm sai
xung encoder.

### Encoder quang
| ESP32-S3 | |
|---|---|
| GPIO 1 | OUT bánh trái |
| GPIO 2 | OUT bánh phải |

VCC theo module (3,3 V hoặc 5 V — nếu 5 V thì **phải** chia áp về 3,3 V trước khi
vào GPIO). Firmware bật pull-up trong và đếm cạnh lên.

### Công tắc va chạm
| ESP32-S3 | |
|---|---|
| GPIO 38 | micro switch trái |
| GPIO 39 | micro switch phải |

Nối kiểu **thường mở xuống GND**: một chân vào GPIO, chân kia xuống GND. Firmware
dùng `INPUT_PULLUP`, chạm = LOW.

### Giao diện
| ESP32-S3 | |
|---|---|
| GPIO 0 | nút BOOT có sẵn — ngắn = Start/Stop, giữ = đổi thuật toán |
| GPIO 48 | LED RGB có sẵn trên board — màu báo mức cảnh báo |
| GPIO 42 | còi chủ động (qua transistor nếu còi ăn > 20 mA) |
| GPIO 40, 41 | LED rời tuỳ chọn (mặc định tắt trong `config.h`) |

---

## Nguồn

```
Pin 2S 7,4 V ── cầu chì 3 A ── công tắc chính
                                    │
                                    ├── Buck 6,0 V ≥3 A ── TB6612 VM ── 2 motor
                                    │
                                    └── Buck 5,0 V ≥3 A ── ESP32 5V/VIN
                                                          └── MQ-3 VCC
ESP32 3V3 ── TB6612 VCC ── MPU6050 ── LoRa ── encoder (nếu module 3,3 V)
```

Tất cả khối **dùng chung GND**. Tụ: 1000 µF gần VM của TB6612, 470 µF gần TB6612,
220–470 µF gần ESP32 và gần LoRa, 100 nF rải khắp.

---

## Kiểm tra trước khi cấp điện lần đầu

- [ ] Đo thông mạch GND giữa tất cả các khối
- [ ] Đo điện áp ra của **hai** buck **trước khi** cắm vào board (6,0 V và 5,0 V)
- [ ] Ăng-ten LoRa đã gắn
- [ ] `MQ3_RL_OHM` trong `config.h` khớp điện trở tải thật đo được trên module MQ-3
- [ ] Không có chân nào trong dải GPIO 26–37
- [ ] Chạy `pio run -e robot -t upload` rồi gõ `info` trên Serial để xem
      MPU6050 / LoRa / encoder có nhận không
