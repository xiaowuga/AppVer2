set(INC_PATH include)
set(SRC_PATH src)
set(LIB_PATH lib)
set(ASSET_PATH asset)

# set(SOURCES
#     ${SRC_PATH}/BasicData.cpp
#     ${SRC_PATH}/ConfigLoader.cpp
#     # ${SRC_PATH}/realsense.cpp
#     ${SRC_PATH}/RPC.cpp
#     ${SRC_PATH}/RPCServer.cpp
#     )
# set(INCLUDE
#     ${INC_PATH}/BasicData.h
#     ${INC_PATH}/ARModule.h
#     ${INC_PATH}/ConfigLoader.h
#     # ${INC_PATH}/realsense.h
#     )

file(GLOB_RECURSE SOURCES ${SRC_PATH}/*.cpp)
file(GLOB_RECURSE INCLUDE ${INC_PATH}/*.h)
 

set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)



if(NOT USE_PREBUILT_LIBS)
    # create Shared library
    set(LIB_TYPE PUBLIC)
    add_library(Basic SHARED ${SOURCES} ${INCLUDE})
else()
    set(LIB_TYPE INTERFACE)
    find_library(Basic_LIB NAMES Basic PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(Basic_LIB)
        add_library(Basic SHARED IMPORTED)
        set_target_properties(Basic PROPERTIES IMPORTED_LOCATION ${Basic_LIB})
    else()
        message(FATAL_ERROR "Prebuilt library Basic not found")
    endif()
endif()

add_subdirectory(${ASSET_PATH}/cvf)
add_subdirectory(${ASSET_PATH}/socketpp)

# include
target_include_directories(Basic ${LIB_TYPE} 
                    ${CMAKE_CURRENT_SOURCE_DIR} 
                    ${INC_PATH} 
                    ${ASSET_PATH}/cvf
                    ${ASSET_PATH}/socketpp/include
                    # ${EIGEN3_INCLUDE_DIR})
                    # /usr/local/include/eigen3\
                    )

# target_link_libraries(Basic ${LIB_TYPE} GlobalLibs CVF SOCKET)
target_link_libraries(Basic ${LIB_TYPE} GlobalLibs CVF SOCKET)

get_target_property(BASIC_INCLUDES Basic INCLUDE_DIRECTORIES)
message(STATUS "Basic include directories: ${BASIC_INCLUDES}")

set(Basic_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(Basic PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()