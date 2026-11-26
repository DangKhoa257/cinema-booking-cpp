# Giao diện Qt Widgets — Giai đoạn 3C

## Cấu trúc

`qt/main_window.*` chứa khung ứng dụng, thanh điều hướng, vùng thông báo và chuyển trang. `qt/catalog_pages.cpp` chứa trang chủ/phim và suất chiếu. `qt/transaction_pages.cpp` chứa sơ đồ ghế, khách hàng, vé và thanh toán mô phỏng. Mỗi trang là một lớp `QWidget` riêng trong `qt/ui_pages.h`; `qt/ui_helpers.h` gom các thành phần trình bày nhỏ.

`qt/ui_session.*` giữ dữ liệu phiên GUI và gọi các manager trong `cinema_core`. Tất cả quy tắc đặt vé, trạng thái ghế, hủy vé và thanh toán vẫn ở `BookingManager`, `Ticket` và `Payment`. `CinemaDataStore` trong core được dùng chung với console để lưu/tải và kiểm tra tham chiếu. GUI không viết lại thuật toán nghiệp vụ. `MainWindow` điều hướng, hiển thị thông báo và yêu cầu các trang cập nhật; `StorageBar` tách các điều khiển chọn thư mục/lưu/tải.

```text
MainWindow -> HomePage / ShowtimePage / SeatMapPage / CustomerPage / TicketPage / PaymentPage
           -> UiSession -> MovieCatalog / ScheduleManager / UserManager / BookingManager / CinemaDataStore
                      -> cinema_core
```

## Màn hình và phạm vi

- Trang chủ có khối giới thiệu và thẻ phim; chọn phim lọc đúng suất, chọn suất mở đúng phòng và sơ đồ ghế.
- Sơ đồ ghế phân biệt Available, Held, Booked và ghế đang chọn; xác nhận phim, giờ, ghế, khách, giá trước khi đặt.
- Trang khách cho phép đăng ký khách demo. Trang vé hiển thị trạng thái và hủy vé hợp lệ.
- Trang thanh toán xử lý lại vé Pending qua `DemoPayment` hoặc `BankTransferPayment`. Chuyển khoản demo luôn giữ Pending, không xác nhận giao dịch ngân hàng.

Khi mở, GUI dùng thư mục mặc định ổn định từ `QStandardPaths::AppDataLocation`, ngoài thư mục build. Nếu chưa có file dữ liệu thì dùng demo; nếu có một phần hoặc file lỗi thì báo lỗi, giữ demo và không tự ghi đè. Có thể chọn thư mục khác. Sau thay đổi, thanh lưu trữ báo **Chưa lưu** và khi thoát sẽ hỏi nếu còn thay đổi. Nút **Lưu** ghi cả năm file; **Tải** chỉ thay dữ liệu khi toàn bộ hợp lệ. Vẫn còn giới hạn của Giai đoạn 2: việc ghi năm file chưa là một giao dịch nguyên tử. Chưa có xác thực người dùng, ảnh poster, dữ liệu phim từ mạng hoặc cổng thanh toán thật.

Giai đoạn 3C bổ sung xác nhận trước khi đặt, thanh toán demo và hủy vé. Khi còn thay đổi chưa lưu, thoát, tải bản khác hoặc chuyển thư mục đều hỏi **Lưu – Không lưu – Hủy**. Nút vô hiệu hóa và focus có kiểu hiển thị rõ; thẻ phim xuống dòng khi tên dài; ghế có tooltip trạng thái. Thanh lưu hiển thị tên thư mục ngắn gọn, toàn bộ đường dẫn ở tooltip. Trang có cuộn dọc ở cửa sổ 860×620; ảnh chụp thật ở `docs/screenshots/`.

`CinemaDataStore` ghi bộ năm file vào `.cinema-next`, tải thử và kiểm tra tham chiếu, sao lưu bộ cũ ở `.cinema-backup`, rồi mới thay file chính. Nếu thay thế lỗi, nó cố khôi phục bộ cũ; bản sao lưu được giữ để khôi phục thủ công nếu mất điện hoặc quá trình bị dừng giữa chừng. Năm file chính vẫn giữ định dạng và API cũ. Không tự ghi đè bộ file thiếu; GUI chỉ ghi đè bản lưu lỗi khi người dùng đồng ý. Đây là phương án đơn giản cho bài tập, **chưa phải giao dịch nguyên tử** hoặc hệ thống backup nhiều phiên bản.

## OOP Cinema 2.0

Target `oop_cinema_qt` dùng `V2Session` (điều phối nghiệp vụ), `MovieGallery`/`SeatMapWidget`/`TicketCard` (widget) và `V2Window` (điều hướng). Giao diện V1 và console giữ nguyên chức năng. Luồng, tài nguyên, lưu trữ và giới hạn chi tiết ở [OOP_CINEMA_V2_GUI.md](OOP_CINEMA_V2_GUI.md). `CinemaV2` quản lý rạp/phòng/suất; `BookingManager` vẫn quyết định trạng thái ghế và vé; `FileStorage` lưu khách/vé. GUI không tự tạo xác nhận chuyển khoản.

## Build và kiểm thử

`BUILD_QT_GUI=OFF` mặc định. Khi bật, CMake tìm Qt 6 Widgets và tạo `cinema_booking_qt`, `test_qt_ui`, `test_qt_persistence`; nếu Qt thiếu, chỉ build console và test cũ. `test_qt_ui` dùng kiểm tra luôn chạy trong Release, kiểm tra phim → suất → ghế → Pending → Paid → Cancelled và đường dẫn lưu có dấu. `qt_persistence_flow` chạy nhiều tiến trình GUI để kiểm tra lưu/khôi phục, phòng khác, bản lưu lỗi và file thiếu. Test CTest dùng `QT_QPA_PLATFORM=offscreen`; kiểm tra giao diện thực tế trên Windows đã chạy với `QT_QPA_PLATFORM=windows`, gồm cửa sổ tối thiểu có cuộn dọc. GitHub Actions build/test Qt ở cả cấu hình mặc định và Release.
