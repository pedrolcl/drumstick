foreach(component ${drumstick_FIND_COMPONENTS})
    string(TOLOWER ${component} comp)
    include(${CMAKE_CURRENT_LIST_DIR}/drumstick-${comp}-config.cmake)
endforeach()
