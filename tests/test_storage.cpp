#include "file_storage.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

static void write(const std::string& p, const std::string& s) {
    std::ofstream(p, std::ios::binary) << s;
}

int main() {
    FileStorage fs;
    const std::string mp = "t_movies.txt", tp = "t_tickets.txt";

    // Luu roi tai lai phim
    std::vector<Movie> movies{Movie(1, "Phim A", 90, "Hanh dong", 50000),
                              Movie(2, "Phim B", 120, "Tinh cam", 75000)};
    assert(fs.saveMovies(movies, mp));
    std::vector<Movie> m2;
    assert(fs.loadMovies(m2, mp));
    assert(m2.size() == 2 && m2[1].getTitle() == "Phim B" && m2[1].getBasePriceVnd() == 75000 &&
           m2[0].getDurationMinutes() == 90 && m2[0].getGenre() == "Hanh dong");

    // Luu roi tai lai ve (du 3 trang thai)
    std::vector<Ticket> tickets{Ticket(1, 1, 1, "A1", 50000, TicketStatus::Pending),
                                Ticket(2, 1, 1, "A2", 50000, TicketStatus::Paid),
                                Ticket(3, 2, 1, "A1", 50000, TicketStatus::Cancelled)};
    assert(fs.saveTickets(tickets, tp));
    std::vector<Ticket> t2;
    assert(fs.loadTickets(t2, tp));
    assert(t2.size() == 3);
    assert(t2[0].getStatus() == TicketStatus::Pending && t2[1].getStatus() == TicketStatus::Paid &&
           t2[2].getStatus() == TicketStatus::Cancelled && t2[2].getSeatId() == "A1" &&
           t2[1].getPriceVnd() == 50000 && t2[2].getCustomerId() == 2);

    // File thieu: bao loi, khong dong vao vector dau ra
    std::vector<Ticket> keep{Ticket(9, 9, 9, "Z9", 1)};
    assert(!fs.loadTickets(keep, "khong_ton_tai.txt"));
    assert(!fs.lastError().empty() && keep.size() == 1);

    // File trong / chi co comment -> danh sach rong hop le
    write(tp, "");
    assert(fs.loadTickets(t2, tp) && t2.empty());
    write(tp, "# chi comment\n\n");
    assert(fs.loadTickets(t2, tp) && t2.empty());

    // File hong: that bai va vector giu nguyen
    const char* badTickets[] = {
        "1|1|1|A1|50000\n",               // thieu cot
        "1|1|1|A1|abc|Pending\n",         // gia khong phai so
        "1|1|1|A1|-5|Pending\n",          // gia am
        "1|1|1|A1|50000|Done\n",          // enum sai
        "1|1|1|a1|50000|Pending\n",       // seatId sai
        "0|1|1|A1|50000|Pending\n",       // id khong duong
        "1|1|1|A1|50000|Pending\n1|1|1|A2|50000|Paid\n",  // trung id
        "1|1|1|A1|50000|Pending\n2|2|1|A1|50000|Paid\n",  // trung ghe hoat dong
    };
    for (const char* bad : badTickets) {
        write(tp, bad);
        std::vector<Ticket> out{Ticket(7, 7, 7, "B7", 7)};
        assert(!fs.loadTickets(out, tp));
        assert(out.size() == 1 && out[0].getId() == 7);
        assert(!fs.lastError().empty());
    }
    write(mp, "1|Phim|0|Genre|1000\n");  // thoi luong 0
    std::vector<Movie> mo{Movie(5, "Giu", 1, "G", 0)};
    assert(!fs.loadMovies(mo, mp) && mo.size() == 1);

    // Du lieu mau + CRLF
    write(mp, "# c\r\n1|Phim Demo|90|Gia tuong|50000\r\n");
    assert(fs.loadMovies(mo, mp) && mo.size() == 1 && mo[0].getTitle() == "Phim Demo");

    // Save tu choi ky tu '|' va khong pha file cu
    assert(fs.saveMovies(movies, mp));
    std::vector<Movie> badSave{Movie(3, "A|B", 90, "G", 1)};
    assert(!fs.saveMovies(badSave, mp));
    assert(fs.loadMovies(m2, mp) && m2.size() == 2);

    std::remove(mp.c_str()); std::remove(tp.c_str());
    std::cout << "test_storage OK\n";
}
