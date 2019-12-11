if (CMAKE_VERSION VERSION_LESS 3.1.0)
    message(FATAL_ERROR "Drumstick requires at least CMake version 3.1.0")
endif()

foreach(component ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
    string(TOLOWER ${component} locomp)
    include(${CMAKE_CURRENT_LIST_DIR}/drumstick-${locomp}-config.cmake)
endforeach()
