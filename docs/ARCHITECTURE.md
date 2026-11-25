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

TV2, TV3, TV5, TV6 đã triển khai các module trên `main`; mã TV4 trong branch TV1 được cherry-pick từ PR #3 đang mở. `CinemaSystem` tạo dữ liệu demo, hiển thị menu và gọi các module qua public API. Chỉ phim và vé có API `FileStorage`; phòng, lịch và khách demo tồn tại trong bộ nhớ của phiên chạy. Trước khi thay danh sách phim hoặc vé sau khi tải, TV1 kiểm tra tham chiếu chéo qua các module.

`Seat` và `CinemaRoom` chỉ mô tả vị trí vật lý. Trạng thái đặt ghế được suy ra từ `Ticket` theo cặp `(showtimeId, seatId)` trong `BookingManager`: Pending là Held, Paid là Booked, Cancelled không giữ ghế. TV1 kiểm tra ghế thuộc phòng của suất và từ chối cặp đã Held/Booked trước khi tạo vé. Cùng vị trí ghế có thể được dùng độc lập ở các suất khác nhau. Khi đặt, TV1 tạo Pending trước khi gọi `Payment`; chỉ kết quả demo thành công mới chuyển sang Paid. `payTicket` cho phép xử lý lại vé Pending. Hủy chuyển vé sang Cancelled, nhờ vậy ghế lại Available.
