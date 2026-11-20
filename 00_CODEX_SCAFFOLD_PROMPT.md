# PROMPT CHO CODEX — CINEMA BOOKING OOP (CHỈ KHỞI TẠO SKELETON)

Bạn là kỹ sư C++ hỗ trợ nhóm 6 sinh viên năm hai làm bài tập lớn Lập trình hướng đối tượng. Hãy thao tác TRỰC TIẾP trong repo Git hiện tại sau khi tôi đã clone. Chỉ thực hiện GIAI ĐOẠN 0: tạo nền tảng tối thiểu để 6 người triển khai song song; KHÔNG viết hộ chức năng nghiệp vụ của thành viên.

## Ngữ cảnh đề bài chính thức
Ứng dụng C++ quản lý đặt vé xem phim; >=10 class, >=2 abstract class, encapsulation, operator overloading, inheritance + association/composition, runtime polymorphism, STL, file I/O. Nhóm phải có AI prompt log, nhật ký theo tuần, báo cáo đánh giá AI 3–5 trang, bảo vệ giải thích và sửa code. Nhóm được phép dùng AI nhưng phải kiểm chứng. Tính năng thanh toán thật KHÔNG bắt buộc.

## Nguyên tắc bắt buộc
- C++17, CMake, console trước; không thêm Qt, database, API bên ngoài, thư viện bên thứ ba. Có thể mở rộng Qt sau.
- Không tự ý `git push`, tạo PR hoặc merge; không xóa file người dùng. Nếu repo đã có file, đọc và bảo toàn trước khi sửa.
- Đừng thực hiện toàn bộ dự án. Chỉ có header public API và `.cpp` stub với TODO cho chủ sở hữu. Tuy nhiên PROJECT PHẢI CẤU HÌNH, BIÊN DỊCH VÀ CHẠY ĐƯỢC ngay (in menu/demo skeleton đơn giản, không giả vờ nghiệp vụ đã hoạt động).
- Chốt hợp đồng API nhất quán ngay từ đầu; viết `docs/API_CONTRACT.md` có tên class, header, method signature, quan hệ, invariant, dữ liệu file, quy ước lỗi. Không được tạo dependency vòng; ưu tiên IDs và const reference, không raw owning pointer.
- `main.cpp`, `CMakeLists.txt`, `docs/API_CONTRACT.md`, workflow CI là file tích hợp chỉ TV1 sửa. Các thành viên không tự sửa header thuộc người khác.
- Đảm bảo tối thiểu hai abstract class thực sự có pure virtual và destructor virtual: `User` và `Payment`. Lớp con `Customer`, `Admin`; `DemoPayment`, `BankTransferPayment`.
- Operator overloading có ví dụ định nghĩa giao diện `operator<<` cho Movie/Ticket (cho phép owner triển khai) và giải thích lý do.
- Không được lưu thông tin tài khoản ngân hàng thật, mật khẩu, token, API key. Chỉ `DemoPayment` là mô phỏng thành công; `BankTransferPayment` chỉ tạo thông tin thanh toán/QR demo hoặc chờ xác minh, TUYỆT ĐỐI không tự xác nhận đã nhận tiền chỉ vì người dùng bấm nút. Không triển khai gọi ngân hàng thật.
- Kiểu dữ liệu tiền dùng `long long` VND (số nguyên), không `float`.

## Quyền sở hữu và module
TV1 (leader): `main.cpp`, `CinemaSystem`, `BookingManager`, CMake/CI, kiến trúc, merge, tích hợp.
TV2: `Movie`, `MovieCatalog` (phim; CRUD, tìm kiếm), file movie.
TV3: `Seat`, `CinemaRoom` (sơ đồ ghế, kiểm tra/trạng thái ghế).
TV4: `Showtime`, `ScheduleManager` (lịch chiếu, phòng/phim, kiểm tra xung đột).
TV5: `User` (abstract), `Customer`, `Admin`, `UserManager` (dữ liệu người dùng; demo login/role không dùng mật khẩu thật).
TV6: `Ticket`, `Payment` (abstract), `DemoPayment`, `BankTransferPayment`, `FileStorage` (vé, thanh toán mô phỏng, lưu/đọc dữ liệu). TV1 phụ trách điều phối quy trình đặt vé, TV6 không làm lại BookingManager.

## Các hợp đồng nghiệp vụ phải thống nhất trong docs/API_CONTRACT.md
Dùng `int` cho ID, `long long` cho tiền; `std::string` cho tên và giờ theo ISO `YYYY-MM-DD HH:MM` (có mô tả giới hạn validation). `enum class SeatStatus { Available, Held, Booked }` và `enum class TicketStatus { Pending, Paid, Cancelled }` trong `include/common/types.h`.
`Movie`: id, title, durationMinutes, genre, basePriceVnd; getters, `operator<<`.
`MovieCatalog`: `bool addMovie(const Movie&)`, `const Movie* findMovie(int) const`, `const std::vector<Movie>& getMovies() const`, `bool removeMovie(int)`.
`Seat`: row/number/status; getter, setter theo invariant. `CinemaRoom`: id/name/seats, `bool hasSeat(const std::string&) const`, `bool isSeatAvailable(const std::string&) const`, `bool setSeatStatus(const std::string&, SeatStatus)`. Quy ước seatId như `A1`.
`Showtime`: id/movieId/roomId/startTime/endTime/priceVnd; getters. `ScheduleManager`: `bool addShowtime(const Showtime&)`, `const Showtime* findShowtime(int) const`, `bool hasRoomConflict(int roomId, const std::string& start, const std::string& end) const`, `const std::vector<Showtime>& getShowtimes() const`. Ghi rõ format thời gian và so sánh ISO.
`User`: virtual destructor, pure virtual `std::string role() const`, `void displayInfo(std::ostream&) const`, id/name. `Customer`, `Admin`: override. `UserManager`: `bool addCustomer(const Customer&)`, `const Customer* findCustomer(int) const`, `const std::vector<Customer>& getCustomers() const`.
`Ticket`: id/customerId/showtimeId/seatId/priceVnd/status; getter/setter có kiểm tra, `operator<<`.
`Payment`: virtual destructor, pure virtual `bool process(long long amountVnd, const std::string& bookingCode)`; `DemoPayment` mô phỏng thành công; `BankTransferPayment` luôn ở trạng thái pending cho tới xác minh thủ công minh bạch (không được báo đã thanh toán thật).
`FileStorage`: `bool saveMovies(const std::vector<Movie>&, const std::string&)`, `bool loadMovies(std::vector<Movie>&, const std::string&)`, và API lưu/đọc vé phù hợp. Thiết kế format đơn giản và ghi trong docs; chưa thực hiện đọc ghi ngoài file mẫu.
`BookingManager` (TV1) chỉ điều phối các module từ interface, nhận ID; đặt vé phải kiểm tra ghế trống, tránh trùng, chỉ chuyển Booked khi demo thanh toán thành công (hoặc ngân hàng được xác minh thực sự); hủy vé hoàn trả trạng thái ghế. Các tính năng thực tế để TODO cho TV1.
`CinemaSystem` quản lý vòng lặp menu, gom module bằng thành viên/const reference phù hợp, không ôm hết logic.

## Cấu trúc cần tạo
- `include/common/types.h`
- `include/movie.h`, `movie_catalog.h`, `seat.h`, `cinema_room.h`, `showtime.h`, `schedule_manager.h`, `user.h`, `customer.h`, `admin.h`, `user_manager.h`, `ticket.h`, `payment.h`, `demo_payment.h`, `bank_transfer_payment.h`, `file_storage.h`, `booking_manager.h`, `cinema_system.h`.
- Các file `.cpp` tương ứng tại `src/`, cộng `src/main.cpp`.
- `data/README.md`, `data/movies.txt`, `data/rooms.txt`, `data/showtimes.txt`, `data/customers.txt`, `data/tickets.txt` (dữ liệu demo minh họa format, không chứa dữ liệu cá nhân thật).
- `tests/` với smoke test tối thiểu; có thể dùng `assert` + CTest (không GoogleTest).
- `docs/ARCHITECTURE.md`, `docs/API_CONTRACT.md`, `docs/AI_USAGE_LOG.md`, `docs/WEEKLY_LOG.md`, `docs/REPORT_OUTLINE.md` (mẫu chưa điền thông tin giả), `docs/TEAM_WORKFLOW.md`.
- `.github/workflows/build.yml` chạy CMake build + CTest trên Ubuntu; `.gitignore`, `README.md`, `CMakeLists.txt`.

## Kiểm tra chất lượng
1. `cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure` phải chạy được trên môi trường có compiler.
2. Không có duplicate definition, undefined reference, header include vòng.
3. Stub hợp lệ: return false/nullptr hoặc in 'Chuc nang dang duoc phat trien'; không viết fake thành công hay triển khai logic kinh doanh mà chủ module chưa làm.
4. Liệt kê rõ file mỗi TV ĐƯỢC PHÉP sửa (bao gồm header và cpp sở hữu), file KHÔNG ĐƯỢC sửa; ghi hướng dẫn branch/PR cụ thể.
5. Nếu không có compiler thì không nói đã compile; mô tả giới hạn kiểm chứng.

## Kết quả trả về sau khi làm
Tóm tắt cấu trúc, nội dung API đã cố định, lệnh build, những gì đã kiểm chứng, TODO từng TV, file thay đổi. KHÔNG tự push. Dừng ở skeleton để leader review trước khi giao việc.
