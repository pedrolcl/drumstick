link_directories(
    ${PULSE_LIBRARY_DIRS}
)

set( HEADERS
    ../../../include/drumstick/rtmidioutput.h
    synthcontroller.h
    synthrenderer.h
)

set( SOURCES
    synthcontroller.cpp 
    synthrenderer.cpp
)

qt5_wrap_cpp( MOC_SRCS ${HEADERS}
    OPTIONS -I ${DRUMSTICK_SOURCE_DIR}/library/include
)

if(STATIC_DRUMSTICK)
    add_library( drumstick-rt-eassynth STATIC ${MOC_SRCS} ${SOURCES} )
    target_compile_definitions(drumstick-rt-eassynth
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-eassynth PROPERTIES
         STATIC_LIB "libdrumstick-rt-eassynth")
else()
    add_library( drumstick-rt-eassynth MODULE ${MOC_SRCS} ${SOURCES} )
    target_compile_definitions(drumstick-rt-eassynth
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries( drumstick-rt-eassynth
    sonivox 
    Qt5::Core 
    ${PULSE_LIBRARIES}
)

target_include_directories( drumstick-rt-eassynth PRIVATE
    ${PULSE_INCLUDE_DIRS}
    ${DRUMSTICK_SOURCE_DIR}/library/include
)

set_target_properties(drumstick-rt-eassynth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/drumstick)

install( TARGETS drumstick-rt-eassynth
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/drumstick)

