include(CMakeFindDependencyMacro)
find_dependency(ALSA REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/drumstick-alsa-targets.cmake)
