# Chuẩn bị phản biện — GasSeeker

22 câu hỏi xoáy mà reviewer nhiều khả năng sẽ hỏi, kèm câu trả lời. Nguyên tắc chung khi trả
lời: **thừa nhận đúng cái mình chưa làm được, rồi giải thích vì sao lựa chọn hiện tại vẫn hợp
lý trong phạm vi đề tài.** Vòng vo hoặc phóng đại là cách nhanh nhất để mất điểm.

---

## A. Về ý tưởng và phạm vi

### 1. "Chỉ một cảm biến thì làm sao biết hướng? Sao không gắn 2–4 cảm biến rồi so sánh?"

Gắn nhiều cảm biến **đúng là dễ hơn** — nhưng nó biến bài toán thành bài toán khác, và ba lý do
khiến một cảm biến là lựa chọn có chủ đích:

1. **Đó chính là bài toán khoa học của đề tài.** Câu hỏi nghiên cứu là *"với chỉ một cảm biến,
   chiến lược chuyển động nào tốt nhất"*. Nhiều cảm biến sẽ chuyển thông tin từ **chính sách
   chuyển động** sang **phần cứng** — không còn gì để so sánh ba thuật toán nữa.
2. **Nhiều MQ-3 cạnh nhau không cho gradient đáng tin.** Khoảng cách giữa hai cảm biến trên xe
   21 cm chỉ được ~15 cm, trong khi sai lệch giữa hai module MQ-3 khác nhau (do `R0` khác nhau,
   độ già khác nhau) thường lớn hơn chênh lệch nồng độ trên 15 cm đó. Phải hiệu chuẩn chéo rất
   kỹ mới dùng được.
3. **Robot vẫn tạo được thông tin không gian bằng cách chuyển động.** Đầu dò gắn lệch trước
   12 cm, nên khi quay ±55° đầu dò dịch ~20 cm — tương đương đặt hai cảm biến cách nhau 20 cm,
   mà **dùng chung một cảm biến nên không có sai lệch giữa các module**.

Đây cũng chính là cách sinh vật làm: bướm đêm chỉ có hai râu rất gần nhau, nó bù bằng hành vi.

### 2. "Sân 2×2 m, mặt phẳng, một nguồn. Cái này có ý nghĩa thực tế gì?"

Xin phân biệt hai thứ: **kiểm chứng nguyên lý** và **sản phẩm thương mại**. Đề tài thuộc loại
thứ nhất.

Cái được kiểm chứng ở quy mô 2×2 m là **chính sách chuyển động** — và chính sách đó
**không phụ thuộc tỉ lệ**: surge-casting hoạt động ở quy mô mét cũng như ở quy mô hàng trăm mét
(đó là quy mô mà bướm đêm dùng nó). Cái **có** phụ thuộc tỉ lệ là hệ định vị, và chúng em đã đo
được trần đó: dead-reckoning trôi 0,5–0,8 % quãng đường, giới hạn sân ở khoảng 3×3 m. Muốn lớn
hơn thì phải đổi hệ định vị (UWB, SLAM), chứ **không** phải đổi thuật toán.

Về "một nguồn": đây là giới hạn có chủ đích và đã ghi trong phần hạn chế. Xem câu 15.

### 3. "Ethanol không phải khí độc. Kết quả có chuyển sang khí độc thật được không?"

**Không chuyển trực tiếp được, và chúng em không tuyên bố như vậy.**

Cái được bảo toàn là **hình học và động học của trường nồng độ**, vì nó bị chi phối bởi hệ số
khuếch tán *D* và khối lượng phân tử *M*. Ethanol (46 g/mol; D ≈ 0,12 cm²/s) nằm gọn giữa dải
của nhóm khí mục tiêu: H₂S (34; 0,17) · CO₂ (44; 0,16) · propan (44; 0,11) · butan (58; 0,09).
Nghĩa là plume ethanol có **dạng và tốc độ lan tương tự**, nên thuật toán bám theo nó sẽ hành
xử tương tự.

Cái **không** bảo toàn: phản ứng hoá học của cảm biến, ngưỡng độc tính, và hành vi của khí nhẹ
hơn không khí. Với khí độc thật cần cảm biến chuyên dụng và hiệu chuẩn lại toàn bộ ngưỡng theo
TLV-TWA / IDLH.

### 4. "Robot chỉ bò dưới đất. Khí nhẹ như CH₄, NH₃, H₂ bay lên cao thì sao?"

**Robot sẽ không phát hiện được, và đây là hạn chế đã nêu rõ ở mục 8.1 của đề cương.**

Đề tài giới hạn ở **nhóm khí nặng hơn hoặc xấp xỉ không khí** — cũng chính là nhóm gây tử vong
phổ biến nhất trong hầm, cống, bể chứa (H₂S, CO₂, propan, butan tích tụ ở đáy). Hướng phát triển
đã đề xuất: bổ sung cảm biến ở nhiều độ cao hoặc cần nâng.

---

## B. Về thuật toán

### 5. "Giai đoạn SEEK là quét zig-zag. Vậy gradient và surge-casting có khác gì quét toàn bộ?"

Câu hỏi rất đúng chỗ. Khác ở **ba điểm định lượng**:

1. **SEEK quét thưa gấp 4 lần** (cách 2 ô thay vì 1 ô → 16 điểm thay vì 64 điểm).
2. **SEEK dừng ngay khi bắt được tín hiệu đầu tiên**, thường sau 2–5 điểm. Quét toàn bộ đi hết
   64 điểm bất kể có tìm thấy hay không.
3. Nhìn số liệu: gradient dùng **4,0 m** quãng đường, quét toàn bộ dùng **16,9 m** — gấp 4,2 lần.
   Nếu gradient chỉ là quét toàn bộ trá hình thì hai con số này phải bằng nhau.

Và chúng em **chủ động dùng chung** giai đoạn SEEK cho cả gradient lẫn surge-casting, để bảng so
sánh **cô lập được đúng biến cần đo**: hành vi *sau khi* bắt được luồng khí — đó mới là điều H1
và H2 phát biểu.

### 6. "Vì sao gradient không quét ba hướng ở mỗi bước như lý thuyết?"

Vì hằng số thời gian hồi phục của cảm biến (~8 s) **lớn hơn** khoảng cách giữa ba phép đo (~4 s).

Cụ thể: sau 4 s, số đọc mới hồi phục `1 − e^(−4/8) = 39 %` quãng đường về giá trị đúng, tức
phép đo thứ ba vẫn mang **61 % ảnh hưởng** của phép đo thứ hai. Ba số liệu sẽ lệch theo **thứ tự
đo**, không theo không gian — robot luôn thiên về một bên bất kể nguồn ở đâu.

Nên chúng em đổi sang: **đi thẳng khi còn tăng** (hai phép đo cách nhau 30 cm — chênh lệch không
gian át được độ trễ), **chỉ quét ba hướng khi nồng độ giảm**, tức khi thực sự cần thông tin
hướng. Đây đúng là biến thể đơn giản mà đề cương cũng nêu, và lý do chọn nó là **vật lý của cảm
biến**, không phải để cho dễ.

### 7. "Robot 'quay lại điểm cao nhất' — có phải là gian lận để giảm sai số không?"

Không, và có **số liệu** chứng minh nó là cần thiết chứ không phải trang trí.

Nguyên nhân vật lý: τ_xuống = 8 s, mà mỗi lần dừng ngửi chỉ 1,5 s quá độ. Robot **luôn** nhận ra
"đã qua đỉnh" sau khi đã đi quá 1–2 bước. Đây là hệ quả không tránh được của cảm biến MOX.

Nếu **không** quay lại, sai số của gradient là ~52 cm; có quay lại, còn ~22 cm. Và quan trọng
hơn: **thời gian định vị được tính cả quãng đường quay về** — chúng em không giấu chi phí đó.

Về mặt ứng dụng, đây cũng là hành vi đúng: robot phải **đứng tại** vị trí nghi là nguồn để đánh
dấu, không phải dừng ở nơi nó tình cờ đi tới.

### 8. "Vì sao surge-casting lại kém hơn gradient ở môi trường không quạt? Nó là thuật toán 'nâng cao' cơ mà."

Vì "nâng cao" không có nghĩa là "tốt hơn ở mọi nơi" — **mỗi thuật toán chỉ tối ưu khi giả định
của nó thoả mãn.**

Giả định của surge-casting là **có gió và biết hướng gió**. Trong môi trường không quạt, giả
định đó sai hoàn toàn: không có gió, nhưng thuật toán vẫn tiến theo một hướng "ngược gió" khai
báo cứng, và **cố tình bỏ qua** thông tin gradient vốn rất tốt trong trường trơn.

Kết quả 2/10 ở môi trường trơn so với 5/10 ở môi trường đứt quãng **chính là bằng chứng cho
H1 và H2**, chứ không phải dấu hiệu thuật toán sai. Nếu surge-casting thắng ở cả hai môi trường
thì mới đáng nghi — nghĩa là thí nghiệm không phân biệt được gì.

### 9. "Hướng gió được khai báo cứng trong code. Vậy có phải bạn đã cho robot biết trước đáp án?"

**Không — biết hướng gió khác hoàn toàn với biết vị trí nguồn.**

Hướng gió chỉ cho biết nguồn nằm **đâu đó ở phía đầu gió**, tức thu hẹp từ 360° xuống một nửa
mặt phẳng. Nó **không** cho biết nguồn ở xa bao nhiêu, lệch trái hay lệch phải bao nhiêu — mà đó
mới là phần khó, và chính là phần cơ chế *cast* phải giải.

Bằng chứng: nếu biết hướng gió là đủ, robot chỉ cần đi thẳng ngược gió là xong. Thực tế nó phải
cast trung bình 3–4 lần với biên độ tăng dần, và vẫn chỉ thành công 5/10.

Đây là đơn giản hoá có chủ đích, đã ghi rõ ở mục 11.2 của đề cương. Trong sản phẩm thật sẽ gắn
cảm biến hướng gió (dây/cảm biến siêu âm) — không khó, chỉ là nằm ngoài phạm vi 2 tuần.

### 10. "Nếu robot chưa từng phát hiện khí thì sao? Nó chạy mãi à?"

Không. Có **ba** cơ chế chấm dứt, xếp theo thứ tự kích hoạt:

1. **Giai đoạn SEEK hữu hạn**: quét hết 16 điểm thô rồi kết luận bằng điểm cao nhất đã đo.
2. **`StallGuard`**: sau 12 phép đo liên tiếp không tiến triển đáng kể, robot dừng và kết luận.
3. **Timeout 8 phút**: chốt chặn cuối, dừng vô điều kiện.

Bộ test có riêng một nhóm (nhóm 7) chạy 24 tình huống và **bắt buộc mọi lần đều phải kết thúc**.
Chính nhóm test này đã phát hiện hai lỗi vòng lặp vô hạn trong quá trình phát triển.

### 11. "Nếu nguồn yếu, `norm` không bao giờ vượt `STOP_HIGH_DELTA` thì robot làm gì?"

Nó **không** thoả điều kiện dừng, nên sẽ tiếp tục tìm cho tới khi `StallGuard` kích hoạt (12 phép
đo không tiến triển), rồi kết luận bằng **điểm đo cao nhất đã ghi được**.

Kết quả sẽ là: robot vẫn chỉ đúng hướng nguồn nhưng dừng xa hơn, và bị tính là "trượt" nếu quá
30 cm. Đây là hành vi **thất bại có kiểm soát** — robot vẫn đưa ra kết luận tốt nhất có thể thay
vì treo hoặc chạy hết pin.

### 12. "Sao không dùng phương pháp tốt hơn như infotaxis hay lọc Bayes?"

Infotaxis đúng là tốt hơn về lý thuyết — nó chọn hành động **tối đa hoá lượng thông tin thu
được**, chứ không bám gradient. Ba lý do chưa dùng:

1. **Nó cần một mô hình phát tán** để cập nhật phân bố hậu nghiệm về vị trí nguồn. Mô hình đó
   phải khớp môi trường thật, mà chúng em **chưa có số liệu thực nghiệm nào** để hiệu chỉnh —
   dùng bây giờ là xây trên cát.
2. Phạm vi đề tài là **so sánh ba chiến lược cổ điển** để hiểu chúng đánh đổi ra sao. Đó là nền
   để sau này đánh giá infotaxis có đáng thêm phức tạp không.
3. Thời gian dưới 2 tuần.

Nó đã nằm trong phần hướng phát triển, cùng với thuật toán lai tự chuyển giữa gradient và
surge-casting dựa trên độ đứt quãng của tín hiệu đo được.

---

## C. Về số liệu và phương pháp

### 13. "Số liệu là mô phỏng, mà mô hình mô phỏng do chính bạn viết. Nó chứng minh được gì?"

Đây là câu hỏi đúng nhất trong tất cả, và câu trả lời trung thực là: **nó không chứng minh
được ba giả thuyết. Nó chỉ chứng minh phần mềm chạy đúng và cho thấy xu hướng nhất quán.**

Chúng em đã làm ba việc để giảm rủi ro thiên vị:

1. **Mô hình không được "chỉnh" theo thuật toán nào.** Tham số plume và tham số cảm biến được
   chốt **trước**, dựa trên datasheet MQ-3 (response ≤ 10 s, recovery ≤ 30 s) và mô hình puff
   tiêu chuẩn trong tài liệu về chemical plume tracing.
2. **Mô hình cố tình làm khó robot**: có nhiễu ADC, có trễ bất đối xứng, có sai số dead-reckoning
   (robot nhìn thấy vị trí *nó tin*, còn chấm điểm dùng vị trí *thật*).
3. **Cùng seed → cùng vị trí nguồn cho cả ba thuật toán**, nên không có chuyện thuật toán nào
   gặp bài dễ hơn.

Dù vậy, trong báo cáo số liệu mô phỏng nằm ở **mục riêng**, có cảnh báo rõ, và **không** được
trộn với số liệu thực nghiệm. Ba giả thuyết chỉ được kết luận sau khi có dữ liệu đo thật.

### 14. "Tỉ lệ thành công 7/10 và 5/10 là thấp. Sản phẩm này dùng được không?"

Ở dạng hiện tại thì **chưa** — và đó là kết quả trung thực chứ không phải thất bại.

Ba điểm cần nói rõ:

1. **Ngưỡng "thành công" là 30 cm — rất khắt khe** so với kích thước robot (21 cm). Nếu nới ra
   50 cm thì tỉ lệ tăng lên đáng kể. Chúng em cố tình chọn ngưỡng chặt để con số có ý nghĩa.
2. **Đa số ca trượt là trượt gần**, không phải trượt hoàn toàn: sai số trung bình của gradient ở
   môi trường trơn là 22 cm, chỉ nhỉnh hơn ngưỡng ở vài lần chạy. Robot **chỉ đúng hướng** trong
   hầu hết các lần.
3. Con số này đến từ **mô phỏng có cố ý làm khó**. Trên phần cứng thật, nếu ba ngưỡng được hiệu
   chuẩn đúng theo cảm biến thật, kỳ vọng sẽ khác — theo cả hai chiều.

### 15. "Nếu có hai nguồn khí thì sao?"

Robot sẽ **hội tụ về một trong hai** — nguồn nào nó bắt được luồng trước, hoặc nguồn mạnh hơn —
và báo đó là vị trí nguồn. Nó **không** biết là có nguồn thứ hai.

Đây là giới hạn cố hữu của cả ba thuật toán: chúng đều là **thuật toán leo đồi** trên một trường
vô hướng, nên chỉ tìm được **một** cực đại. Riêng quét toàn bộ có dữ liệu để phát hiện hai đỉnh
(vì nó đo hết mọi ô), nhưng phần kết luận hiện chỉ lấy điểm cao nhất.

Cách mở rộng: dùng **bản đồ nhiệt** mà robot đã dựng sẵn (sản phẩm phụ của Lớp 2) rồi tìm các
cực đại địa phương. Đây là bổ sung ở phần phân tích hậu kỳ, không cần đổi thuật toán.

### 16. "`ppm` của bạn sai bao nhiêu phần trăm?"

**Chúng em không công bố sai số phần trăm, vì không có thiết bị chuẩn để so sánh.** Cái công bố
được là **bậc độ lớn** và ba nguồn sai số:

1. **Cảm biến MQ không chọn lọc** — phản ứng với nhiều hơi hữu cơ khác, không riêng ethanol.
2. **Độ nhạy phụ thuộc nhiệt độ và độ ẩm** — datasheet có đồ thị hiệu chỉnh, chúng em không bù.
3. **`R0` trôi theo thời gian** — giảm bớt bằng cách tự hiệu chuẩn lại mỗi lần bật máy.

Thêm nữa, hằng số `A`, `B` hiện fit từ **hai điểm đọc thô trên đồ thị datasheet**, bản thân việc
đọc đồ thị đã có sai số.

Đó chính là lý do thiết kế **tách hai lớp dữ liệu**: `ppm` chỉ để người đọc tham khảo, còn thuật
toán chỉ dùng đếm ADC thô — đại lượng chỉ cần **đơn điệu đúng**, không cần chính xác tuyệt đối.

### 17. "Vì sao bán kính thành công là 30 cm? Có phải chọn cho vừa kết quả không?"

30 cm được chốt **trước khi** có bất kỳ số liệu nào, và có ba căn cứ:

1. Robot dài ~21 cm — kết luận sai lệch nhỏ hơn kích thước robot thì không còn ý nghĩa phân biệt.
2. Một ô lưới là 25 cm — 30 cm tương đương "đúng ô hoặc lệch tối đa một ô".
3. Nguồn ethanol thật (đĩa/lọ) có đường kính vài cm, nhưng vùng nồng độ cao quanh nó rộng cỡ
   vài chục cm.

Nếu hội đồng muốn, chúng em có sẵn dữ liệu thô để tính lại tỉ lệ thành công ở bất kỳ ngưỡng nào
— `err_cm` của từng lần chạy đều được lưu trong `summary.csv`.

### 18. "Mỗi ô dừng ngửi 2,3 giây thì quá lãng phí. Sao không đo liên tục rồi lọc?"

Vì vấn đề **không phải nhiễu, mà là độ trễ** — và lọc không sửa được độ trễ, lọc chỉ làm tăng nó.

Tính cụ thể: cảm biến có τ_lên = 2,5 s. Đo trong lúc xe chạy 18 cm/s thì giá trị đọc được ứng
với vị trí cách vị trí thật:

```
Δs ≈ v × τ = 18 cm/s × 2,5 s ≈ 45 cm
```

45 cm là gần hai ô lưới. Không có bộ lọc nào lấy lại được thông tin đó — nó đã bị chính vật lý
của cảm biến làm nhoè. Cách duy nhất là **giảm v về 0** khi đo.

Đây là đánh đổi có ý thức: mất 2,3 s mỗi phép đo để đổi lấy phép đo **gắn đúng với một toạ độ**.

### 19. "Sau 5 phút chạy, robot còn biết mình ở đâu không?"

Biết, nhưng **có sai số tích luỹ** — và chúng em đã đo được nó: **0,5–0,8 % quãng đường**
(10 cm sau 17 m; 50 cm sau 64 m).

Đây là hạn chế **cố hữu của dead-reckoning**, đã nêu ở mục 8.4 đề cương. Nó cũng chính là **trần
thật** giới hạn kích thước sân ở khoảng 3×3 m, chứ không phải thời gian hay pin.

Ba biện pháp đã dùng để giảm: lấy góc từ gyro thay vì từ encoder (tránh sai số trượt bánh), đo
bias gyro lúc khởi động, và hiệu chuẩn đường kính bánh bằng lệnh `drive 100` đo thước.

Biện pháp chưa dùng (nằm trong hướng phát triển): mốc chuẩn cố định (UWB, thị giác) để reset sai
số định kỳ.

---

## D. Về kỹ thuật triển khai

### 20. "Vì sao không dùng camera hoặc AI?"

Ba lý do, theo thứ tự quan trọng:

1. **Camera không nhìn thấy khí.** Ethanol và các khí độc mục tiêu đều **không màu**. Camera chỉ
   giúp định vị/tránh vật cản, không giúp gì cho bài toán chính.
2. **Bài toán này không thiếu dữ liệu để cần học máy** — nó thiếu **thông tin**. Robot có đúng
   một số đo vô hướng tại một điểm. Mạng nơ-ron không tạo thêm được thông tin không có trong dữ
   liệu; cái quyết định là **chính sách chuyển động để đi thu thập thông tin**, và đó chính xác
   là thứ đề tài nghiên cứu.
3. Đề cương xếp AI vào mục "Không làm" của phiên bản đầu.

### 21. "Bạn viết cả mô phỏng — có phải làm màu không? Sao không thử thẳng trên xe?"

Mô phỏng là **công cụ tiết kiệm thời gian**, và đây là con số cụ thể:

- 60 lần chạy mô phỏng: **40 giây**.
- 60 lần chạy trên xe thật: hơn **4 giờ**, tốn cồn, tốn pin, và cần người trông.

Với deadline 2 tuần và linh kiện chưa về, mô phỏng cho phép hoàn thiện xong thuật toán **trước
khi** có phần cứng.

Quan trọng hơn: nhờ kiến trúc tách lớp, **cùng một file `.cpp` thuật toán chạy trên cả hai** —
không có bản mô phỏng riêng để lệch với firmware. Và mô phỏng đã phát hiện **ba vấn đề thiết kế
thật** (vượt đỉnh do trễ cảm biến, quét ba hướng bị lệch theo thứ tự đo, kẹt vòng lặp vô hạn ở
góc sân) mà nếu chỉ thử trên xe sẽ mất nhiều ngày mới nhận ra.

### 22. "Bạn có tự tin phần mềm chạy đúng không? Kiểm chứng thế nào?"

Có **86 phép kiểm tra tự động** chạy trong 1 giây, không cần phần cứng, kiểm tra từ chuẩn hoá
góc, điều kiện dừng ba thành phần, quy đổi ppm, cho tới định dạng gói tin.

Nhóm quan trọng nhất chạy **3 thuật toán × 2 môi trường × 4 vị trí nguồn** và bắt buộc mọi lần
đều phải **kết thúc**, không được chạy hết giờ.

Và bộ test đã **thực sự bắt được lỗi**, không phải chỉ để trang trí — hai lỗi vòng lặp vô hạn mà
10 lần chạy mô phỏng thủ công trước đó không phát hiện, vì chúng chỉ xuất hiện với vị trí nguồn
sát góc sân.

Điều em **không** dám khẳng định: firmware chạy đúng trên **phần cứng thật**. Nó mới chỉ build
sạch. Đó là việc của giai đoạn tiếp theo, và chúng em đã chuẩn bị sẵn lệnh `selftest` kiểm tra
lần lượt từng khối phần cứng để rút ngắn giai đoạn đó.

---

## Ba câu trả lời cần thuộc lòng

Nếu chỉ nhớ được ba điều, hãy nhớ ba điều này:

**1. Vì sao phải dừng lại mới đo được?**
> Cảm biến MQ-3 có trễ ~2,5 giây. Xe chạy 18 cm/s thì số đọc ứng với vị trí cách đó 45 cm — gần
> hai ô lưới. Không bộ lọc nào lấy lại được thông tin đã bị làm nhoè; cách duy nhất là dừng hẳn.

**2. Surge-casting hoạt động thế nào?**
> Trong gió rối, nồng độ tại một điểm là ngẫu nhiên nên gradient vô nghĩa. Nhưng có một điều
> luôn đúng: **ngửi thấy khí thì nguồn ở phía đầu gió**. Nên: thấy khí → lao thẳng ngược gió;
> mất khí → nghĩa là đã ra khỏi bề ngang của luồng, nên quét **ngang** với biên độ tăng dần cho
> tới khi bắt lại. Mất tín hiệu chuyển từ thất bại thành hành vi tìm kiếm có định hướng.

**3. Điểm mạnh nhất của đề tài là gì?**
> Không phải phần cứng — phần cứng là linh kiện phổ thông. Điểm mạnh là **thiết kế thuật toán
> xuất phát từ giới hạn vật lý thật của cảm biến**, và **kiến trúc cho phép cùng một mã thuật
> toán chạy trên cả robot lẫn máy tính**, nhờ đó kiểm chứng được bằng 86 phép kiểm tra tự động
> trước khi có phần cứng.
