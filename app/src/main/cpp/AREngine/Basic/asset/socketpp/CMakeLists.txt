cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 设置源文件和头文件
# file(GLOB INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}include/sockpp/*.h)
file(GLOB INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/sockpp/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(SOCKET SHARED ${INCLUDE} ${SOURCES})
# add_library(SOCKET SHARED ${SOURCES})

# 设置包含目录
target_include_directories(SOCKET PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/BFC ${CMAKE_CURRENT_SOURCE_DIR}/CVX)
target_link_libraries(SOCKET PUBLIC GlobalLibs)