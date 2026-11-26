# Dữ liệu demo

Bộ metadata phim thật dành cho V2 nằm trong [`v2/movie_details.txt`](v2/movie_details.txt); định dạng, nguồn và các trường còn thiếu được ghi ở [`docs/MOVIE_DATA_V2.md`](../docs/MOVIE_DATA_V2.md). File này không tự thay năm file V1 hoặc giá vé/suất chiếu demo.

Bộ cụm rạp/phòng/lịch mô phỏng V2 nằm ở [`v2/cinema_demo.txt`](v2/cinema_demo.txt). Quy tắc ngày tương đối, lối đi, VIP và định dạng bản lưu tuyệt đối ở [`docs/CINEMA_V2_DATA.md`](../docs/CINEMA_V2_DATA.md). Không dùng file demo để khôi phục vé đã bán.

Các file `.txt` minh họa định dạng trong `docs/API_CONTRACT.md`. `FileStorage` hiện đọc/ghi phim và vé; file phòng, suất chiếu và khách hàng vẫn là mẫu cho phần mở rộng sau. Mỗi dòng là một record phân cách bằng `|`; dòng bắt đầu `#` là chú thích. Tất cả tên là dữ liệu hư cấu. Không thêm mật khẩu, tài khoản ngân hàng, token hoặc dữ liệu cá nhân thật.
