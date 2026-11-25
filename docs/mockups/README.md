# Mockup OOP Cinema 2.0 — Giai đoạn 4A

Mở `oop-cinema-v2.html` trong trình duyệt để xem bản thiết kế tương tác. File tự chứa HTML/CSS/JavaScript, không gọi mạng và không sửa dữ liệu ứng dụng. Có thể mở trực tiếp từng màn hình bằng `?page=home`, `?page=detail`, `?page=schedule`, `?page=seats`, `?page=concessions`, `?page=checkout` hoặc `?page=confirmation`.

Các đường điều hướng, ghế và bộ đếm bắp nước chỉ giúp TV1 review luồng. Phim, suất, rạp và giá hiển thị đều ghi rõ là **minh họa**; nút xác nhận không tạo vé hoặc thanh toán. Đồ họa poster được tạo bằng CSS cho wireframe, không phải poster phim thật. Khi triển khai Qt 4C, UI sẽ lấy dữ liệu từ core/storage, không sao chép dữ liệu hoặc tính giá từ file mockup.

Review theo thứ tự: trang chủ → chi tiết phim → rạp/lịch → ghế → bắp nước → checkout → trạng thái đơn. Đối chiếu [đặc tả](../OOP_CINEMA_V2_SPEC.md) trước khi chốt API 4B.
