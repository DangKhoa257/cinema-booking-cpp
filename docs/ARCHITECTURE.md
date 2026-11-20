# Kiến trúc Giai đoạn 0

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

Skeleton chỉ có constructor/getter, role demo, thông báo console và stub trả thất bại. Không có nghiệp vụ đặt vé, thanh toán hoặc file I/O. Dữ liệu mẫu ở `data/` chỉ mô tả định dạng dự kiến.

`Seat` và `CinemaRoom` chỉ mô tả vị trí vật lý. Trạng thái đặt ghế được suy ra từ `Ticket` theo cặp `(showtimeId, seatId)` trong `BookingManager`: Pending là Held, Paid là Booked, Cancelled không giữ ghế. TV1 phải kiểm tra ghế thuộc phòng của suất và từ chối cặp đã Held/Booked trước khi tạo vé. Cùng vị trí ghế có thể được dùng độc lập ở các suất khác nhau. `getSeatStatus` đã có phần tra trạng thái; đặt/hủy vé vẫn là stub cho TV1.
