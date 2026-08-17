# Báo cáo GasSeeker — dự án LaTeX

Nhóm 14 · CLB Pay it Forward · PIFKID 2026

---

## 1. Biên dịch

```bash
latexmk -xelatex main.tex     # khuyến nghị — tự chạy đủ số lần cho mục lục
# hoặc
xelatex main && xelatex main
# hoặc
pdflatex main && pdflatex main
```

Trên **Overleaf**: mở project, chọn Menu → Compiler → **XeLaTeX** (hoặc pdfLaTeX, cả hai đều chạy),
Main document → `main.tex`.

Phải chạy **ít nhất hai lần** thì Mục lục, Danh mục hình vẽ và Danh mục bảng mới đúng số trang.

## 2. Cấu trúc thư mục

```
main.tex          preamble + khung báo cáo, gọi từng file body/
Khung.tex         trang bìa
body/             nội dung, mỗi chương một file
pic/              toàn bộ hình vẽ (PNG, độ phân giải in ~370 dpi)
```

Thứ tự các file trong `body/` khớp với thứ tự `\input` trong `main.tex`.
Muốn sửa nội dung chương nào thì mở đúng file chương đó; không cần đụng `main.tex`.

## 3. Nguồn gốc format và những gì đã thay đổi

Format kế thừa từ một mẫu báo cáo LaTeX của sinh viên Trường Đại học Bách khoa – ĐHQG TP.HCM
(bài tập lớn môn Phương pháp tính). **Toàn bộ thông tin cá nhân và nội dung của nhóm gốc đã được gỡ bỏ.**

Cụ thể, đã **giữ lại** (đây là phần format):

- Khung viền trang bìa vẽ bằng TikZ, kèm bốn ke góc.
- Bố cục bìa: tên đơn vị → tên báo cáo → hai đường kẻ ôm lấy tên đề tài in màu xanh → bảng thông tin → địa danh và năm.
- Header chạy trang bằng `fancyhdr`, mục lục dot-leader bằng `tocloft`.
- Hộp nhấn mạnh bằng `tcolorbox` với vạch màu bên trái (bản gốc là môi trường `bt`).
- Danh mục tài liệu tham khảo dùng `thebibliography` đánh số `[1] [2] …`.
- Cỡ chữ 12 pt, khổ A4, `\parindent` 2 em.

Đã **gỡ bỏ**:

- Logo và tên Trường Đại học Bách khoa (`pic/bklogo.png`, `pic/bk_name_vi.png`) — nhóm không thuộc trường này.
- Bảng bảy thành viên và MSSV của nhóm gốc, tên giảng viên hướng dẫn, tên lớp.
- Header "Bài tập lớn – Phương Pháp Tính".
- Toàn bộ nội dung Newton-Raphson (`body/sec1.tex` … `body/sec5.tex`) và các hình `pic/` đi kèm.
- Các gói không dùng tới: `siunitx`, `empheq`, `pgfplots`, phần cấu hình tô màu mã MATLAB.

Đã **thay đổi có chủ ý**:

| Hạng mục | Bản gốc | Bản này | Lý do |
|---|---|---|---|
| Engine | chỉ `pdflatex` (`[T5]{fontenc}` + `babel[vietnamese]`) | chạy được **cả** `pdflatex` lẫn `xelatex`/`lualatex` | nhánh `xelatex` dùng `fontspec` + Times New Roman (dự phòng Liberation Serif), nhờ đó biên dịch được cả trên máy không cài gói tiếng Việt của TeX Live |
| Lề | trái 2 cm | trái 2,5 cm | chừa chỗ đóng gáy khi in |
| Tiêu đề chương | `\section` đánh số trơn | `CHƯƠNG n.` căn giữa, mỗi chương một trang mới | quy ước báo cáo |
| Đánh số hình/bảng | liên tục toàn bài | theo chương (`Hình 3.1`, `Bảng 6.2`) | dễ tra khi báo cáo dài |
| Nếu có logo CLB | — | đặt file vào `pic/logo.png`, bìa tự hiển thị | không có file thì bìa vẫn cân đối |

Nếu biên dịch bằng **pdfLaTeX**, công thức toán sẽ dùng font Times (gói `mathptmx`) khớp với chữ thân bài.
Nếu biên dịch bằng **XeLaTeX**, công thức dùng font toán mặc định của LaTeX, hơi khác chữ thân bài — điều này bình thường và không phải lỗi.

## 4. Nguồn số liệu trong Chương 6

Toàn bộ số liệu là **kết quả mô phỏng**, sinh ra bằng chính trình mô phỏng trong repo GasSeeker:

```bash
pio run -e sim
./.pio/build/sim/program --trials 30 --seed 1 --traj
```

Cấu hình: 3 thuật toán × 2 môi trường × 30 lần chạy = **180 lần chạy**, hạt giống bắt đầu bằng 1.
Ba thuật toán dùng chung hạt giống nên gặp đúng cùng một vị trí nguồn.

Kiểm định thống kê dùng `scipy.stats`: kiểm định chính xác Fisher cho tỉ lệ thành công,
kiểm định Mann–Whitney U cho thời gian, khoảng tin cậy Wilson cho tỉ lệ.

**Nếu chạy lại mô phỏng với số lần thử hoặc hạt giống khác thì các con số sẽ đổi** —
khi đó phải cập nhật lại Bảng 6.1, Bảng 6.2, Bảng 6.3 và bốn hình trong Chương 6 cho khớp.

## 5. Việc còn phải làm trước khi nộp

- [ ] Bổ sung logo CLB vào `pic/logo.png` nếu có.
- [ ] Rà lại danh sách thành viên và vai trò ở Phụ lục B cho đúng thực tế phân công.
- [ ] Khi có số liệu đo trên xe thật: cập nhật các hằng số đánh dấu **[ĐO]** ở Phụ lục A,
      rồi bổ sung một mục thực nghiệm vào Chương 6.
