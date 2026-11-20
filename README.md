# Cinema Booking C++ — Giai đoạn 0

Skeleton C++17 cho bài tập lớn OOP của nhóm 6 thành viên. Ứng dụng console hiện chỉ in thông báo khởi tạo. Các thao tác đặt vé, quản lý phim, ghế, lịch, người dùng, thanh toán và lưu file chưa được triển khai.

## Build và chạy

Yêu cầu CMake >= 3.16 và compiler hỗ trợ C++17.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Chạy `build/cinema_booking` trên Linux hoặc `build/cinema_booking.exe` trên Windows (đường dẫn có thể khác nếu dùng generator nhiều cấu hình).

Đọc [hợp đồng API](docs/API_CONTRACT.md), [kiến trúc](docs/ARCHITECTURE.md) và [quy trình nhóm](docs/TEAM_WORKFLOW.md) trước khi viết chức năng. Không sửa public API một mình.
