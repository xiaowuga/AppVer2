set(CMAKE_CXX_STANDARD 17)
set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

#find_package(OpenCV REQUIRED)

# ====================================
# T05 PoseEstimation Client
# ====================================
#if(ENABLE_T05_PoseEstimation_Client OR ENABLE_Interaction)
#
#    if(NOT USE_PREBUILT_LIBS)
#        add_library(PoseEstimation ${SRC_PATH}/PoseEstimation.cpp)
#        set_target_properties(PoseEstimation PROPERTIES
#            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib
#        )
#        set(INCLUDE_VISIBILITY PUBLIC)  # <-- 定义可见性
#    else()
#        find_library(PoseEstimation_LIB NAMES PoseEstimation PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
#        if(NOT PoseEstimation_LIB)
#            message(FATAL_ERROR "Prebuilt library PoseEstimation not found")
#        endif()
#        add_library(PoseEstimation IMPORTED INTERFACE)
#        set_target_properties(PoseEstimation PROPERTIES IMPORTED_LOCATION ${PoseEstimation_LIB})
#        set(INCLUDE_VISIBILITY INTERFACE)  # <-- 设置为接口库可见性
#    endif()
#
#    target_include_directories(PoseEstimation ${INCLUDE_VISIBILITY}
#        ${INC_PATH}
#        ${PROJECT_SOURCE_DIR}/AREngine/
#        ${OpenCV_INCLUDE_DIRS}
#    )
#
#    target_link_libraries(PoseEstimation ${INCLUDE_VISIBILITY} ${OpenCV_LIBS} ${GlobalLibs} Basic)
#
#    set(PoseEstimation_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)
#
#endif()


# ====================================
# T05 PoseEstimation Server
# ====================================
#if(ENABLE_T05_PoseEstimation_Server OR ENABLE_Interaction)
#    find_package(CURL REQUIRED)
#    include_directories(${CURL_INCLUDE_DIRS})
#
#    set(ONNXRUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/asset/onnxruntime-linux-x64-gpu-1.15.1")
#
#    add_library(PoseEstimationServer ${SRC_PATH}/PoseEstimationServer.cpp)
#
#    target_include_directories(PoseEstimationServer PRIVATE
#        ${INC_PATH}
#        ${PROJECT_SOURCE_DIR}/AREngine/
#        ${ONNXRUNTIME_DIR}/include
#        ../Basic/asset/cvf
#        ../Basic/include
#    )
#
#    target_link_directories(PoseEstimationServer PRIVATE ${ONNXRUNTIME_DIR}/lib)
#
#    target_link_libraries(PoseEstimationServer PRIVATE
#        ${GlobalLibs}
#        Basic
#        ${ONNXRUNTIME_DIR}/lib/libonnxruntime.so
#        ${CURL_LIBRARIES}
#    )
#
#
#endif()

# ====================================
# T05 PoseEstimation Fetch
# ====================================
#if(ENABLE_T05_PoseEstimation_Fetch)
add_library(PoseEstimationFetch ${SRC_PATH}/PoseEstimationFetch.cpp)

target_include_directories(PoseEstimationFetch PUBLIC
    ${INC_PATH}
    ${PROJECT_SOURCE_DIR}/AREngine/
)

target_link_libraries(PoseEstimationFetch PUBLIC
    ${GlobalLibs}
    Basic
)
#endif()