set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)

set(SOURCES 
        ${SRC_PATH}/AlignModule.cpp
        ${SRC_PATH}/AlignTrajectory.cpp
        ${SRC_PATH}/AlignUtils.cpp
        ${SRC_PATH}/print.cpp  
    )
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # 编译并创建库
    set(LIB_TYPE PUBLIC)
    add_library(AlignModule SHARED ${SOURCES})
    # add_subdirectory(${ASSET_PATH})
else()
    # 不编译, 导入moduleX/lib目录下的的.so文件
    set(LIB_TYPE INTERFACE)
    find_library(AlignModule_LIB NAMES AlignModule PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(AlignModule_LIB)
        add_library(AlignModule SHARED IMPORTED)
        set_target_properties(AlignModule PROPERTIES IMPORTED_LOCATION ${AlignModule_LIB})

    else()
        message(FATAL_ERROR "Prebuilt library AlignModule not found")
    endif()
endif()

find_package(Pangolin REQUIRED)
# find_package(BOOST_SERIALIZATION boost_serialization)
find_package(Boost REQUIRED COMPONENTS serialization)


include_directories(${Pangolin_INCLUDE_DIRS})

include_directories(
        ${OpenCV_INCLUDE_DIRS})

link_directories(${OpenCV_LIBS})

target_include_directories(AlignModule ${LIB_TYPE}
        # ${ASSET_PATH}
        ${INC_PATH}
        ${INC_PATH}/utils/
        ${PROJECT_SOURCE_DIR}/AREngine/
)
# find_package(Eigen3 3.3 REQUIRED)
target_link_libraries(AlignModule ${LIB_TYPE}
        GlobalLibs
        Basic
        ${Pangolin_LIBRARIES}
        ${Boost_LIBRARIES}
)

set(AlignModule_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(AlignModule PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
