# Slides báo cáo — dàn ý

Dàn ý từng slide dùng làm nội dung và ghi chú người nói.

## Dàn ý 16 slide

> Quy ước: **[TIÊU ĐỀ]** · nội dung hiện trên slide · *Ghi chú người nói (không hiện)*

---

### Slide 1 — Bìa
**GasSeeker — Robot tự hành dò tìm nguồn rò rỉ khí bằng cảm biến đơn**
Đề tài cuối khoá PIFKID · [tên nhóm] · [ngày]
*Hình nền: ảnh xe thật (chèn sau khi lắp xong).*

---

### Slide 2 — Vấn đề
**Trong hầm, cống, bể chứa — rò rỉ khí độc là nguyên nhân tử vong hàng đầu**
- Khí không màu, không mùi cảnh báo → mất ý thức nhanh
- Người ứng cứu **phải trực tiếp đi vào vùng nguy hiểm** để tìm điểm rò
- Tìm càng lâu → phát tán càng nhiều → rủi ro càng cao

> Robot không chịu ảnh hưởng sinh học của khí độc.

*Nói: đây là bài toán có người chết thật, không phải bài toán học thuật.*

---

### Slide 3 — Câu hỏi nghiên cứu
**Với chỉ MỘT cảm biến khí, chiến lược chuyển động nào tìm nguồn nhanh và ổn định nhất?**

| | Giả thuyết |
|---|---|
| **H1** | Môi trường khuếch tán trơn → **bám gradient** nhanh nhất |
| **H2** | Môi trường đứt quãng (có gió rối) → **surge-casting** nhanh nhất |
| **H3** | **Quét toàn bộ** luôn thành công nhưng chậm nhất — mốc so sánh |

*Nói: đề tài không phải "làm một con robot", mà là "trả lời một câu hỏi bằng thực nghiệm".*

---

### Slide 4 — Sản phẩm
**Ảnh xe thật, chú thích từng khối** + bảng thông số:

| | |
|---|---|
| Vi điều khiển | ESP32-S3 |
| Cảm biến khí | MQ-3 — **chỉ một** |
| Định vị | encoder + MPU6050 (dead reckoning) |
| Truyền dữ liệu | LoRa SX1262, 923 MHz |
| Chấp hành | 4 motor, vi sai hai bên, 2 × TB6612 |
| Khu vực khảo sát | 2 × 2 m, lưới ô 25 cm |

*Nói thêm: robot có hai vai trò — tự tìm nguồn, và làm trạm đo di động dựng bản đồ nhiệt.*

---

### Slide 5 — Sơ đồ khối
Sơ đồ khối tổng quát (lấy từ `BaoCaoGiuaKy.md` mục 4).
Nhấn mạnh mũi tên **Lớp 1** đi vào thuật toán, **Lớp 2** đi ra LoRa.

---

### Slide 6 — Sơ đồ nguồn
Sơ đồ nguồn (mục 5) + ba nguyên tắc an toàn.
*Nói: hai buck riêng — nếu chung, motor khởi động làm sụt áp → sợi đốt MQ-3 hạ nhiệt → số đọc
khí nhiễu theo chính chuyển động của robot. Đó là tương quan giả rất khó phát hiện.*

---

### Slide 7 — Hai lớp dữ liệu khí
**Thuật toán KHÔNG BAO GIỜ dùng ppm**

| Lớp | Giá trị | Dùng cho | Yêu cầu |
|---|---|---|---|
| **1** | ADC thô đã lọc | thuật toán, điều kiện dừng | chỉ cần **đơn điệu** đúng |
| **2** | ppm ước lượng, mức cảnh báo | hiển thị cho người | **bậc độ lớn** |

`ppm = A × (Rs/R0)^B` với **B = −1,865** → sai số tương đối bị **khuếch đại 1,87 lần**

*Nói: đưa ppm vào điều kiện dừng = đưa vào đại lượng đã khuếch đại nhiễu gần gấp đôi, để quyết
định một việc chỉ cần biết tăng hay giảm.*

---

### Slide 8 — Vấn đề trung tâm: ĐỘ TRỄ
**Cảm biến MQ-3: đáp ứng ≤ 10 s, hồi phục ≤ 30 s**

```
Đo trong lúc xe chạy:  Δs = v × τ = 18 cm/s × 2,5 s ≈ 45 cm
                       ↑ gần HAI ô lưới
```

Ba thiết kế bắt buộc kéo theo:
1. **Dừng ngửi** — mọi phép đo đều dừng hẳn xe
2. **Gradient đi thẳng khi còn tăng**, chỉ quét khi giảm
3. **Quay lại điểm cao nhất** — robot luôn vượt qua đỉnh rồi mới biết

*Đây là slide quan trọng nhất về mặt kỹ thuật. Nói chậm.*

---

### Slide 9 — Thuật toán 1: Quét toàn bộ
Ảnh `docs/img/quydao_quettoanbo.png` + `T ≈ (A/d²) × (2,9 + d/18) s`
→ 64 ô × 4,3 s ≈ **277 s**

*Nói: nó cố tình KHÔNG dừng sớm. Cho dừng sớm thì không còn là baseline.*

---

### Slide 10 — Thuật toán 2: Bám gradient
Ảnh `docs/img/quydao_gradient.png` + sơ đồ vòng lặp

**Vì sao quay tại chỗ lại đo được thông tin không gian?**
Đầu dò gắn lệch trước **12 cm** → quay ±55° làm đầu dò dịch **≈ 20 cm**

*Chỉ vào ảnh: thấy robot vượt qua nguồn rồi quay lại điểm cao nhất — đó là hệ quả trực tiếp của
độ trễ ở slide 8.*

---

### Slide 11 — Vì sao gradient hỏng trong gió rối
Sơ đồ sợi khí:
```
        nguồn ★
   gió ←──╫──── ▓ ▒   ▓  ▒ ▓ ▒   ← các sợi khí RỜI RẠC
             ↑
   cảm biến đọc: 0, 0, 850, 0, 0, 1200, 0, 300, 0 ...
```
**Nồng độ không còn giảm đơn điệu theo khoảng cách → bám gradient = bám nhiễu**
Số liệu: gradient chỉ thành công **1/10** trong môi trường đứt quãng.

---

### Slide 12 — Thuật toán 3: Surge-casting
**Ý tưởng: bướm đêm tìm bạn tình qua pheromone ở hàng trăm mét, chỉ với hai râu**

> Trong gió rối, nồng độ tại một điểm là ngẫu nhiên.
> Nhưng có MỘT điều luôn đúng: **ngửi thấy khí → nguồn ở phía đầu gió.**

- **SURGE** — thấy khí → lao thẳng **ngược gió**
- **CAST** — mất khí → nghĩa là đã ra khỏi **bề ngang** của luồng → quét **vuông góc** với gió,
  biên độ tăng dần: 15 → 24 → 38 → 61 cm

**"Mất tín hiệu" chuyển từ thất bại thành hành vi tìm kiếm có định hướng.**

Máy trạng thái + ảnh `docs/img/quydao_surgecast.png`

---

### Slide 13 — Kiến trúc phần mềm
Sơ đồ `src/core/ → IRobot → {RobotIO, SimRobot}`

**Cùng một file `.cpp` thuật toán chạy trên cả ESP32 lẫn máy tính**
- 60 lần chạy mô phỏng: **40 giây** — cùng số lần trên xe thật: **hơn 4 giờ**
- **86 phép kiểm tra tự động**, chạy trong 1 giây
- Bộ test đã bắt được **2 lỗi vòng lặp vô hạn** mà chạy tay không thấy

---

### Slide 14 — Kết quả mô phỏng
⚠️ **SỐ LIỆU MÔ PHỎNG — chưa phải đo trên robot thật**

| Thuật toán | Môi trường | Thời gian | Thành công |
|---|---|---|---|
| Quét toàn bộ | trơn | 277 ± 6 s | **10/10** |
| **Gradient** | **trơn** | **100 ± 29 s** | 7/10 |
| Gradient | đứt quãng | 90 ± 44 s | **1/10** ← hỏng |
| **Surge-casting** | **đứt quãng** | **132 ± 63 s** | 5/10 |

Ảnh `docs/img/kq_thoigian.png`

**Xu hướng khớp cả ba giả thuyết** — gradient nhanh gấp 2,8 lần ở môi trường trơn nhưng hỏng
hẳn khi đứt quãng; surge-casting cứu được đúng trường hợp đó.

---

### Slide 15 — Tiến độ
✅ Ba thuật toán · firmware xe + trạm thu · mô phỏng · 86/86 test · tài liệu
🔄 Lắp ráp phần cứng
❌ Hiệu chuẩn MQ-3 · số liệu thực nghiệm

**Rủi ro lớn nhất:** ba ngưỡng điều khiển hiện là giá trị phỏng đoán từ mô hình. Phải đo lại trên
cảm biến thật — quy trình đã chuẩn bị sẵn (6 khoảng cách, mỗi vị trí một lệnh).

---

### Slide 16 — Hạn chế và hướng phát triển

**Hạn chế (nói thẳng)**
- Chỉ dò được khí **nặng hơn hoặc xấp xỉ không khí**
- **Không định danh** được loại khí
- `ppm` chỉ là **ước lượng bậc độ lớn**
- Hướng gió **khai báo trước**, robot không tự đo
- Dead-reckoning trôi **0,5–0,8 % quãng đường** → trần sân ≈ 3 × 3 m

**Hướng phát triển**
- Thuật toán **lai thích nghi** — tự chuyển giữa gradient và surge-casting theo độ đứt quãng
- Cảm biến ở **nhiều độ cao** cho khí nhẹ
- **Infotaxis / lọc Bayes**
- **Nhiều robot** phối hợp

---

### Slide dự phòng (để sẵn cuối deck, dùng khi bị hỏi)

- **B1** — Bảng chân GPIO đầy đủ
- **B2** — Đường đặc tuyến MQ-3 (khoảng cách ↔ norm ↔ ppm) đo được ở giai đoạn hiệu chuẩn
- **B3** — Điều kiện dừng ba thành phần, giải thích từng vế
- **B4** — Bảng trần kích thước sân (1,5 m → 5 m)
- **B5** — Bản đồ nhiệt `docs/img/bandonhiet.png`
- **B6** — Hai lỗi vòng lặp vô hạn và cách sửa

---

## Ba việc cần làm trước khi trình chiếu

1. **Chèn ảnh xe thật vào slide 1 và 4.** Hội đồng cần thấy phần cứng có tồn tại.
2. **Tập nói slide 8, 11, 12.** Ba slide này là phần kỹ thuật cốt lõi; các câu hỏi xoáy sẽ tập
   trung vào đây. Xem [`PHAN_BIEN.md`](PHAN_BIEN.md) mục "Ba câu trả lời cần thuộc lòng".
3. **In `PHAN_BIEN.md` ra giấy** để cạnh khi bảo vệ.
