# Build, đóng gói và khôi phục trên Windows x64

## Build và test

Cài Qt 6 `mingw_64`, MinGW 13.1.0 đi kèm Qt, CMake và Ninja. Ví dụ local dưới đây dùng Qt 6.12.0; workflow Windows dùng Qt 6.9.3 vì `aqtinstall` 3.3.0 trên runner chưa đọc được cấu trúc metadata mới của Qt 6.11/6.12. Chạy PowerShell trong repository:

```powershell
cmake -S . -B build-qt-release -G Ninja -DBUILD_QT_GUI=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64 -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build-qt-release
ctest --test-dir build-qt-release --output-on-failure
./scripts/package_windows.ps1 -BuildDir build-qt-release -QtRoot C:/Qt/6.12.0/mingw_64 -MingwBin C:/Qt/Tools/mingw1310_64/bin
```

Thư mục `dist/CinemaBooking-Windows-x64` chứa exe, DLL Qt/MinGW và plugin; ZIP là `dist/CinemaBooking-Windows-x64.zip`. Script chạy thử exe với PATH chỉ chứa thư mục Windows, rồi mới tạo ZIP. Không đưa source, thư mục build hoặc dữ liệu cá nhân vào ZIP. Workflow `windows-package.yml` lặp lại build, CTest và đóng gói trên Windows, xuất ZIP dưới dạng Actions artifact. Không tạo GitHub Release.

Giải nén toàn bộ ZIP trước khi mở `cinema_booking_qt.exe`; không chạy riêng exe khỏi thư mục DLL/plugin. Máy cần Windows x64 hỗ trợ Qt 6.12 và runtime hệ thống tiêu chuẩn. Dữ liệu lưu ở AppData của người dùng hoặc thư mục họ chọn, không nằm trong ZIP.

## Khôi phục dữ liệu

Mỗi lần lưu thành công, `.cinema-backup` giữ bộ năm file ngay trước lần lưu. Nếu chương trình hoặc Windows dừng giữa lúc thay năm file, đóng chương trình, sao chép năm file trong `.cinema-backup` vào thư mục dữ liệu chính rồi mở lại. Nên sao chép cả thư mục dữ liệu sang nơi khác trước khi khôi phục. Nếu bản sao lưu không đầy đủ, không ghi đè các file đang có; cần kiểm tra thủ công. `.cinema-next` chỉ là dữ liệu tạm, không được tự coi là bản lưu hợp lệ.

Nếu một file bị lỗi hoặc thiếu, chương trình báo lỗi và giữ dữ liệu đang chạy. GUI yêu cầu xác nhận trước khi ghi đè bản lưu lỗi. Cơ chế này bảo vệ tình huống ghi lỗi thông thường nhưng chưa đảm bảo giao dịch nguyên tử khi mất điện, hết dung lượng hoặc ổ đĩa hỏng; nên giữ thêm bản sao bên ngoài cho dữ liệu quan trọng.
