set(INC_PATH include)
set(SRC_PATH src)
set(LIB_PATH lib)

set(SOURCES
    ${SRC_PATH}/RenderUpload.cpp
)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(RenderUpload SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(RenderUpload_LIB NAMES RenderUpload PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(RenderUpload_LIB)
        add_library(RenderUpload SHARED IMPORTED)
        set_target_properties(RenderUpload PROPERTIES IMPORTED_LOCATION ${RenderUpload_LIB})

    else()
        message(FATAL_ERROR "Prebuilt library RenderUpload not found")
    endif()
endif()

target_include_directories(RenderUpload ${LIB_TYPE}
    ${INC_PATH}
)


target_link_libraries(RenderUpload ${LIB_TYPE} ${GlobalLibs} Basic ARInput)

set(RenderUpload_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(RenderUpload PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
