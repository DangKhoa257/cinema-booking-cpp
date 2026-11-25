# OOP Cinema — Cinema Booking C++17

> **OOP Cinema 2.0:** [đặc tả đã duyệt](docs/OOP_CINEMA_V2_SPEC.md), [mockup 4A](docs/mockups/oop-cinema-v2.html) và [danh mục phim V2 4B.1](docs/MOVIE_DATA_V2.md). Ứng dụng console/Qt chạy thật hiện vẫn là phiên bản 3C; 4C sẽ nối danh mục mới vào GUI.

Ứng dụng console C++17 và giao diện Qt 6 Widgets tùy chọn cho bài tập lớn OOP của nhóm hiện có 5 thành viên. Cả hai giao diện đều xem phim/suất/phòng, đăng ký khách, đặt/hủy vé, thanh toán mô phỏng và lưu/tải dữ liệu. Chuyển khoản demo luôn giữ vé `Pending`, không xác nhận đã nhận tiền thật.

## Build và chạy

Yêu cầu CMake >= 3.16 và compiler hỗ trợ C++17.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Chạy `build/cinema_booking` trên Linux hoặc `build/cinema_booking.exe` trên Windows (đường dẫn có thể khác nếu dùng generator nhiều cấu hình). Khách demo có ID 1; cùng ghế A1 có thể đặt ở hai suất demo khác nhau. Trong menu 9, chọn 5 để lưu cả phim, phòng, suất chiếu, khách và vé vào thư mục mặc định `build/cinema_data`; ở lần chạy sau chọn 6 để tải lại thư mục đó. Có thể nhập đường dẫn thư mục riêng. Các lựa chọn 1–4 vẫn lưu/tải riêng phim hoặc vé như trước. Chương trình tạo dữ liệu demo khi mở, sau đó người dùng tải bản đã lưu khi cần. File sai hoặc thiếu sẽ không thay dữ liệu đang chạy.

## Giao diện Qt 6

Target Qt được tắt mặc định để máy không có Qt vẫn build và test console. Bật bằng `-DBUILD_QT_GUI=ON` và trỏ CMake tới bản Qt 6 Widgets phù hợp với compiler. Ví dụ Windows với Qt 6.12.0 và MinGW của Qt:

```sh
cmake -S . -B build-qt -G Ninja -DBUILD_QT_GUI=ON -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64 -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build-qt
ctest --test-dir build-qt --output-on-failure
```

Chạy `build-qt/cinema_booking_qt.exe` trên Windows hoặc `build-qt/cinema_booking_qt` trên Linux. Trên Windows, thêm thư mục `bin` của Qt và MinGW tương ứng vào `PATH` khi chạy ngoài Qt Creator. Nếu Qt 6 Widgets không được tìm thấy, CMake báo warning và vẫn build console; không tạo target GUI.

Trong GUI, chọn phim → suất → ghế → khách và phương thức demo, kiểm tra dòng xác nhận rồi đặt vé. Thanh phía trên có **Thư mục…**, **Tải**, **Lưu**. Vị trí mặc định do Qt `QStandardPaths::AppDataLocation` cung cấp, nằm ngoài thư mục build; có thể chọn thư mục khác. Khi mở, ứng dụng tự tải bản lưu hợp lệ nếu có file dữ liệu. Nếu thiếu hoặc lỗi file, dữ liệu demo vẫn dùng được và giao diện báo lỗi; không tự ghi đè bản lưu lỗi. Dấu **Chưa lưu** nhắc lưu sau khi đặt/hủy/thanh toán hoặc thêm khách. Xem [kiến trúc Qt](docs/QT_UI.md) để biết phạm vi và cách kiểm thử.

Khi thoát, tải bản khác hoặc chuyển thư mục mà còn thay đổi, ứng dụng hỏi **Lưu – Không lưu – Hủy**. Khi lưu, năm file được ghi vào vùng tạm và kiểm tra trước khi thay thế; bộ file trước đó được giữ ở `.cinema-backup` trong thư mục dữ liệu. Nếu quá trình thay thế bị gián đoạn, xem [hướng dẫn khôi phục](docs/WINDOWS_PACKAGE.md). Không đặt dữ liệu cá nhân thật trong bản demo.

## Bản Windows x64

Trên trang Actions của PR, tải artifact `CinemaBooking-Windows-x64` rồi giải nén ZIP. Mở `CinemaBooking-Windows-x64/cinema_booking_qt.exe`; giữ các DLL và thư mục `platforms` cạnh file exe. Bản ZIP chứa runtime Qt/MinGW, không cần cài Qt trên máy nhận. Hướng dẫn build, đóng gói, kiểm tra và khôi phục bản lưu: [WINDOWS_PACKAGE.md](docs/WINDOWS_PACKAGE.md). Ảnh chụp giao diện Qt Windows thật: [trang chủ](docs/screenshots/home-1120x760.png), [sơ đồ ghế 1120×760](docs/screenshots/seat-1120x760.png), [sơ đồ ghế 860×620](docs/screenshots/seat-860x620.png).

Đọc [hợp đồng API](docs/API_CONTRACT.md), [kiến trúc](docs/ARCHITECTURE.md) và [quy trình nhóm](docs/TEAM_WORKFLOW.md) trước khi viết chức năng. Không sửa public API một mình.
