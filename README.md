# GasSeeker

Robot tự hành dò tìm vị trí nguồn rò rỉ khí bằng một cảm biến khí.
Đề tài cuối khoá PIFKID. Khí mô phỏng: hơi ethanol.

Ba chiến lược dò tìm được cài đặt và so sánh trong cùng điều kiện:

| # | Chiến lược | Vai trò |
|---|---|---|
| 1 | Quét toàn bộ (boustrophedon) | baseline — chắc chắn tìm ra, chậm nhất |
| 2 | Bám gradient (chemotaxis) | nhanh khi trường nồng độ trơn |
| 3 | Surge-casting | chịu được tín hiệu đứt quãng |

## Bắt đầu nhanh (chưa cần phần cứng)

```bash
pio run -e test && ./.pio/build/test/program        # 86 phep kiem tra, 1 giay
pio run -e sim  && ./.pio/build/sim/program --trials 10 --traj
python3 -m pip install -r tools/requirements.txt
python3 tools/analyze.py --sim
```

Biểu đồ và bảng số liệu nằm ở `data/figures/`.

## Nạp lên phần cứng

```bash
pio run -e robot -t upload && pio device monitor   # ESP32-S3 trên xe
pio run -e base  -t upload                         # ESP32-S3 trạm thu LoRa
```

## Cấu trúc

```
src/core/   thuật toán + xử lý tín hiệu — THUẦN C++, không phụ thuộc Arduino
src/robot/  firmware trên xe        src/base/  firmware trạm thu
src/lora/   driver SX1262           src/sim/   mô phỏng chạy trên máy tính
tools/      receiver.py · analyze.py · mq3_fit.py
```

Điểm cốt lõi của kiến trúc: **cùng một file `.cpp` thuật toán chạy trên cả ESP32
lẫn máy tính**, nhờ tách qua giao diện `gs::IRobot`. Không có bản mô phỏng riêng
để lệch nhau.

## Tài liệu

| File | Nội dung |
|---|---|
| [docs/RUNBOOK.md](docs/RUNBOOK.md) | **Hướng dẫn 10 giai đoạn: từ đống linh kiện tới bảng số liệu. Đọc trước tiên.** |
| [docs/WIRING.md](docs/WIRING.md) | Sơ đồ chân, đấu nối, checklist trước khi cấp điện |
| [docs/DECISIONS.md](docs/DECISIONS.md) | Mọi lựa chọn tự chốt, kèm lý do và cách đổi lại |
| [CLAUDE.md](CLAUDE.md) | Ngữ cảnh dự án và nhật ký các phiên làm việc |

Toàn bộ hằng số nằm ở [`src/core/config.h`](src/core/config.h). Các giá trị đánh dấu
`[DO]` **bắt buộc** phải đo lại trên phần cứng thật trước khi lấy số liệu.
