# OOP Cinema 2.0 — giao diện đặt vé demo

## Chạy chương trình

Bật `BUILD_QT_GUI=ON` và build target `oop_cinema_qt`. Ứng dụng Qt V1 (`cinema_booking_qt`) và console (`cinema_booking`) vẫn chạy độc lập. CMake sao chép `data/v2/movie_details.txt`, `data/v2/cinema_demo.txt` và `qt/assets/` cạnh executable. Có thể chạy `oop_cinema_qt --data-dir <thư-mục>` để chọn bản lưu riêng; mặc định dùng `QStandardPaths::AppDataLocation`. Tùy chọn `--window-size 860x620` phục vụ kiểm tra bố cục.

Luồng demo: **chọn phim → chọn cụm rạp và suất → chọn tối đa 8 ghế Standard/VIP → chọn bắp nước → nhập thông tin người mua → áp một voucher → Pending hoặc thanh toán mô phỏng → vé điện tử**. Cụm rạp, phòng, suất và giá do OOP Cinema mô phỏng; không lấy lịch hoặc giá từ rạp có thật. Ghế VIP cộng 20.000 ₫ trên giá vé cơ bản. Bắp nước có 6 sản phẩm; voucher `OOP10`, `SINHVIEN`, `CUOITUAN` giảm trên tổng vé và bắp nước, tối đa theo mức từng mã và không âm. Không cần tài khoản. `DemoPayment` chuyển vé Pending sang Paid; chọn giữ Pending không xác nhận chuyển khoản ngân hàng. Đơn Paid hoặc Pending đều có thể hủy theo chính sách demo.

`qt/v2_session.*` nạp danh mục phim và `CinemaV2`, gọi `BookingManager`, `DemoPayment`, `BankTransferPayment` và `FileStorage`. `qt/v2_widgets.*` chứa lưới poster, sơ đồ ghế và vé; `qt/v2_window.*` chỉ điều hướng và trình bày. Mã đơn dạng `OOP-000001` dựa trên ID vé đầu tiên. QR offline mã hóa `OOP-CINEMA|mã-đơn|movieId|showtimeId` bằng thư viện MIT của [Project Nayuki](https://github.com/nayuki/QR-Code-generator). QR chỉ nhận diện đơn trong bản demo, không phải phương thức thanh toán hoặc xác thực vào rạp. Vé đã hủy không hiển thị QR còn hiệu lực.

## Lưu và khôi phục

Thư mục V2 dùng `movie_details.txt`, `cinema_v2.txt`, `customers.txt`, `tickets.txt`, `orders.json`. Các file V1 của console nằm ở nơi khác. Tải dữ liệu vào đối tượng tạm, kiểm tra tham chiếu và trạng thái ghế trước khi thay dữ liệu đang chạy. Nếu file lỗi hoặc thiếu, GUI dùng dữ liệu demo và báo rõ, giữ nguyên các file cũ. Lệnh Lưu với bản cũ hỏng đòi xác nhận; các file cũ được chép vào `damaged-backup-*` trước khi thay. Khi thoát có thay đổi chưa lưu, GUI hỏi **Lưu – Không lưu – Hủy**. Bản lưu nhiều file vẫn chưa là một giao dịch nguyên tử; đây là giới hạn có chủ ý trong phạm vi bài tập.

## Tài nguyên và phạm vi

Chín phim có poster do TV1 cung cấp được hiển thị và cho đặt mới. Hai phim cũ không có poster (104, 108) chỉ giữ trong metadata và lịch sử đặt vé. Bảng ánh xạ ID, tên, ảnh và nguồn/ngày xác minh nằm ở `qt/assets/posters/mapping.json` và [MOVIE_DATA_V2.md](MOVIE_DATA_V2.md). Banner trang chủ/trang chi tiết phóng mờ **poster của chính phim đang chọn** làm nền; ảnh dọc vẫn giữ tỷ lệ 2:3. Qt không tải ảnh từ mạng. Quyền phân phối công khai poster cần TV1 xác nhận.

Lịch demo sinh tương đối khi nạp `cinema_demo.txt`, nhưng bản lưu chuyển thành thời gian tuyệt đối để giữ tham chiếu vé. Sau khi tất cả suất trong bản lưu đã qua, cần chọn thư mục dữ liệu mới cho một phiên demo mới; không tự dịch lịch của vé cũ. Không có đăng nhập, hoàn tiền thật, email gửi thật, cổng thanh toán, ghế đôi hoặc giao dịch nguyên tử nhiều file.

## Kiểm thử

`test_v2_session` kiểm tra 9 phim đang hiển thị, 2 ID lịch sử, 4 cụm, 12 phòng 80–120 ghế, giá VIP, voucher, đặt trùng, Pending → Paid → Cancelled, lưu/mở lại với đường dẫn Unicode và bản lưu lỗi. `test_v2_qt_navigation` kiểm tra cả 9 ánh xạ poster, tỷ lệ 2:3, banner chuyển đúng metadata và phim 101 → suất 1 → ghế A1 ở cửa sổ 860×620 và resize 1120×760. CTest Qt chạy offscreen; ảnh thật từ Qt Windows và bản đóng gói Release ở [trang chủ có poster](screenshots/oop-v2-posters-home-1120x760.jpg), [chi tiết phim](screenshots/oop-v2-posters-detail-1120x760.jpg). Các ảnh trước bản cập nhật poster vẫn nằm trong `screenshots/` để đối chiếu lịch sử.
