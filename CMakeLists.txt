cmake_minimum_required(VERSION 3.16)
project(cinema_booking_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cinema_core
    src/movie.cpp src/movie_catalog.cpp src/seat.cpp src/cinema_room.cpp
    src/showtime.cpp src/schedule_manager.cpp src/user.cpp src/customer.cpp
    src/admin.cpp src/user_manager.cpp src/ticket.cpp src/payment.cpp
    src/demo_payment.cpp src/bank_transfer_payment.cpp src/file_storage.cpp
    src/booking_manager.cpp src/cinema_system.cpp)
target_include_directories(cinema_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(cinema_booking src/main.cpp)
target_link_libraries(cinema_booking PRIVATE cinema_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(smoke_test tests/smoke_test.cpp)
    target_link_libraries(smoke_test PRIVATE cinema_core)
    add_test(NAME smoke_test COMMAND smoke_test)
endif()
