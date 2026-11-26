cmake_minimum_required(VERSION 3.16)
project(cinema_booking_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cinema_core
    src/movie.cpp src/movie_catalog.cpp src/movie_details.cpp
    src/movie_details_catalog.cpp src/cinema_v2.cpp src/seat.cpp src/cinema_room.cpp
    src/showtime.cpp src/schedule_manager.cpp src/user.cpp src/customer.cpp
    src/admin.cpp src/user_manager.cpp src/ticket.cpp src/payment.cpp
    src/demo_payment.cpp src/bank_transfer_payment.cpp src/file_storage.cpp
    src/cinema_data_store.cpp src/booking_manager.cpp src/cinema_system.cpp)
target_include_directories(cinema_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(cinema_booking src/main.cpp)
target_link_libraries(cinema_booking PRIVATE cinema_core)

option(BUILD_QT_GUI "Build the optional Qt 6 Widgets application" OFF)
if(BUILD_QT_GUI)
    find_package(Qt6 6.2 COMPONENTS Widgets QUIET)
    if(Qt6_FOUND)
        add_library(cinema_qt_ui
            qt/ui_session.cpp qt/catalog_pages.cpp qt/transaction_pages.cpp
            qt/storage_bar.cpp qt/main_window.cpp)
        target_include_directories(cinema_qt_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/qt)
        target_link_libraries(cinema_qt_ui PUBLIC cinema_core Qt6::Widgets)
        add_executable(cinema_booking_qt WIN32 qt/main.cpp)
        target_link_libraries(cinema_booking_qt PRIVATE cinema_qt_ui)

        add_library(oop_cinema_ui qt/v2_session.cpp qt/v2_widgets.cpp qt/v2_window.cpp
                  third_party/qrcodegen/qrcodegen.cpp)
        target_include_directories(oop_cinema_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/qt)
        target_include_directories(oop_cinema_ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/qrcodegen)
        target_link_libraries(oop_cinema_ui PUBLIC cinema_core Qt6::Widgets)
        add_executable(oop_cinema_qt WIN32 qt/v2_main.cpp)
        target_link_libraries(oop_cinema_qt PRIVATE oop_cinema_ui)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/v2/movie_details.txt
                  ${CMAKE_CURRENT_SOURCE_DIR}/data/v2/cinema_demo.txt
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/v2-data)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/qt/assets/
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets)
    else()
        message(WARNING "Qt 6 Widgets not found; building console and tests only")
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    if(TARGET cinema_booking_qt)
        add_executable(test_v2_session tests/test_v2_session.cpp)
        target_link_libraries(test_v2_session PRIVATE oop_cinema_ui)
        target_compile_definitions(test_v2_session PRIVATE
            MOVIE_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/movie_details.txt"
            CINEMA_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/cinema_demo.txt"
            LEGACY_MOVIE_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/v2-before-posters/movie_details.txt"
            LEGACY_CINEMA_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/v2-before-posters/cinema_demo.txt")
        add_test(NAME test_v2_session COMMAND test_v2_session)
        add_executable(test_v2_qt_navigation tests/test_v2_qt_navigation.cpp)
        target_link_libraries(test_v2_qt_navigation PRIVATE oop_cinema_ui)
        target_compile_definitions(test_v2_qt_navigation PRIVATE
            MOVIE_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/movie_details.txt"
            CINEMA_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/cinema_demo.txt"
            ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/qt/assets")
        add_test(NAME test_v2_qt_navigation COMMAND test_v2_qt_navigation)
        set_tests_properties(test_v2_qt_navigation PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        add_executable(test_v2_qt_autosave tests/test_v2_qt_autosave.cpp)
        target_link_libraries(test_v2_qt_autosave PRIVATE oop_cinema_ui)
        target_compile_definitions(test_v2_qt_autosave PRIVATE
            MOVIE_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/movie_details.txt"
            CINEMA_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/cinema_demo.txt"
            ASSET_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/qt/assets")
        add_test(NAME test_v2_qt_autosave COMMAND test_v2_qt_autosave)
        set_tests_properties(test_v2_qt_autosave PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        add_executable(test_qt_ui tests/test_qt_ui.cpp)
        target_link_libraries(test_qt_ui PRIVATE cinema_qt_ui)
        add_test(NAME test_qt_ui COMMAND test_qt_ui)
        set_tests_properties(test_qt_ui PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        add_executable(test_qt_persistence tests/test_qt_persistence.cpp)
        target_link_libraries(test_qt_persistence PRIVATE cinema_qt_ui)
        add_test(NAME qt_persistence_flow
            COMMAND ${CMAKE_COMMAND}
                "-DAPP=$<TARGET_FILE:test_qt_persistence>"
                "-DDATA_DIR=${CMAKE_CURRENT_BINARY_DIR}/qt_persistence_data"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/qt_persistence_flow.cmake")
        set_tests_properties(qt_persistence_flow PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
    add_executable(smoke_test tests/smoke_test.cpp)
    target_link_libraries(smoke_test PRIVATE cinema_core)
    add_test(NAME smoke_test COMMAND smoke_test)

    foreach(test_name test_movie test_room test_users test_ticket_payment test_storage test_booking test_storage_extended)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE cinema_core)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()

    add_executable(test_data_snapshot tests/test_data_snapshot.cpp)
    target_link_libraries(test_data_snapshot PRIVATE cinema_core)
    add_test(NAME test_data_snapshot COMMAND test_data_snapshot)

    add_executable(test_movie_details tests/test_movie_details.cpp)
    target_link_libraries(test_movie_details PRIVATE cinema_core)
    add_test(NAME test_movie_details COMMAND test_movie_details)

    add_executable(test_movie_catalog_data tests/test_movie_catalog_data.cpp)
    target_link_libraries(test_movie_catalog_data PRIVATE cinema_core)
    target_compile_definitions(test_movie_catalog_data PRIVATE
        MOVIE_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/movie_details.txt")
    add_test(NAME test_movie_catalog_data COMMAND test_movie_catalog_data)

    add_executable(test_cinema_v2 tests/test_cinema_v2.cpp)
    target_link_libraries(test_cinema_v2 PRIVATE cinema_core)
    target_compile_definitions(test_cinema_v2 PRIVATE
        MOVIE_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/movie_details.txt"
        CINEMA_DATA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/v2/cinema_demo.txt"
        LEGACY_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_test(NAME test_cinema_v2 COMMAND test_cinema_v2)

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
