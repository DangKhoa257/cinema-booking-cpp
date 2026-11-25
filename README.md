# Cinema Booking C++17

Ứng dụng console C++17 và giao diện Qt 6 Widgets tùy chọn cho bài tập lớn OOP của nhóm 6 thành viên. Có dữ liệu demo để xem phim, suất, phòng/ghế; đăng ký khách, đặt/hủy vé, thanh toán mô phỏng và lưu/tải dữ liệu bằng console. Chuyển khoản demo luôn giữ vé `Pending`, không xác nhận đã nhận tiền thật.

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

Chạy `build-qt/cinema_booking_qt.exe` trên Windows hoặc `build-qt/cinema_booking_qt` trên Linux. Trên Windows, thêm thư mục `bin` của Qt tương ứng vào `PATH` khi chạy ngoài Qt Creator. Nếu Qt 6 Widgets không được tìm thấy, CMake báo warning và vẫn build console; không tạo target GUI. Xem [kiến trúc Qt](docs/QT_UI.md) để biết phạm vi và cách kiểm thử.

Đọc [hợp đồng API](docs/API_CONTRACT.md), [kiến trúc](docs/ARCHITECTURE.md) và [quy trình nhóm](docs/TEAM_WORKFLOW.md) trước khi viết chức năng. Không sửa public API một mình.
