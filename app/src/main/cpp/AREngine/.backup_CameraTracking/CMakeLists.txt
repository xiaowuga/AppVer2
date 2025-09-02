find_package(OpenCV 4.5 REQUIRED)
find_package(Eigen3 3.3 REQUIRED)
include_directories(
        ..
        ${OpenCV_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
)
message(STATUS "EIGEN3_INCLUDE_DIR: ${EIGEN3_INCLUDE_DIR}")
set(CMAKE_CXX_STANDARD 17)

set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)

set(SOURCES ${SRC_PATH}/CameraTracking.cpp)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # 编译并创建库
    set(LIB_TYPE PUBLIC)
    add_library(CameraTracking SHARED ${SOURCES})
    add_subdirectory(${ASSET_PATH})
else()
    # 不编译, 导入moduleX/lib目录下的的.so文件
    set(LIB_TYPE INTERFACE)
    find_library(CameraTracking_LIB NAMES CameraTracking PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(CameraTracking_LIB)
        add_library(CameraTracking SHARED IMPORTED)
        set_target_properties(CameraTracking PROPERTIES IMPORTED_LOCATION ${CameraTracking_LIB})

    else()
        message(FATAL_ERROR "Prebuilt library CameraTracking not found")
    endif()
endif()

include_directories(
        ${OpenCV_INCLUDE_DIRS})

link_directories(${OpenCV_LIBS})

target_include_directories(CameraTracking ${LIB_TYPE}
        ${ASSET_PATH}
        ${INC_PATH}
        ${PROJECT_SOURCE_DIR}/AREngine/
)
# find_package(Eigen3 3.3 REQUIRED)
target_link_libraries(CameraTracking ${LIB_TYPE}
        # ${CMAKE_CURRENT_SOURCE_DIR}/asset/librvgVIOSystem.so
        ${CMAKE_CURRENT_SOURCE_DIR}/asset/libRGBD_PLANE_VIO.so
        # GlobalLibs
        # Eigen3::Eigen
)

set(CameraTracking_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(CameraTracking PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()

# add_subdirectory(../Basic)
include_directories(
    ../Basic/asset/cvf
    ../Basic/include
)
add_library(CameraTrackingServer SHARED src/CameraTrackingServer.cpp)
set(RELOC_LIBS         
    ${OpenCV_LIBS}
    ${EIGEN3_LIBS}
    ${Pangolin_LIBRARIES}
    ${BOOST_SERIALIZATION}
)
target_include_directories(CameraTrackingServer ${LIB_TYPE}
        ${ASSET_PATH}
        ${INC_PATH}
        ${PROJECT_SOURCE_DIR}/AREngine/
)
target_link_libraries(CameraTrackingServer 
        ${CMAKE_CURRENT_SOURCE_DIR}/asset/libRelocalization.so
        ${RELOC_LIBS}
        # Basic
)