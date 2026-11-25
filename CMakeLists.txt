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

    foreach(test_name test_movie test_room test_users test_ticket_payment test_storage test_booking test_storage_extended)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE cinema_core)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_schedule.cpp")
        add_executable(test_schedule tests/test_schedule.cpp)
        target_link_libraries(test_schedule PRIVATE cinema_core)
        add_test(NAME test_schedule COMMAND test_schedule)
    endif()

    add_test(NAME console_flow
        COMMAND ${CMAKE_COMMAND}
            "-DAPP=$<TARGET_FILE:cinema_booking>"
            "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/tests/console_input.txt"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/console_flow.cmake")

    add_test(NAME persistence_flow
        COMMAND ${CMAKE_COMMAND}
            "-DAPP=$<TARGET_FILE:cinema_booking>"
            "-DDATA_DIR=${CMAKE_CURRENT_BINARY_DIR}/persistence_data"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/persistence_flow.cmake")
endif()
