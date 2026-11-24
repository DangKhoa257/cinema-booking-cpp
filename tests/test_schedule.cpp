#include <iostream>

#include "schedule_manager.h"
#include "showtime.h"

static int failures = 0;

static void check(bool cond, const char* name) {
    std::cout << (cond ? "PASS: " : "FAIL: ") << name << "\n";
    if (!cond) ++failures;
}

int main() {
    ScheduleManager m;
    check(m.addShowtime(Showtime(1, 1, 1, "2026-12-01 10:00", "2026-12-01 12:00", 80000)), "them suat hop le");
    check(!m.addShowtime(Showtime(1, 1, 2, "2026-12-02 10:00", "2026-12-02 12:00", 80000)), "trung ID bi tu choi");
    check(!m.addShowtime(Showtime(2, 1, 1, "2026-12-01 11:00", "2026-12-01 13:00", 80000)), "cung phong giao nhau bi tu choi");
    check(m.addShowtime(Showtime(3, 1, 2, "2026-12-01 10:00", "2026-12-01 12:00", 80000)), "khac phong cung gio hop le");
    check(m.addShowtime(Showtime(4, 1, 1, "2026-12-01 12:00", "2026-12-01 14:00", 80000)), "noi tiep 12:00 hop le");
    check(!m.addShowtime(Showtime(5, 1, 3, "2026-12-02 12:00", "2026-12-02 12:00", 80000)), "start == end bi tu choi");
    check(!m.addShowtime(Showtime(6, 1, 3, "2026-12-02 13:00", "2026-12-02 12:00", 80000)), "start > end bi tu choi");
    check(!m.addShowtime(Showtime(7, 1, 3, "2026-02-30 10:00", "2026-02-30 12:00", 80000)), "ngay 30/02 bi tu choi");
    check(m.addShowtime(Showtime(8, 1, 3, "2028-02-29 10:00", "2028-02-29 12:00", 80000)), "29/02 nam nhuan hop le");
    check(!m.addShowtime(Showtime(9, 1, 3, "2027-02-29 10:00", "2027-02-29 12:00", 80000)), "29/02 nam khong nhuan bi tu choi");
    check(!m.addShowtime(Showtime(10, 1, 3, "0000-01-01 10:00", "0000-01-01 12:00", 80000)), "nam 0000 bi tu choi");
    check(!m.addShowtime(Showtime(11, 1, 3, "2026-12-03 10:00", "2026-12-03 12:00", -1)), "gia am bi tu choi");
    check(!m.addShowtime(Showtime(0, 1, 3, "2026-12-03 10:00", "2026-12-03 12:00", 80000)), "ID khong hop le bi tu choi");
    check(!m.addShowtime(Showtime(12, 1, 0, "2026-12-03 10:00", "2026-12-03 12:00", 80000)), "roomId <= 0 bi tu choi");
    check(m.findShowtime(1) != nullptr, "tim ID ton tai");
    check(m.findShowtime(999) == nullptr, "tim ID khong ton tai");
    check(!m.hasRoomConflict(1, "2026-12-05 10:00", "2026-12-05 12:00"), "hasRoomConflict: khong xung dot");
    check(m.hasRoomConflict(1, "2026-12-01 10:30", "2026-12-01 11:30"), "hasRoomConflict: xung dot");

    return failures == 0 ? 0 : 1;
}