# Giai đoạn 3A — nền tảng Qt Widgets

## Cấu trúc

`qt/main_window.*` chứa khung ứng dụng, thanh điều hướng, vùng thông báo và chuyển trang. `qt/catalog_pages.cpp` chứa trang chủ/phim và suất chiếu. `qt/transaction_pages.cpp` chứa sơ đồ ghế, khách hàng, vé và thanh toán mô phỏng. Mỗi trang là một lớp `QWidget` riêng trong `qt/ui_pages.h`; `qt/ui_helpers.h` gom các thành phần trình bày nhỏ.

`qt/ui_session.*` giữ dữ liệu demo của phiên GUI và gọi các manager trong `cinema_core`. Tất cả quy tắc đặt vé, trạng thái ghế, hủy vé và thanh toán vẫn ở `BookingManager`, `Ticket` và `Payment`. GUI không viết lại thuật toán nghiệp vụ. `MainWindow` chỉ điều hướng, hiển thị thông báo và yêu cầu các trang cập nhật sau thao tác.

```text
MainWindow -> HomePage / ShowtimePage / SeatMapPage / CustomerPage / TicketPage / PaymentPage
           -> UiSession -> MovieCatalog / ScheduleManager / UserManager / BookingManager
                      -> cinema_core
```

## Màn hình và phạm vi

- Trang chủ có khối giới thiệu và thẻ phim; danh sách suất có giờ, phòng, giá.
- Sơ đồ ghế theo suất chiếu, cho phép chọn khách, ghế và phương thức demo để đặt vé.
- Trang khách cho phép đăng ký khách demo. Trang vé hiển thị trạng thái và hủy vé hợp lệ.
- Trang thanh toán xử lý lại vé Pending qua `DemoPayment` hoặc `BankTransferPayment`. Chuyển khoản demo luôn giữ Pending, không xác nhận giao dịch ngân hàng.

GUI hiện dùng dữ liệu demo trong bộ nhớ mỗi lần khởi động. Các API `FileStorage` và thao tác lưu/tải toàn bộ vẫn dùng được trong console; màn hình Qt chưa có điều khiển lưu/tải. Chưa có xác thực người dùng, ảnh poster, dữ liệu phim từ mạng hoặc cổng thanh toán thật.

## Build và kiểm thử

`BUILD_QT_GUI=OFF` mặc định. Khi bật, CMake tìm Qt 6 Widgets và tạo `cinema_booking_qt`, `test_qt_ui`; nếu Qt thiếu, chỉ build console và test cũ. Bản Windows đã kiểm tra với Qt 6.12.0/MinGW 13.1.0. `test_qt_ui` chạy với `QT_QPA_PLATFORM=offscreen` trong CTest, kiểm tra sáu trang và luồng chọn ghế → Pending → Paid → Cancelled. Giao diện cũng được chạy trên Qt platform Windows để kiểm tra hiển thị font và bố cục. GitHub Actions có job console và job Qt 6 riêng.
