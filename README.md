# Cinema Booking C++ — tích hợp TV1

Ứng dụng console C++17 cho bài tập lớn OOP của nhóm 6 thành viên. Có dữ liệu demo để xem phim, suất, phòng/ghế; đăng ký khách, đặt/hủy vé, thanh toán mô phỏng và lưu/tải dữ liệu. Chuyển khoản demo luôn giữ vé `Pending`, không xác nhận đã nhận tiền thật.

## Build và chạy

Yêu cầu CMake >= 3.16 và compiler hỗ trợ C++17.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Chạy `build/cinema_booking` trên Linux hoặc `build/cinema_booking.exe` trên Windows (đường dẫn có thể khác nếu dùng generator nhiều cấu hình). Khách demo có ID 1; cùng ghế A1 có thể đặt ở hai suất demo khác nhau. Trong menu 9, chọn 5 để lưu cả phim, phòng, suất chiếu, khách và vé vào thư mục mặc định `build/cinema_data`; ở lần chạy sau chọn 6 để tải lại thư mục đó. Có thể nhập đường dẫn thư mục riêng. Các lựa chọn 1–4 vẫn lưu/tải riêng phim hoặc vé như trước. Chương trình tạo dữ liệu demo khi mở, sau đó người dùng tải bản đã lưu khi cần. File sai hoặc thiếu sẽ không thay dữ liệu đang chạy.

Giai đoạn lưu trữ nằm trên branch `feature/persistent-storage`, bắt đầu từ `main` sau PR #6. Các branch tích hợp TV1 trước đó vẫn được giữ nguyên.

Đọc [hợp đồng API](docs/API_CONTRACT.md), [kiến trúc](docs/ARCHITECTURE.md) và [quy trình nhóm](docs/TEAM_WORKFLOW.md) trước khi viết chức năng. Không sửa public API một mình.
