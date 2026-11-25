# Cinema Booking C++ — tích hợp TV1

Ứng dụng console C++17 cho bài tập lớn OOP của nhóm 6 thành viên. Có dữ liệu demo để xem phim, suất, phòng/ghế; đăng ký khách, đặt/hủy vé, thanh toán mô phỏng và lưu/tải phim hoặc vé. Chuyển khoản demo luôn giữ vé `Pending`, không xác nhận đã nhận tiền thật.

## Build và chạy

Yêu cầu CMake >= 3.16 và compiler hỗ trợ C++17.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Chạy `build/cinema_booking` trên Linux hoặc `build/cinema_booking.exe` trên Windows (đường dẫn có thể khác nếu dùng generator nhiều cấu hình). Khách demo có ID 1; cùng ghế A1 có thể đặt ở hai suất demo khác nhau. Menu lưu/tải hiện dùng API `FileStorage` cho phim và vé; dữ liệu phòng, lịch và khách chỉ có trong phiên chạy. Đường dẫn mặc định là `build/movies_saved.txt` và `build/tickets_saved.txt`.

Branch `feature/tv1-final-integration` được tạo từ `main` sau khi PR TV4 #3 merge, rồi chỉ cherry-pick bốn commit TV1. Branch `feature/booking-integration` cũ được giữ nguyên làm bản dự phòng.

Đọc [hợp đồng API](docs/API_CONTRACT.md), [kiến trúc](docs/ARCHITECTURE.md) và [quy trình nhóm](docs/TEAM_WORKFLOW.md) trước khi viết chức năng. Không sửa public API một mình.
