set(INC_PATH include)
set(SRC_PATH src)
set(LIB_PATH lib)
set(ASSET_PATH ../Basic/asset)

file(GLOB_RECURSE SOURCES ${SRC_PATH}/*.cpp)
file(GLOB_RECURSE INCLUDE ${INC_PATH}/*.h)
 

set(SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(NOT USE_PREBUILT_LIBS)
    # create Shared library
    set(LIB_TYPE PUBLIC)
    add_library(ObjectTracking2 SHARED ${SOURCES} ${INCLUDE})
else()
    set(LIB_TYPE INTERFACE)
    find_library(ObjectTracking2_LIB NAMES ObjectTracking2 PATHS ${SHARED_LIB} NO_DEFAULT_PATH)
    if(ObjectTracking2_LIB)
        add_library(ObjectTracking2 SHARED IMPORTED)
        set_target_properties(ObjectTracking2 PROPERTIES IMPORTED_LOCATION ${ObjectTracking2_LIB})
    else()
        message(FATAL_ERROR "Prebuilt library ObjectTracking2 not found")
    endif()
endif()

#add_subdirectory(${ASSET_PATH}/cvf)


# include
target_include_directories(ObjectTracking2 ${LIB_TYPE} 
                    ${CMAKE_CURRENT_SOURCE_DIR} 
                    ${INC_PATH} 
                    ${ASSET_PATH}/cvf
                    ${ASSET_PATH}/socketpp/include
                    )

target_link_libraries(ObjectTracking2 ${LIB_TYPE} GlobalLibs CVF)

set(ObjectTracking2_LIB_PATH ${SHARED_LIB} PARENT_SCOPE)

if(NOT USE_PREBUILT_LIBS)
    # set library output
    set(LIBRARY_OUTPUT_PATH ${SHARED_LIB})
    file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
    set_target_properties(ObjectTracking2 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif()
