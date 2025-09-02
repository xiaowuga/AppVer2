set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)
set(SOURCES
    ${SRC_PATH}/Rendering.cpp
)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)


set(CMAKE_CUDA_ARCHITECTURES 89)
enable_language(CUDA)
find_package(CUDAToolkit REQUIRED)
find_library(CUDA_DRIVER_LIBRARY cuda)
# find_package(CUDA REQUIRED)
# set(CMAKE_CUDA_ARCHITECTURES 100)
find_package(vsg 1.1.0)
set(CMAKE_CXX_STANDARD 17)

if(NOT USE_PREBUILT_LIBS)
    # 编译并创建库
    set(LIB_TYPE "PUBLIC")
    add_library(Rendering SHARED ${SOURCES})
else()
    # 不编译, 导入moduleX/lib目录下的的.so文件
    set(LIB_TYPE "INTERFACE")
    find_library(Rendering_LIB NAMES Rendering PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(Rendering_LIB)
        message(STATUS "Found Rendering prebuilt lib in ${SHARED_LIB}")
        add_library(Rendering SHARED IMPORTED GLOBAL)
        set_target_properties(Rendering PROPERTIES IMPORTED_LOCATION ${Rendering_LIB})
    else()
        message(FATAL_ERROR "Prebuilt library Rendering not found in ${SHARED_LIB}")
    endif()
endif()

# 根据需要添加asset路径下的子目录
add_subdirectory(${ASSET_PATH})

if(TARGET Rendering)
    message(STATUS "Rendering [internal] exists")
else()
    message(STATUS "Rendering [internal] not exists")
endif()

target_include_directories(Rendering ${LIB_TYPE} 
    ${INC_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/AREngine/
    ${OpenCV_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/Video_Codec_SDK_12.0.16/Lib/linux/stubs/x86_64
)

target_link_libraries(Rendering ${LIB_TYPE} 
    vsg::vsg 
    flatbuffers
    vsgRenderer
    libnvcuvid.so libnvidia-encode.so 
    # ${CUDA_CUDA_LIBRARY}
    CUDA::toolkit
    ${CUDA_DRIVER_LIBRARY}
    )

get_target_property(Rendering_LINK_dirs Rendering LINK_DIRECTORIES)
set(Rendering_LIB_PATH ${SHARED_LIB} ${Rendering_LINK_dirs} PARENT_SCOPE)


if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(Rendering PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()

# get_target_property(Rendering_include_dirs Rendering INTERFACE_INCLUDE_DIRECTORIES)
# get_target_property(Rendering_LINK_dirs Rendering LINK_DIRECTORIES)
# get_target_property(Rendering_interface_LINK_dirs Rendering INTERFACE_LINK_DIRECTORIES)

# message(STATUS "Rendering include directories:${Rendering_include_dirs}")
# message(STATUS "Rendering link directories:${Rendering_LINK_dirs}")
# message(STATUS "Rendering interface link directories:${Rendering_interface_LINK_dirs}")
