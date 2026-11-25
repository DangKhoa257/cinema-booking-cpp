# Nhật ký phát triển theo tuần

| Tuần / ngày | Thành viên | Việc đã làm | Bằng chứng / test | Vướng mắc | Kế hoạch tuần sau |
| --- | --- | --- | --- | --- | --- |
| Khởi tạo / 2026-11-20 | Codex; chờ TV1 review | Tạo skeleton Giai đoạn 0 | Build và chạy smoke test bằng `g++`; CMake/CTest chưa chạy tại máy này | Cần review mô hình ghế theo suất | TV1 chốt API và giao module |
| Kiểm tra trước khi push / 2026-11-20 | Codex theo yêu cầu của người dùng | Đối chiếu API/OOP/ranh giới module; sửa mô hình ghế theo suất | G++ C++17 build và smoke test thành công | CMake/CTest không có trên máy; CI sẽ kiểm tra trên Ubuntu | TV1 giao module và triển khai đặt/hủy vé |
| Tích hợp TV1 / 2026-11-25 | Codex theo yêu cầu TV1 | Triển khai đặt/hủy/thanh toán demo, menu, test và tài liệu API | CMake build; 9 CTest pass; kịch bản console đầy đủ và GitHub Actions Ubuntu pass | PR TV4 chưa merge; branch TV1 chứa commit TV4 tạm thời | Review thứ tự merge và quyết định phạm vi lưu trữ |

Mỗi thành viên gửi cập nhật thật theo tuần. Không điền tiến độ giả cho TV2–TV6.
