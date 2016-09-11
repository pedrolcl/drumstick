link_directories(
    ${PULSE_LIBRARY_DIRS}
)

set( HEADERS
    ../../../include/rtmidioutput.h
    synthcontroller.h
    synthrenderer.h
)

set( SOURCES
    synthcontroller.cpp 
    synthrenderer.cpp
)

add_definitions(-DQT_PLUGIN)

qt5_wrap_cpp( MOC_SRCS ${HEADERS} )

if(STATIC_DRUMSTICK)
    add_definitions(-DQT_STATICPLUGIN)
    add_library( drumstick-rt-eassynth STATIC ${MOC_SRCS} ${SOURCES} )
    set_target_properties(drumstick-rt-eassynth PROPERTIES
         STATIC_LIB "libdrumstick-rt-eassynth")
else()
    add_library( drumstick-rt-eassynth MODULE ${MOC_SRCS} ${SOURCES} )
endif()

target_link_libraries( drumstick-rt-eassynth
    sonivox 
    Qt5::Core 
    ${PULSE_LIBRARIES}
)

target_include_directories( drumstick-rt-eassynth
    PRIVATE ${PULSE_INCLUDE_DIRS}
)

set_target_properties(drumstick-rt-eassynth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install( TARGETS drumstick-rt-eassynth
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib${LIB_SUFFIX}/drumstick
    LIBRARY DESTINATION lib${LIB_SUFFIX}/drumstick
)
