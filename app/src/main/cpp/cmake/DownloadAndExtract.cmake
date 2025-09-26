function(download_and_extract URL OUT_DIR)
    # 保证输出目录存在
    file(MAKE_DIRECTORY ${OUT_DIR})
    set(ARCHIVE "${OUT_DIR}/archive.tmp")

    if(EXISTS "${ARCHIVE}")
        message(STATUS "Archive already exists: ${ARCHIVE}")
    else()
        message(STATUS "Downloading: ${URL}")

        if(UNIX)
            # Linux / macOS
            execute_process(
                    COMMAND curl -L -o "${ARCHIVE}" "${URL}"
                    RESULT_VARIABLE result
            )
        elseif(WIN32)
            # Windows 用 PowerShell
            execute_process(
                    COMMAND powershell -Command "Invoke-WebRequest '${URL}' -OutFile '${ARCHIVE}'"
                    RESULT_VARIABLE result
            )
        else()
            message(FATAL_ERROR "Unsupported platform for download_and_extract")
        endif()

        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Download failed: ${URL}")
        endif()
    endif()

    # 解压
    message(STATUS "Extracting to: ${OUT_DIR}")
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xvf "${ARCHIVE}"
            WORKING_DIRECTORY ${OUT_DIR}
            RESULT_VARIABLE extract_result
    )

    if(NOT extract_result EQUAL 0)
        message(FATAL_ERROR "Extraction failed: ${ARCHIVE}")
    endif()

    file(REMOVE "${ARCHIVE}")
    message(STATUS "Removed archive: ${ARCHIVE}")
endfunction()
