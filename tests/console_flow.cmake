execute_process(
    COMMAND "${APP}"
    INPUT_FILE "${INPUT}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    RESULT_VARIABLE result)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "Console exited with ${result}: ${error}")
endif()

foreach(expected
        "Danh sach phim"
        "Danh sach suat chieu"
        "A1: Trong"
        "Da tao ve #1 - Pending"
        "Dat ve that bai"
        "Da tao ve #2 - Paid"
        "Ve da Paid"
        "Da huy ve"
        "Tam biet")
    string(FIND "${output}" "${expected}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Console output missing: ${expected}\n${output}")
    endif()
endforeach()
