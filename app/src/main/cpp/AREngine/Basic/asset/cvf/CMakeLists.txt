cmake_minimum_required(VERSION 3.10)

# 设置源文件和头文件
file(GLOB BFC_SOURCES "BFC/src.cpp")

add_library(CVF SHARED 
			${BFC_SOURCES}
			CVX/src.cpp
			CVRender/src.cpp
			BFCS/log.cpp

			)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 设置包含目录
target_include_directories(CVF PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/BFC ${CMAKE_CURRENT_SOURCE_DIR}/CVX)
target_link_libraries(CVF PUBLIC GlobalLibs c++_shared)