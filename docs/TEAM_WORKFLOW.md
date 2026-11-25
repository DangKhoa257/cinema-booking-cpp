# Quy trình làm việc nhóm

## Quyền sở hữu

| Thành viên | File được phép sửa |
| --- | --- |
| TV1 | `src/main.cpp`, `include/booking_manager.h`, `src/booking_manager.cpp`, `include/cinema_system.h`, `src/cinema_system.cpp`, `CMakeLists.txt`, `.github/workflows/build.yml`, `docs/API_CONTRACT.md`, `docs/ARCHITECTURE.md`, `docs/TEAM_WORKFLOW.md`, `README.md` |
| TV2 | `include/movie.h`, `src/movie.cpp`, `include/movie_catalog.h`, `src/movie_catalog.cpp`, `data/movies.txt`, test riêng cho phim trong `tests/` |
| TV3 | `include/seat.h`, `src/seat.cpp`, `include/cinema_room.h`, `src/cinema_room.cpp`, `data/rooms.txt`, test riêng cho ghế/phòng trong `tests/` |
| TV4 | `include/showtime.h`, `src/showtime.cpp`, `include/schedule_manager.h`, `src/schedule_manager.cpp`, `data/showtimes.txt`, test riêng cho lịch trong `tests/` |
| TV5 | `include/user.h`, `src/user.cpp`, `include/customer.h`, `src/customer.cpp`, `include/admin.h`, `src/admin.cpp`, `include/user_manager.h`, `src/user_manager.cpp`, `data/customers.txt`, test riêng cho người dùng trong `tests/` |
| TV6 | `include/ticket.h`, `src/ticket.cpp`, `include/payment.h`, `src/payment.cpp`, `include/demo_payment.h`, `src/demo_payment.cpp`, `include/bank_transfer_payment.h`, `src/bank_transfer_payment.cpp`, `include/file_storage.h`, `src/file_storage.cpp`, `data/tickets.txt`, test riêng cho vé/thanh toán/lưu trữ trong `tests/` |

`include/common/types.h` là hợp đồng chung, chỉ TV1 sửa sau khi cả nhóm thống nhất. `.gitignore`, `tests/smoke_test.cpp`, `tests/test_booking.cpp`, `tests/console_input.txt`, `tests/console_flow.cmake` và `data/README.md` do TV1 duy trì. `docs/AI_USAGE_LOG.md`, `docs/WEEKLY_LOG.md`, `docs/REPORT_OUTLINE.md` do TV1 tổng hợp; mỗi thành viên gửi phần ghi chép của mình để TV1 cập nhật. Không ai sửa header hoặc `.cpp` của người khác. TV2–TV6 không sửa các file tích hợp của TV1. Nếu cần thay API, gửi đề xuất gồm chữ ký cũ/mới, lý do và module bị ảnh hưởng; chỉ sửa sau khi TV1 và owner liên quan đồng ý, rồi cập nhật `docs/API_CONTRACT.md` trước hoặc cùng PR.

`feature/booking-integration` chứa nguyên các commit TV4 từ PR #3 đang mở để chạy tích hợp. Đây là dependency tạm thời, không có nghĩa PR #3 đã merge vào `main`. Khi TV4 PR merge, TV1 cần đồng bộ branch và kiểm tra lại diff để tránh đưa cùng thay đổi vào hai lần.

## Nhánh và review

1. Tạo nhánh từ nhánh tích hợp mới nhất, ví dụ `git switch -c tv2/movie-catalog` (hoặc `codex/tv2-movie-catalog` nếu Codex tạo nhánh).
2. Chỉ commit file thuộc quyền sở hữu; viết commit ngắn, nêu chức năng. Cập nhật test liên quan và ghi prompt AI/thao tác kiểm chứng vào log gửi TV1.
3. Chạy `cmake -S . -B build`, `cmake --build build`, `ctest --test-dir build --output-on-failure`.
4. Mở PR vào nhánh tích hợp, mô tả thay đổi, kết quả test, TODO và ảnh hưởng API. TV1 review và tích hợp sau khi CI xanh. Không tự merge hoặc push thay người khác.

Tránh commit thư mục `build/` hoặc dữ liệu thật. Bất kỳ code AI đề xuất đều phải được người viết đọc, giải thích được và kiểm thử trước khi đưa vào PR.
