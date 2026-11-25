# Kiến trúc tích hợp TV1

Ứng dụng console C++17, không phụ thuộc thư viện ngoài. `CinemaSystem` sở hữu các kho dữ liệu theo giá trị; `BookingManager` giữ tham chiếu đến lịch, phòng, khách và vé do `CinemaSystem` sở hữu. Vì vậy các thành viên dữ liệu phải được khai báo trước `booking_` và sống suốt vòng đời của nó.

```text
main -> CinemaSystem -> BookingManager -> ScheduleManager, CinemaRoom,
                              UserManager, Ticket, Payment (polymorphic)
                   -> MovieCatalog, FileStorage

MovieCatalog -> Movie
CinemaRoom -> Seat
ScheduleManager -> Showtime
UserManager -> Customer -> User (abstract)
Admin -> User (abstract)
DemoPayment, BankTransferPayment -> Payment (abstract)
```

Các quan hệ tham chiếu chéo dùng ID; module phim không include lịch, module lịch không include phòng, module vé không include khách. `BookingManager` là nơi kết hợp chúng. `Payment&` cho phép đa hình lúc chạy; `User` và `Payment` có pure virtual method và destructor virtual. Vector và string của STL quản lý bộ nhớ, không có raw owning pointer.

TV2–TV6 đã triển khai các module trên `main`, bao gồm TV4 qua PR #3; TV1 đã merge qua PR #6. `CinemaSystem` tạo dữ liệu demo, hiển thị menu và gọi các module qua public API. Giai đoạn 2 bổ sung FileStorage cho phòng, lịch và khách. Menu lưu toàn bộ ghi năm file trong cùng thư mục. Menu tải toàn bộ đọc năm vector tạm, dựng manager tạm và kiểm tra tham chiếu chéo trước khi gán vào các thành viên. Vì `BookingManager` giữ tham chiếu đến chính các thành viên đó, nó tiếp tục hoạt động sau khi tải. File sai không thay bất kỳ dữ liệu đang dùng nào. Việc ghi từng file dùng file tạm, nhưng cả thư mục chưa có giao dịch nguyên tử; bản lưu bị gián đoạn có thể phải khôi phục từ bản sao lưu.

Giai đoạn 3A thêm ứng dụng Qt Widgets tùy chọn (`cinema_booking_qt`) trên cùng thư viện `cinema_core`; chương trình console vẫn là target độc lập. Các trang Qt chỉ hiển thị và thu thập đầu vào, `UiSession` chuyển yêu cầu nghiệp vụ đến các manager hiện có. Chi tiết cấu trúc GUI và phạm vi hiện tại ở [QT_UI.md](QT_UI.md).

Giai đoạn 3B gom việc lưu/tải năm file và kiểm tra tham chiếu vào `CinemaDataStore` thuộc TV1 trong `cinema_core`. `CinemaSystem` và `UiSession` dùng cùng lớp đó. GUI không có bản sao của quy tắc tải hoặc đặt vé; khi tải lỗi, cả hai giữ nguyên dữ liệu đang dùng. Qt dùng `QStandardPaths::AppDataLocation` cho thư mục mặc định và hiển thị trạng thái thay đổi chưa lưu.

Giai đoạn 3C thêm vùng ghi tạm và bản sao lưu trong `CinemaDataStore`, cùng kiểm tra lại bộ file vừa tuần tự hóa trước khi thay thế. Nếu bước thay thế thất bại, lớp cố khôi phục từ `.cinema-backup`; bản sao này cũng phục vụ khôi phục khi chương trình bị dừng giữa chừng. Quy tắc kiểm tra tham chiếu vẫn chỉ nằm ở core. GUI thêm xác nhận các thao tác quan trọng và hộp thoại ba lựa chọn khi bỏ dữ liệu chưa lưu. Bản Windows Qt/MinGW được đóng gói bằng `windeployqt`, không chứa dữ liệu người dùng.

`Seat` và `CinemaRoom` chỉ mô tả vị trí vật lý. Trạng thái đặt ghế được suy ra từ `Ticket` theo cặp `(showtimeId, seatId)` trong `BookingManager`: Pending là Held, Paid là Booked, Cancelled không giữ ghế. TV1 kiểm tra ghế thuộc phòng của suất và từ chối cặp đã Held/Booked trước khi tạo vé. Cùng vị trí ghế có thể được dùng độc lập ở các suất khác nhau. Khi đặt, TV1 tạo Pending trước khi gọi `Payment`; chỉ kết quả demo thành công mới chuyển sang Paid. `payTicket` cho phép xử lý lại vé Pending. Hủy chuyển vé sang Cancelled, nhờ vậy ghế lại Available.

Giai đoạn 4B.1 bổ sung `MovieDetails` và `MovieDetailsCatalog` trong `cinema_core`, không chỉnh `Movie`/`MovieCatalog` TV2 hoặc luồng console/Qt 3C. Catalog V2 đọc một file metadata UTF-8 riêng, gồm nguồn và ngày xác minh; trạng thái là ảnh chụp biên tập, chưa gắn với suất OOP Cinema. `toV1Movie(price)` là cầu nối cần giá nội bộ rõ ràng trước khi phim được đặt vé. 4C sẽ nối catalog này vào GUI; xem [MOVIE_DATA_V2.md](MOVIE_DATA_V2.md).
