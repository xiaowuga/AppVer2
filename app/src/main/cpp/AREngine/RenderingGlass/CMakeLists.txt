
set(CMAKE_CXX_STANDARD 17)

set(INC_PATH include)
set(SRC_PATH src)

set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

file(GLOB SOURCES "${SRC_PATH}/*.cpp")



if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(RenderingGlass SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(RenderingGlass_LIB NAMES RenderingGlass PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(RenderingGlass_LIB)
        add_library(RenderingGlass SHARED IMPORTED)
        set_target_properties(RenderingGlass PROPERTIES IMPORTED_LOCATION ${ARInput_LIB})
    else()
        message(FATAL_ERROR "Prebuilt library RenderingGlass not found")
    endif()
endif()


target_include_directories(RenderingGlass PUBLIC ${INC_PATH})

target_link_libraries(RenderingGlass PUBLIC  ${GlobalLibs} Basic ARInput)

set(RenderingGlass_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(RenderingGlass PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()