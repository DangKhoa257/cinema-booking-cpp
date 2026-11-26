# Cụm rạp, phòng, ghế và lịch V2 (Giai đoạn 4B.2)

`data/v2/cinema_demo.txt` là dữ liệu **giả định** của thương hiệu OOP Cinema. Tên bốn khu vực chỉ là địa bàn, không phải địa chỉ kinh doanh có thật. Giá trong file là giá nội bộ đặt cho demo, không sao chép từ rạp khác. Metadata phim nằm riêng ở `data/v2/movie_details.txt` và được kiểm tra theo `movieId` khi tải.

## Mô hình và API

- `CinemaBranch` có ID, tên, khu vực, địa chỉ mô tả, trạng thái hoạt động và danh sách `roomIds`. `BranchCatalog` thêm/tìm cụm và gắn mỗi `roomId` đúng một cụm. Quan hệ phải được đối chiếu với danh sách `CinemaRoom` ở `CinemaV2`.
- `RoomSeatLayout` sinh ghế vật lý từ số hàng, số ghế mỗi hàng, vị trí lối đi và hàng VIP đầu tiên. `SeatPosition` chứa `seatId`, hàng, số và cột hiển thị. Cột lối đi không tạo `Seat`; `SeatCategory` chỉ có `Standard`/`VIP`. Mọi phòng trong file demo có 80–120 ghế. `Seat`/`CinemaRoom` V1 không bị sửa.
- `CinemaV2` giữ catalog cụm, vector phòng, layout và `ScheduleManager`. `addShowtime` kiểm tra phim/phòng, giá dương, thời lượng đúng metadata phim, xung đột cùng phòng **kể cả thời gian vệ sinh**. Hai phòng khác nhau có thể chiếu đồng thời. `showtimesFor` lọc theo movieId, branchId, ngày.
- `currentVietnamTime`, `isBookable` và `CinemaV2::bookTicket` kiểm tra suất chưa bắt đầu theo **Asia/Ho_Chi_Minh, UTC+07** trước khi gọi `BookingManager` V1. `BookingManager` vẫn giữ quy tắc ghế theo `(showtimeId, seatId)`; layout không lưu `Available/Held/Booked`. Để bảo đảm chính sách V2, tầng UI/tích hợp 4C **phải gọi `CinemaV2::bookTicket`**, không gọi trực tiếp `BookingManager::bookTicket` V1. Vé cũ, hủy vé và thanh toán demo giữ hành vi V1.
- `CinemaV2::load` parse toàn file vào object tạm và chỉ thay bộ nhớ sau khi mọi tham chiếu/ID/layout/xung đột hợp lệ. `save` là thao tác tường minh, sao lưu file cũ thành `.bak` trước khi thay. Cả hai nhận đường dẫn UTF-8, dùng `std::filesystem::u8path` để hỗ trợ Windows có dấu. Không thay năm file V1 hoặc tự ghi đè dữ liệu người dùng khi khởi động.

Con trỏ trả từ `find`, `layout`, `showtimesFor` chỉ dùng khi catalog/lịch chưa bị thêm hoặc nạp lại: các vector có thể dời bộ nhớ. Giữ ID khi qua bước giao diện.

## Định dạng file

Một file UTF-8, một record mỗi dòng, `|` phân cách, không escape. Text không chứa `|` hoặc xuống dòng. Dòng trống và dòng bắt đầu `#` được bỏ qua; BOM UTF-8 đầu file được chấp nhận. Thứ tự bắt buộc: `C` đầu tiên, sau đó `B` trước `R`, và `R` trước `T`/`S` tham chiếu tới nó. Dữ liệu lỗi giữ nguyên object đang hoạt động.

| Loại | Cột | Ý nghĩa |
| --- | --- | --- |
| `C` | `C|cleaningMinutes` | Phút vệ sinh/chuyển ca 0–180; mặc định trong lớp là 15, file phải khai báo rõ. |
| `B` | `B|id|name|area|address|active` | `active` là `1` hoặc `0`. Địa chỉ demo chỉ ghi khu vực. |
| `R` | `R|roomId|branchId|name|rowCount|seatsPerRow|aisleAfter|vipFromRow` | Hàng `A`…; lối đi sau ghế số `aisleAfter`; từ `vipFromRow` là VIP. Layout sinh 80–120 ghế. |
| `T` | `T|showtimeId|movieId|roomId|dayOffset|HH:MM|priceVnd` | **Chỉ file mẫu:** ngày chiếu là ngày Việt Nam lúc nạp cộng 1–30 ngày. `endTime` sinh từ thời lượng phim. |
| `S` | `S|showtimeId|movieId|roomId|startTime|endTime|priceVnd` | Bản lưu vận hành: thời gian tuyệt đối `YYYY-MM-DD HH:MM` theo giờ Việt Nam. `endTime` là kết thúc phim, chưa cộng phút vệ sinh. |

`CinemaV2::save` luôn ghi `S` với giờ tuyệt đối. File mẫu `T` sẽ sinh lịch tương lai khi được **chọn nạp demo**; không dùng lại file `T` để khôi phục vé đã bán, vì ID suất giữ nguyên nhưng thời gian sẽ trôi. Khi có vé, lưu bản `S` và tải đúng bản đó cùng vé ở bước tích hợp dữ liệu V2 sau này. Nếu bản lưu cũ chứa suất đã qua giờ, vẫn cho đọc lịch sử nhưng `isBookable` từ chối vé mới. `CinemaV2` chưa tham gia giao dịch snapshot năm file V1; 4B migration phải điều phối đồng bộ vé với lịch V2.

## Bộ demo và giới hạn

File mẫu có **4 cụm**, **12 phòng** (mỗi cụm 3), **1.144 ghế** và **32 quy tắc suất** trải trên ngày kế tiếp và ngày sau nữa. Ba layout lặp theo các cụm: 10×10, 8×12 và 9×10; hàng cuối là VIP, có một cột lối đi giữa. Giá 70.000–90.000 VND là giả lập nội bộ; không có ghế đôi hoặc phụ thu VIP ở 4B.2. Không gán địa chỉ phố cụ thể. Dữ liệu được nạp từ file vào core, không hardcode trong QWidget; GUI 3C và console V1 tiếp tục chạy riêng.

**Giới hạn cần 4C/4B migration xử lý:** nạp metadata phim trước file rạp; nối `CinemaV2` với phiên người dùng và snapshot vé trước khi thay bộ demo trong GUI; không tạo lịch từ template mỗi lần mở bản lưu có vé. `BookingManager` V1 trực tiếp chưa biết thời gian, nên đường đặt vé V2 phải đi qua cổng `CinemaV2::bookTicket`. Poster và tìm kiếm Unicode là việc 4C theo quyết định TV1.
