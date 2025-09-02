set(INC_PATH include)
set(SRC_PATH src)
set(ASSET_PATH asset)
set(LIB_PATH lib)

set(SOURCES
    ${SRC_PATH}/ObjectTracking.cpp
    ${SRC_PATH}/RbotTracking.cpp
    ${SRC_PATH}/ObjectTrackingServer.cpp

)
set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # 创建库
    set(LIB_TYPE PUBLIC)
    add_library(ObjectTracking SHARED ${SOURCES})
else()
    set(LIB_TYPE INTERFACE)
    find_library(ObjectTracking_LIB NAMES ObjectTracking PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(ObjectTracking_LIB)
        add_library(ObjectTracking SHARED IMPORTED)
        set_target_properties(ObjectTracking PROPERTIES IMPORTED_LOCATION ${ObjectTracking_LIB})
        
    else()
        message(FATAL_ERROR "Prebuilt library ObjectTracking not found")
    endif()
endif()

# add thrid-party lib
add_subdirectory(${ASSET_PATH}/RBOT)

target_include_directories(ObjectTracking ${LIB_TYPE} 
    ${ASSET_PATH}
    ${INC_PATH}
    ${PROJECT_SOURCE_DIR}/AREngine/
)

target_link_libraries(ObjectTracking ${LIB_TYPE} RBOT_LIB ${GlobalLibs} Basic)

set(ObjectTracking_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(ObjectTracking PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
