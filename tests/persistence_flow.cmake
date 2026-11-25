# Moi execute_process mo mot tien trinh ung dung moi.
file(MAKE_DIRECTORY "${DATA_DIR}")
set(input1 "${DATA_DIR}/session1.in")
file(WRITE "${input1}" "4\n2\nKhach Moi\n5\n2\n1\nA1\n2\n9\n5\n${DATA_DIR}/good\n0\n")
execute_process(COMMAND "${APP}" INPUT_FILE "${input1}"
    OUTPUT_VARIABLE output1 ERROR_VARIABLE error1 RESULT_VARIABLE result1)
if(NOT result1 EQUAL 0 OR NOT output1 MATCHES "Da tao ve #1 - Pending" OR
   NOT output1 MATCHES "Thao tac file thanh cong")
    message(FATAL_ERROR "Session 1 failed: ${result1} ${error1}\n${output1}")
endif()

foreach(name movies rooms showtimes customers tickets)
    if(NOT EXISTS "${DATA_DIR}/good/${name}.txt")
        message(FATAL_ERROR "Missing saved ${name}.txt")
    endif()
endforeach()
file(MAKE_DIRECTORY "${DATA_DIR}/bad")
foreach(name movies rooms showtimes customers tickets)
    configure_file("${DATA_DIR}/good/${name}.txt"
                   "${DATA_DIR}/bad/${name}.txt" COPYONLY)
endforeach()
# Hop le ve cu phap nhung tham chieu phim khong ton tai.
file(WRITE "${DATA_DIR}/bad/showtimes.txt"
    "1|999|1|2026-12-01 19:00|2026-12-01 20:30|50000\n")

set(input2 "${DATA_DIR}/session2.in")
file(WRITE "${input2}"
    "9\n6\n${DATA_DIR}/good\n6\n5\n2\n1\nA1\n1\n9\n6\n${DATA_DIR}/bad\n6\n8\n1\n1\n5\n2\n2\nA1\n1\n7\n1\n9\n5\n${DATA_DIR}/good\n0\n")
execute_process(COMMAND "${APP}" INPUT_FILE "${input2}"
    OUTPUT_VARIABLE output2 ERROR_VARIABLE error2 RESULT_VARIABLE result2)
if(NOT result2 EQUAL 0 OR NOT output2 MATCHES "Khach: 2.*Suat: 1.*Pending" OR
   NOT output2 MATCHES "Dat ve that bai" OR
   NOT output2 MATCHES "Suat chieu tham chieu phim/phong sai" OR
   NOT output2 MATCHES "Ve da Paid" OR
   NOT output2 MATCHES "Da tao ve #2 - Paid" OR
   NOT output2 MATCHES "Da huy ve")
    message(FATAL_ERROR "Session 2 failed: ${result2} ${error2}\n${output2}")
endif()

set(input3 "${DATA_DIR}/session3.in")
file(WRITE "${input3}" "9\n6\n${DATA_DIR}/good\n6\n3\n1\n0\n")
execute_process(COMMAND "${APP}" INPUT_FILE "${input3}"
    OUTPUT_VARIABLE output3 ERROR_VARIABLE error3 RESULT_VARIABLE result3)
if(NOT result3 EQUAL 0 OR NOT output3 MATCHES "Ve #1.*Cancelled" OR
   NOT output3 MATCHES "Ve #2.*Paid" OR NOT output3 MATCHES "A1: Trong")
    message(FATAL_ERROR "Session 3 failed: ${result3} ${error3}\n${output3}")
endif()
message(STATUS "Three separate console sessions persisted customers, seats and tickets")
