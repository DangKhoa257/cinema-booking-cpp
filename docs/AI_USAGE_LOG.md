# Nhật ký sử dụng AI

Ghi một mục cho mỗi lần dùng AI có ảnh hưởng tới thiết kế, code, test hoặc báo cáo. Không ghi mật khẩu, token hay dữ liệu cá nhân.

| Ngày | Thành viên | Công cụ/model | Prompt hoặc mục tiêu | Kết quả được dùng | Cách kiểm chứng | Chỉnh sửa của người viết |
| --- | --- | --- | --- | --- | --- | --- |
| 2026-11-20 | Codex; chờ TV1 review | Codex | Khởi tạo Giai đoạn 0 theo `00_CODEX_SCAFFOLD_PROMPT.md` | Skeleton, hợp đồng API, docs, CI, smoke test | `g++ -std=c++17 -Wall -Wextra -pedantic` biên dịch hai executable; chạy app và smoke test thành công; CMake/CTest chưa chạy vì không cài trong môi trường | Cần TV1 review API và phân công trước khi giao việc |
| 2026-11-20 | Codex theo yêu cầu kiểm tra trước khi push | Codex | Đối chiếu API/OOP/module và sửa trạng thái ghế theo suất chiếu | Bỏ trạng thái khỏi ghế vật lý; thêm truy vấn `(showtimeId, seatId)` ở TV1; cập nhật tài liệu và smoke test | Biên dịch lại hai executable bằng G++ C++17, chạy app và smoke test thành công; CMake/CTest vẫn chưa có tại máy này | Giữ `bookTicket`/`cancelTicket` là stub; TV1 sẽ triển khai kiểm tra trùng vé |
| 2026-11-25 | Codex theo yêu cầu TV1 | Codex | Tích hợp các module đã merge, đưa commit TV4 vào branch TV1 theo chỉ đạo, triển khai booking/menu và kiểm thử | `BookingManager`, menu console, test booking, đăng ký test CMake, cập nhật hợp đồng | CMake/Ninja/G++ C++17 build; 9 CTest pass; chạy kịch bản console đặt, thanh toán, hủy, lưu/tải thành công; GitHub Actions Ubuntu thành công | PR TV4 vẫn đang mở; cần xử lý dependency trước khi merge branch TV1 |

Thêm dòng mới khi thực hiện công việc tiếp theo; giữ prompt quan trọng và ghi rõ lỗi AI đã được phát hiện/sửa.
