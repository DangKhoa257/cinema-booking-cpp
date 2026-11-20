# Hợp đồng public API — phiên bản Giai đoạn 0

Tài liệu này chốt chữ ký khai báo trong `include/`. Mọi thành viên triển khai đúng header của mình. Đề xuất đổi chữ ký, kiểu dữ liệu hoặc ý nghĩa trả về phải được TV1 và các module liên quan thống nhất trước khi sửa. Getter/constructor là phần khung đã có; các thao tác nghiệp vụ trong `.cpp` vẫn là TODO.

## Kiểu chung, quy ước và invariant

- `include/common/types.h`: `enum class SeatStatus { Available, Held, Booked };` và `enum class TicketStatus { Pending, Paid, Cancelled };`. `SeatStatus` là trạng thái **theo suất chiếu**, được suy ra từ vé; `Seat` không lưu trạng thái.
- ID dùng `int` dương, duy nhất trong từng loại đối tượng. ID tham chiếu phải tồn tại khi thao tác nghiệp vụ được thực hiện. Không dùng ID làm chỉ số vector.
- Tiền dùng `long long` VND nguyên, không âm; không dùng `float` hoặc `double`.
- `std::string` cho tên và thời gian. Thời gian dạng ISO `YYYY-MM-DD HH:MM`, giờ 24h, không kèm múi giờ. Dữ liệu trong cùng một phiên phải được hiểu theo một múi giờ địa phương; chưa xử lý UTC, DST, ngày nhuận và parse ngày đầy đủ ở skeleton. Sau khi xác thực đúng độ dài/cú pháp và giá trị ngày giờ, so sánh từ điển của hai chuỗi cùng format cho đúng thứ tự thời gian. Khoảng chiếu là nửa mở `[start,end)`, yêu cầu `start < end`; hai suất chạm mốc cuối/đầu không xung đột.
- `seatId` dạng chữ hàng in hoa `A`–`Z` + số ghế từ 1 trở lên, ví dụ `A1`. Mỗi phòng không được trùng `seatId`. Ghế không tồn tại không được coi là trống.
- `bool`: `true` chỉ khi thao tác hoàn tất theo nghĩa tài liệu; `false` khi dữ liệu sai, không tìm thấy, trùng, xung đột, lỗi I/O hoặc chức năng còn là stub. Hàm `find*` trả `nullptr` khi không tìm thấy. `get*` trả tham chiếu `const` tới dữ liệu do đối tượng quản lý; không giữ pointer/tham chiếu đó sau khi vector bị thay đổi hoặc đối tượng bị hủy.
- Constructor hiện chỉ gán dữ liệu, chưa kiểm tra invariant. Caller cần cấp dữ liệu hợp lệ; owner bổ sung validation trong giai đoạn triển khai và thống nhất cách báo lỗi trước khi đổi API. Không dùng raw owning pointer.
- `operator<<` cho `Movie` và `Ticket` là giao diện xuất ra `std::ostream&` để in console hoặc test bằng `std::ostringstream`, không phải định dạng lưu file. Owner triển khai cách trình bày; hiện in nhãn TODO.

## TV2 — phim

`include/movie.h` — `Movie(int id, std::string title, int durationMinutes, std::string genre, long long basePriceVnd)`; `int getId() const`; `const std::string& getTitle() const`; `int getDurationMinutes() const`; `const std::string& getGenre() const`; `long long getBasePriceVnd() const`; hàm tự do `std::ostream& operator<<(std::ostream&, const Movie&)`. Phim hợp lệ có title/genre không rỗng, durationMinutes > 0, basePriceVnd >= 0.

`include/movie_catalog.h` — `bool addMovie(const Movie&)`; `const Movie* findMovie(int id) const`; `const std::vector<Movie>& getMovies() const`; `bool removeMovie(int id)`. `addMovie` từ chối ID trùng; `removeMovie` trả false nếu không có ID. Kiểm tra phim đang được lịch tham chiếu thuộc tầng tích hợp TV1, không tạo dependency từ catalog sang lịch.

## TV3 — phòng và ghế

`include/seat.h` — `Seat(char row, int number)`; `char getRow() const`; `int getNumber() const`; `std::string getSeatId() const`. Đây là ghế vật lý, không có trạng thái đặt vé. Hàng `A`–`Z`, số >= 1.

`include/cinema_room.h` — `CinemaRoom(int id, std::string name, std::vector<Seat> seats)`; `int getId() const`; `const std::string& getName() const`; `const std::vector<Seat>& getSeats() const`; `bool hasSeat(const std::string& seatId) const`. `hasSeat` chỉ xác nhận vị trí ghế vật lý tồn tại; phòng có id dương, tên không rỗng, seatId duy nhất. TV3 không cần đọc vé hoặc trạng thái suất chiếu.

## TV4 — lịch chiếu

`include/showtime.h` — `Showtime(int id, int movieId, int roomId, std::string startTime, std::string endTime, long long priceVnd)`; `int getId() const`; `int getMovieId() const`; `int getRoomId() const`; `const std::string& getStartTime() const`; `const std::string& getEndTime() const`; `long long getPriceVnd() const`.

`include/schedule_manager.h` — `bool addShowtime(const Showtime&)`; `const Showtime* findShowtime(int id) const`; `bool hasRoomConflict(int roomId, const std::string& start, const std::string& end) const`; `const std::vector<Showtime>& getShowtimes() const`. `addShowtime` từ chối ID trùng hoặc xung đột cùng phòng; movieId/roomId trỏ đến dữ liệu do tầng tích hợp TV1 quản lý. Xung đột khi `existing.start < end && start < existing.end` với cùng roomId, sau khi xác thực thời gian.

## TV5 — người dùng

`include/user.h` — abstract `User(int id, std::string name)`; `virtual ~User()`; `int getId() const`; `const std::string& getName() const`; pure virtual `std::string role() const`; pure virtual `void displayInfo(std::ostream&) const`. Hủy qua `User*` an toàn. ID dương, tên không rỗng. Không có mật khẩu, token hay thông tin cá nhân thật.

`include/customer.h` và `include/admin.h` — `Customer(int id, std::string name)` / `Admin(int id, std::string name)`; override `role()` và `displayInfo(std::ostream&)`. `role()` lần lượt là `Customer` và `Admin`. Đây chỉ là role demo, chưa phải xác thực bảo mật.

`include/user_manager.h` — `bool addCustomer(const Customer&)`; `const Customer* findCustomer(int id) const`; `const std::vector<Customer>& getCustomers() const`. ID khách duy nhất; Admin chưa được lưu trong manager ở bản skeleton.

## TV6 — vé, thanh toán, lưu trữ

`include/ticket.h` — `Ticket(int id, int customerId, int showtimeId, std::string seatId, long long priceVnd, TicketStatus status = TicketStatus::Pending)`; `int getId() const`; `int getCustomerId() const`; `int getShowtimeId() const`; `const std::string& getSeatId() const`; `long long getPriceVnd() const`; `TicketStatus getStatus() const`; `bool setStatus(TicketStatus status)`; hàm tự do `std::ostream& operator<<(std::ostream&, const Ticket&)`. Một vé tham chiếu khách/suất/ghế hợp lệ, giá không âm. Cần kiểm tra chuyển trạng thái: `Pending -> Paid/Cancelled`, `Paid -> Cancelled` theo chính sách hủy của TV1; không hồi sinh vé đã `Cancelled`.

`include/payment.h` — abstract `virtual ~Payment()`; pure virtual `bool process(long long amountVnd, const std::string& bookingCode)`. `include/demo_payment.h` — `bool process(long long amountVnd, const std::string& bookingCode) override`. `include/bank_transfer_payment.h` — cùng chữ ký override. `true` nghĩa là phương thức thanh toán đã hoàn tất theo quy tắc của chính nó; số tiền dương và mã đặt vé không rỗng. `DemoPayment` về sau trả true cho mô phỏng thành công hợp lệ và phải ghi rõ là DEMO. Skeleton hiện trả false. `BankTransferPayment` chỉ phát hướng dẫn/QR **demo** và trả false để giữ `Pending` cho tới quy trình xác minh thủ công được thống nhất; thao tác bấm nút không chứng minh đã nhận tiền. Không tích hợp ngân hàng thật và không lưu tài khoản ngân hàng, mật khẩu, token, API key.

`include/file_storage.h` — `bool saveMovies(const std::vector<Movie>&, const std::string& path)`; `bool loadMovies(std::vector<Movie>&, const std::string& path)`; `bool saveTickets(const std::vector<Ticket>&, const std::string& path)`; `bool loadTickets(std::vector<Ticket>&, const std::string& path)`. `save*` trả true sau khi ghi trọn file; `load*` chỉ thay vector đầu ra sau khi parse/validate toàn bộ thành công, tránh dữ liệu nửa chừng. Chưa có I/O trong skeleton.

## TV1 — điều phối và giao diện console

`include/booking_manager.h` — `BookingManager(const ScheduleManager& schedule, std::vector<CinemaRoom>& rooms, const UserManager& users, std::vector<Ticket>& tickets)`; `bool bookTicket(int customerId, int showtimeId, const std::string& seatId, Payment& payment)`; `bool cancelTicket(int ticketId)`; `SeatStatus getSeatStatus(int showtimeId, const std::string& seatId) const`. Nhận ID, tra qua public API, không sở hữu các đối tượng được tham chiếu. Owner `CinemaSystem` phải sống lâu hơn `BookingManager`. `getSeatStatus` chỉ xét vé có đúng cặp `(showtimeId, seatId)`: `Pending -> Held`, `Paid -> Booked`, `Cancelled` hoặc không có vé hoạt động `-> Available`; nếu dữ liệu sai có cả Pending/Paid thì Booked ưu tiên. Hàm này chưa kiểm tra ghế vật lý có tồn tại; TV1 phải tra suất, phòng và `CinemaRoom::hasSeat` trước khi cho đặt. Khi triển khai `bookTicket`: kiểm tra khách/suất/phòng/ghế, từ chối nếu `getSeatStatus(...) != Available`, tạo Pending, chỉ chuyển Paid sau `DemoPayment` true hoặc xác minh ngân hàng thực sự. Tại mọi thời điểm, tối đa một vé chưa `Cancelled` cho cùng `(showtimeId, seatId)`; cùng `seatId` ở hai suất khác nhau là độc lập. `cancelTicket` chuyển vé sang `Cancelled`, khiến ghế lại `Available` nếu không còn vé hoạt động. Hai hàm đặt/hủy hiện là stub trả false và không sửa dữ liệu. TV1 chịu trách nhiệm thực thi invariant này khi triển khai.

`include/cinema_system.h` — `CinemaSystem()`; `void run()`. Sở hữu catalog, rooms, schedule, users, tickets, storage và booking coordinator. `run()` hiện chỉ in thông báo skeleton; TV1 sẽ thêm vòng lặp menu. `src/main.cpp` chỉ khởi tạo và gọi `run()`.

## Dữ liệu file dự kiến

File UTF-8, một record mỗi dòng, ký tự `|` phân cách, không có escape. Tên/genre không chứa `|` hoặc xuống dòng. Dòng rỗng và dòng bắt đầu `#` được bỏ qua; không có header bắt buộc. Mã trạng thái ghi đúng tên enum bằng tiếng Anh. TV6 kiểm tra số cột, số nguyên, enum và ID; quy ước ghi/đọc này chưa được triển khai. Không dùng `operator<<` làm serializer.

| File | Cột theo thứ tự |
| --- | --- |
| `movies.txt` | `id|title|durationMinutes|genre|basePriceVnd` |
| `rooms.txt` | `roomId|name|seatId,seatId,...` |
| `showtimes.txt` | `id|movieId|roomId|startTime|endTime|priceVnd` |
| `customers.txt` | `id|name` |
| `tickets.txt` | `id|customerId|showtimeId|seatId|priceVnd|TicketStatus` |

API `FileStorage` tại Giai đoạn 0 chỉ chốt movies/tickets; room/showtime/customer file là mẫu để các owner thống nhất trước khi bổ sung API sau review. Các file mẫu không chứa dữ liệu cá nhân thật.
