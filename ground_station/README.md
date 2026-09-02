# GasSeeker Ground Station

Dashboard local dùng trong vận hành và demo GasSeeker. Không cần Internet và không cần cài package Python/Node.

## Có gì trên dashboard

- Kết nối ESP32-S3 trạm thu qua **Web Serial** ở 115200 baud.
- Đọc đúng protocol hiện tại của trạm: `RX,<rssi>,<snr>,$GS,...*HH`.
- Kiểm tra XOR checksum của gói `$GS` trước khi hiển thị.
- Bản đồ sa bàn **200 × 200 cm**, lưới 25 cm.
- Quỹ đạo robot realtime và heatmap tương đối theo `norm` (không coi ppm là dữ liệu điều khiển).
- Telemetry: algorithm, FSM state, ADC, normalized signal, ppm estimate, alarm level, x/y/heading, travelled distance, best signal, RSSI và SNR.
- Link health: số gói hợp lệ, gói lỗi, tốc độ nhận và thời điểm gói gần nhất.
- Mission control qua uplink đang có sẵn: `start`, `stop`, `algo 0|1|2`, `cal`, `info`.
- Xuất phiên hiện tại ra CSV với schema tương thích `tools/receiver.py`.
- **Demo Mode** tạo telemetry synthetic để trình diễn UI khi chưa có robot/trạm thu. Dữ liệu này luôn phải được giới thiệu là mô phỏng, không phải kết quả thực nghiệm.

## Chạy trước khi thuyết trình

Từ root repository:

```bash
python3 ground_station/server.py
```

Trình duyệt sẽ mở `http://127.0.0.1:8765/`. Nên dùng **Google Chrome** hoặc **Microsoft Edge** vì Web Serial không được Safari/Firefox hỗ trợ đầy đủ.

### Demo không cần phần cứng

1. Bấm **Demo Mode**.
2. Quan sát robot chạy trên sa bàn, quỹ đạo, heatmap, Δ tín hiệu, RSSI/SNR và state thay đổi realtime.
3. Bấm **Xuất CSV** nếu muốn cho ban giám khảo thấy dữ liệu phiên chạy được lưu lại.
4. Nói rõ đây là *simulation/demo telemetry* để minh họa Ground Station, không phải dữ liệu thực địa.

### Demo với phần cứng thật

1. Nạp firmware xe:

```bash
pio run -e robot -t upload
```

2. Nạp firmware trạm thu vào ESP32-S3 thứ hai:

```bash
pio run -e base -t upload
```

3. Gắn antenna vào cả hai module LoRa trước khi cấp điện.
4. Cắm ESP32 trạm thu vào laptop bằng USB.
5. Chạy `python3 ground_station/server.py`.
6. Bấm **Kết nối USB**, chọn đúng cổng ESP32.
7. Đợi telemetry hiện lên. Sau đó có thể chọn thuật toán và dùng START/STOP từ dashboard.

> STOP trên dashboard là lệnh uplink qua LoRa; nút BOOT/E-stop vật lý trên xe vẫn là phương án dừng cục bộ quan trọng khi demo phần cứng.

## Protocol

Robot phát:

```text
$GS,t,algo,state,adc,norm,ppm,level,x,y,head,dist,cx,cy,best,fin*HH
```

Trạm thu chuyển sang laptop:

```text
RX,<rssi>,<snr>,$GS,t,algo,state,adc,norm,ppm,level,x,y,head,dist,cx,cy,best,fin*HH
```

Dashboard gửi chuỗi lệnh thường qua USB, ví dụ `start\n`; firmware `src/base/main.cpp` tự đóng gói thành `CMD,start` rồi phát LoRa lên xe.

## Chuẩn bị demo offline

- Mở dashboard và chạy Demo Mode ít nhất một lần trước khi đến địa điểm.
- Giữ Chrome/Edge đã cài sẵn, không phụ thuộc Wi-Fi.
- Khi dùng phần cứng thật, kiểm tra LoRa bằng bench/ping trước.
- Đặt laptop sao cho ban giám khảo nhìn thấy cả sa bàn vật lý và dashboard.
- Không trình bày heatmap/ppm synthetic trong Demo Mode như kết quả thực nghiệm.
