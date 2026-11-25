# Movie Catalog V2 — định dạng và nguồn dữ liệu (4B.1)

Ngày đối chiếu: **2026-11-25 (Asia/Saigon)**. `data/v2/movie_details.txt` là bộ phim để tích hợp ở 4C, **không phải** lịch chiếu hay giá của OOP Cinema. Trạng thái `NowShowing` là ảnh chụp biên tập ngày đối chiếu, không tự động theo thời gian. Trước khi demo/phát hành ở ngày khác phải kiểm tra lại trang chi tiết và lịch rạp; không tự coi trạng thái cũ là còn hiệu lực.

## API và tương thích

`MovieDetails` chứa metadata theo `movieId`; `MovieDetailsCatalog` cung cấp `find`, `all`, `nowShowing`, `comingSoon`, `searchTitle`, `filterGenre`, `posterPath`, `load`, `save`. `MovieDetails::toV1Movie(basePriceVnd)` chỉ chuyển sang `Movie` khi caller cung cấp **giá cơ bản nội bộ OOP Cinema**. Không lấy giá của rạp nguồn hoặc tự đặt giá 0. Không sửa `Movie`, `MovieCatalog`, `FileStorage`, `CinemaDataStore`, năm file V1 hoặc `UiSession`. GUI hiện vẫn dùng hai phim demo cho đến khi 4C nối catalog mới; dữ liệu người dùng trong năm file V1 không bị thay.

Đường dẫn truyền cho `load`/`save` là UTF-8 và được chuyển bằng `std::filesystem::u8path`, kể cả thư mục Windows có dấu. `load` parse toàn bộ vào catalog tạm, kiểm tra ID trùng, số cột, escape, ngày, URL, UTF-8, rồi mới thay bộ nhớ. `save` là thao tác tường minh, không chạy khi khởi động. Nếu file đích có sẵn, lưu bản cũ `.bak` trước khi thay; lỗi ghi/đổi file không được thay dữ liệu trong bộ nhớ. Bản sao `.bak` cũng là đường khôi phục thủ công nếu hệ thống mất điện giữa bước thay file. Đây là **một file độc lập**, chưa tham gia giao dịch năm file V1; 4B migration sẽ xử lý snapshot đa file.

Kết quả tìm kiếm trả con trỏ tới phần tử catalog, chỉ dùng khi catalog không bị `add`/`load`; không giữ qua lần nạp lại. Tìm kiếm không phân biệt chữ hoa/thường ASCII, giữ nguyên byte UTF-8 tiếng Việt có dấu (ví dụ tìm `Điện` được, `điện` có thể chưa khớp). 4C có thể dùng `QString` để tìm kiếm tiếng Việt không phân biệt dấu/hoa thường. `posterPath` trả chuỗi rỗng khi asset thiếu hoặc ra ngoài thư mục asset; GUI vẽ đồ họa thay thế rõ ràng, không giả làm poster chính thức.

## File UTF-8

Mỗi dòng một phim, **15 cột** theo thứ tự:

`movieId|title|originalTitle|genre|durationMinutes|ageRating|releaseDate|director|cast|synopsis|posterAsset|trailerUrl|availability|sourceUrl|verifiedAt`

- `movieId` và `durationMinutes` là số nguyên dương. `title`, `genre`, `sourceUrl`, `verifiedAt` bắt buộc. `sourceUrl` và `trailerUrl` nếu có phải là HTTPS. `verifiedAt`/`releaseDate` dùng `YYYY-MM-DD`; `releaseDate` có thể rỗng khi chưa xác minh.
- `originalTitle`, `ageRating`, `director`, `cast`, `synopsis`, `posterAsset`, `trailerUrl` có thể rỗng; không đặt giá trị suy đoán. `availability` là `NowShowing` hoặc `ComingSoon` và phải được xác minh lại khi cập nhật.
- `posterAsset` là đường dẫn **tương đối** dưới thư mục asset do caller chọn; không chấp nhận `..` hoặc đường dẫn tuyệt đối. Bộ seed chưa có poster vì chưa có giấy phép sử dụng. Trailer cũng để trống khi chưa kiểm tra được URL và quyền dùng.
- Dùng `\|`, `\\`, `\n`, `\r` cho dấu `|`, gạch chéo ngược và xuống dòng trong trường. Dòng bắt đầu `#` và dòng trống được bỏ qua; BOM UTF-8 đầu file được chấp nhận. `title`/`genre` vẫn không chứa `|` hoặc xuống dòng để có thể chuyển sang `Movie` V1.
- Bộ dữ liệu này không ghi giá vé, địa chỉ rạp, phòng hay suất chiếu của Galaxy. `Movie::basePriceVnd` chỉ được đặt sau khi OOP Cinema có chính sách giá riêng.

## Phim đã đối chiếu

Trang chi tiết của Galaxy cung cấp tên, thể loại, thời lượng, phân loại khi có, ngày chiếu, đoàn phim. [Lịch rạp Galaxy CineO Coop Quang Trung](https://www.galaxycine.vn/rap-gia-ve/galaxy-quang-trung/) và [lịch Galaxy Cine+ Thiso Mall Phan Huy Ích](https://www.galaxycine.vn/rap-gia-ve/galaxy-cine-thiso-phan-huy-ich/) liệt kê ngày **25/11** cùng các phim trong danh mục; [lịch Galaxy Cinema Nha Trang Center](https://www.galaxycine.vn/rap-gia-ve/galaxy-nha-trang-center/) hỗ trợ kiểm tra thêm. Trang rạp có thể thay đổi theo khu vực và giờ truy cập; không sao chép suất sang OOP Cinema.

| ID | Phim | Thời lượng / tuổi | Trang chi tiết nguồn | Ghi chú |
| --- | --- | --- | --- | --- |
| 101 | Godzilla Trừ Không | 135 phút / T16 | [Galaxy](https://www.galaxycine.vn/dat-ve/godzilla-minus-zero/) | Tên gốc *Godzilla Minus Zero*. |
| 102 | Bò Sữa Bay | 108 phút / T18 | [Galaxy](https://www.galaxycine.vn/dat-ve/bo-sua-bay/) | Trang nguồn ghi suất sớm 05/11, khởi chiếu chính 06/11. |
| 103 | Trại Giam Hạnh Phúc | 127 phút / T16 | [Galaxy](https://www.galaxycine.vn/dat-ve/trai-giam-hanh-phuc/) | Trang nguồn ghi suất sớm 18–19/11, khởi chiếu chính 20/11. |
| 104 | Thòng Lọng Ma: Quỷ Cái Đòi Mạng | 105 phút / T18 | [Galaxy](https://www.galaxycine.vn/dat-ve/the-rope-curse-4-kuntilanak/) | Tên gốc *The Rope Curse 4: Kuntilanak*. |
| 105 | Đấu Trường Sinh Tử: Bình Minh Ngày Chiêu Quân | 158 phút / T16 | [Galaxy](https://www.galaxycine.vn/dat-ve/the-hunger-games-sunrise-on-the-reaping/) | Tên gốc trong file nguồn. |
| 107 | Kiến Lé Phiêu Lưu Ké | 77 phút / **chưa công bố** | [Galaxy](https://www.galaxycine.vn/dat-ve/arnie--barney/) | Không suy đoán phân loại tuổi. |
| 108 | Ma Mút Con Vượt Kỷ Băng Hà | 73 phút / **chưa công bố** | [Galaxy](https://www.galaxycine.vn/dat-ve/mother-for-a-little-mammoth/) | Không thấy diễn viên và phân loại tuổi trên trang nguồn. |

Các tóm tắt trong file là diễn đạt ngắn của nhóm, không chép nguyên văn mô tả nguồn. **Thiếu:** poster của cả bảy phim, trailer đã xác minh của cả bảy, phân loại tuổi của 107/108, diễn viên của 108. Trang `Ramayana: Khởi Nguồn Một Huyền Thoại` ghi **0 phút**, nên chưa đưa vào file vì `Movie` V1 yêu cầu thời lượng dương. `Chị Chị Em Em 3` cũng chưa đưa vào vì [trang đặt vé](https://www.galaxycine.vn/dat-ve/chi-chi-em-em-3/) và [trang phim cũ](https://www.galaxycine.vn/phim/chi-chi-em-em-3/) ghi ngày khởi chiếu khác nhau. Không nhúng ảnh/trailer từ website rạp nếu chưa có quyền.

## Kiểm thử và cập nhật

`test_movie_details` kiểm tra ID trùng, ngày/UTF-8/đường dẫn lỗi, tìm kiếm/lọc, round trip ở đường dẫn Unicode, bản sao lưu và bộ nhớ khi file hỏng. `test_movie_catalog_data` nạp bộ phim thật và xác nhận `FileStorage::loadMovies` vẫn đọc file V1. Khi cập nhật dữ liệu, đổi `verifiedAt`, kiểm tra từng trường với URL phim, đối chiếu lịch rạp gần ngày cập nhật, chạy CTest, rồi review riêng thay đổi dữ liệu. Không tự động cập nhật status từ nguồn trực tuyến trong bản desktop.
