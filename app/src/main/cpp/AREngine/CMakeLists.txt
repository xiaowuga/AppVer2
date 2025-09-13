add_library(ARENGINE_LIB INTERFACE)


if(ENABLE_AppVer2)
    add_subdirectory(Basic)
    add_subdirectory(ARInput)
    add_subdirectory(RelocationGlass)
    add_subdirectory(PoseEstimationFetch)
    add_subdirectory(PoseEstimationRokid)
    add_subdirectory(InteractiveUnderstanding)
endif ()

