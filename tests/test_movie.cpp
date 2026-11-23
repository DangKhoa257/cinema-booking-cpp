#include "movie.h"
#include "movie_catalog.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

void expectInvalidMovie(int id, const std::string& title, int duration,
                        const std::string& genre, long long price) {
    bool thrown = false;

    try {
        Movie movie(id, title, duration, genre, price);
        (void)movie;
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    assert(thrown);
}

int main() {
    // Valid movies and getters.
    Movie movie1(1, "Avengers", 180, "Action", 90000);
    Movie movie2(2, "Doraemon", 100, "Animation", 70000);

    assert(movie1.getId() == 1);
    assert(movie1.getTitle() == "Avengers");
    assert(movie1.getDurationMinutes() == 180);
    assert(movie1.getGenre() == "Action");
    assert(movie1.getBasePriceVnd() == 90000);

    // Catalog operations and duplicate IDs.
    MovieCatalog catalog;
    assert(catalog.getMovies().empty());
    assert(catalog.addMovie(movie1));
    assert(catalog.addMovie(movie2));
    assert(!catalog.addMovie(movie1));

    const Movie* found = catalog.findMovie(1);
    assert(found != nullptr);
    assert(found->getTitle() == "Avengers");
    assert(catalog.findMovie(999) == nullptr);
    assert(catalog.getMovies().size() == 2);

    assert(catalog.removeMovie(1));
    assert(catalog.findMovie(1) == nullptr);
    assert(catalog.getMovies().size() == 1);
    assert(!catalog.removeMovie(999));

    // Existing invalid-data tests.
    expectInvalidMovie(0, "Film", 100, "Action", 50000);
    expectInvalidMovie(-1, "Film", 100, "Action", 50000);
    expectInvalidMovie(3, "", 100, "Action", 50000);
    expectInvalidMovie(3, "   ", 100, "Action", 50000);
    expectInvalidMovie(3, "Film", 0, "Action", 50000);
    expectInvalidMovie(3, "Film", -10, "Action", 50000);
    expectInvalidMovie(3, "Film", 100, "", 50000);
    expectInvalidMovie(3, "Film", 100, "   ", 50000);
    expectInvalidMovie(3, "Film", 100, "Action", -1);

    // A pipe, LF, or CR is forbidden in both serialized text fields.
    expectInvalidMovie(3, "Bad|Title", 100, "Action", 50000);
    expectInvalidMovie(3, "Bad\nTitle", 100, "Action", 50000);
    expectInvalidMovie(3, "Bad\rTitle", 100, "Action", 50000);
    expectInvalidMovie(3, "Film", 100, "Bad|Genre", 50000);
    expectInvalidMovie(3, "Film", 100, "Bad\nGenre", 50000);
    expectInvalidMovie(3, "Film", 100, "Bad\rGenre", 50000);

    // operator<< output.
    std::ostringstream output;
    output << movie2;
    assert(output.str().find("Doraemon") != std::string::npos);
    assert(output.str().find("70000") != std::string::npos);

    std::cout << "All TV2 Movie tests passed!" << std::endl;
    return 0;
}
