set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)

set(SOURCES
    ${SRC_PATH}/InputRealSense.cpp
    ${SRC_PATH}/realsense.cpp
    ${SRC_PATH}/InputOffline.cpp

)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(InputRealSense SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(InputRealSense_LIB NAMES InputRealSense PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(InputRealSense_LIB)
        add_library(InputRealSense SHARED IMPORTED)
        set_target_properties(InputRealSense PROPERTIES IMPORTED_LOCATION ${InputRealSense_LIB})
        
    else()
        message(FATAL_ERROR "Prebuilt library InputRealSense not found")
    endif()
endif()

find_package(realsense2 REQUIRED)

target_include_directories(InputRealSense ${LIB_TYPE} 
    ${ASSET_PATH}
    ${INC_PATH}
    ${PROJECT_SOURCE_DIR}/AREngine/
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(InputRealSense ${LIB_TYPE} ${GlobalLibs} ${OpenCV_LIBS} realsense2::realsense2)

set(InputRealSense_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(InputRealSense PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
