
#include "movie.h"
#include "movie_catalog.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Kiem tra constructor tu choi du lieu khong hop le
void expectInvalidMovie(int id, std::string title, int duration,
                        std::string genre, long long price) {
    bool thrown = false;

    try {
        Movie movie(id, title, duration, genre, price);
    }
    catch (const std::invalid_argument&) {
        thrown = true;
    }

    assert(thrown);
}

int main() {
    // 1. Tao hai phim hop le
    Movie movie1(1, "Avengers", 180, "Action", 90000);
    Movie movie2(2, "Doraemon", 100, "Animation", 70000);

    assert(movie1.getId() == 1);
    assert(movie1.getTitle() == "Avengers");
    assert(movie1.getDurationMinutes() == 180);
    assert(movie1.getGenre() == "Action");
    assert(movie1.getBasePriceVnd() == 90000);

    // 2. Kiem tra MovieCatalog
    MovieCatalog catalog;

    assert(catalog.getMovies().empty());

    assert(catalog.addMovie(movie1));
    assert(catalog.addMovie(movie2));

    // Khong cho phep ID trung
    assert(!catalog.addMovie(movie1));

    // 3. Tim phim
    const Movie* found = catalog.findMovie(1);
    assert(found != nullptr);
    assert(found->getTitle() == "Avengers");

    assert(catalog.findMovie(999) == nullptr);

    // 4. Kiem tra danh sach
    assert(catalog.getMovies().size() == 2);

    // 5. Xoa phim
    assert(catalog.removeMovie(1));
    assert(catalog.findMovie(1) == nullptr);
    assert(catalog.getMovies().size() == 1);

    // Khong xoa duoc phim khong ton tai
    assert(!catalog.removeMovie(999));

    // 6. Kiem tra du lieu khong hop le
    expectInvalidMovie(0, "Film", 100, "Action", 50000);
    expectInvalidMovie(-1, "Film", 100, "Action", 50000);
    expectInvalidMovie(3, "", 100, "Action", 50000);
    expectInvalidMovie(3, "   ", 100, "Action", 50000);
    expectInvalidMovie(3, "Film", 0, "Action", 50000);
    expectInvalidMovie(3, "Film", -10, "Action", 50000);
    expectInvalidMovie(3, "Film", 100, "", 50000);
    expectInvalidMovie(3, "Film", 100, "   ", 50000);
    expectInvalidMovie(3, "Film", 100, "Action", -1);

    // 7. Kiem tra operator<<
    std::ostringstream output;
    output << movie2;

    assert(output.str().find("Doraemon") != std::string::npos);
    assert(output.str().find("70000") != std::string::npos);

    std::cout << "All TV2 Movie tests passed!" << std::endl;

    return 0;
}
