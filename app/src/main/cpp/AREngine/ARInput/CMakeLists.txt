set(CMAKE_CXX_STANDARD 17)

set(INC_PATH include)
set(SRC_PATH src)

set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(SOURCES ${SRC_PATH}/ARInput.cpp)

if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(ARInput SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(ARInput_LIB NAMES ARInput PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(ARInput_LIB)
        add_library(ARInput SHARED IMPORTED)
        set_target_properties(ARInput PROPERTIES IMPORTED_LOCATION ${ARInput_LIB})
    else()
        message(FATAL_ERROR "Prebuilt library ARInput not found")
    endif()
endif()


target_include_directories(ARInput PUBLIC ${INC_PATH})

target_link_libraries(ARInput PUBLIC  ${GlobalLibs} Basic)

set(ARInput_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(ARInput PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
