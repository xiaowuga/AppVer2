set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)

set(SOURCES
    ${SRC_PATH}/Detection.cpp
    ${SRC_PATH}/SegAndPoseEstimation.cpp
)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(Detection SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(Detection_LIB NAMES Detection PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(Detection_LIB)
        add_library(Detection SHARED IMPORTED)
        set_target_properties(Detection PROPERTIES IMPORTED_LOCATION ${Detection_LIB})
        
    else()
        message(FATAL_ERROR "Prebuilt library Detection not found")
    endif()
endif()

target_include_directories(Detection ${LIB_TYPE} 
    ${ASSET_PATH}
    ${INC_PATH}
    ${PROJECT_SOURCE_DIR}/AREngine/
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(Detection ${LIB_TYPE} ${GlobalLibs})

set(Detection_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(Detection PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
