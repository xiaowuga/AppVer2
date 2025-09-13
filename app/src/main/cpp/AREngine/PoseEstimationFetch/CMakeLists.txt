#set(CMAKE_CXX_STANDARD 17)
#set(INC_PATH include)
#set(SRC_PATH src)
#set(ASSET_PATH asset)
#set(LIB_PATH lib)
#set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)
#
##find_package(OpenCV REQUIRED)
#
#
## ====================================
## T05 PoseEstimation Fetch
## ====================================
##if(ENABLE_T05_PoseEstimation_Fetch)
#add_library(PoseEstimationFetch ${SRC_PATH}/PoseEstimationFetch.cpp)
#
#target_include_directories(PoseEstimationFetch PUBLIC
#    ${INC_PATH}
#    ${PROJECT_SOURCE_DIR}/AREngine/
#)
#
#target_link_libraries(PoseEstimationFetch PUBLIC
#    ${GlobalLibs}
#    Basic
#)
#endif()



set(CMAKE_CXX_STANDARD 17)

set(INC_PATH include)
set(SRC_PATH src)

set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(SOURCES ${SRC_PATH}/PoseEstimationFetch.cpp)

if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(PoseEstimationFetch SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(PoseEstimationFetch_LIB NAMES PoseEstimationFetch PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(PoseEstimationFetch_LIB)
        add_library(PoseEstimationFetch SHARED IMPORTED)
#        set_target_properties(PoseEstimationFetch PROPERTIES IMPORTED_LOCATION ${ARInput_LIB})
    else()
        message(FATAL_ERROR "Prebuilt library PoseEstimationFetch not found")
    endif()
endif()


target_include_directories(PoseEstimationFetch PUBLIC ${INC_PATH})

target_link_libraries(PoseEstimationFetch PUBLIC  ${GlobalLibs}  Basic )

set(PoseEstimationFetch_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(PoseEstimationFetch PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
