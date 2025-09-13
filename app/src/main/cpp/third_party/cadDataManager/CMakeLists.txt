cmake_minimum_required(VERSION 3.22.1)

project("cadDataManager")



#add_library(${CMAKE_PROJECT_NAME} SHARED
#        native-lib.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(cadDataManager SHARED IMPORTED)

set_target_properties(cadDataManager PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libcadDataManager.so
)
#target_link_libraries(${CMAKE_PROJECT_NAME}
#        cadDataManager
#        android
#        log)