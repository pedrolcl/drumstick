set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")

set( HEADERS
    synthcontroller.h
    synthrenderer.h
    filewrapper.h
)

set( SOURCES
    synthcontroller.cpp 
    synthrenderer.cpp
    filewrapper.cpp
)

if (QT_VERSION VERSION_LESS 5.15.0)
    qt5_wrap_cpp( MOC_SRCS ${HEADERS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
else()
    qt_wrap_cpp( MOC_SRCS ${HEADERS}
        OPTIONS -I ${Drumstick_SOURCE_DIR}/library/include)
endif()

if(STATIC_DRUMSTICK)
    add_library( drumstick-rt-eassynth STATIC ${MOC_SRCS} ${SOURCES} ${HEADERS})
    target_compile_definitions(drumstick-rt-eassynth
        PRIVATE QT_STATICPLUGIN)
    set_target_properties(drumstick-rt-eassynth PROPERTIES
         STATIC_LIB "libdrumstick-rt-eassynth")
else()
    add_library( drumstick-rt-eassynth MODULE ${MOC_SRCS} ${SOURCES} ${HEADERS})
    target_compile_definitions(drumstick-rt-eassynth
        PRIVATE QT_PLUGIN)
endif()

target_link_libraries( drumstick-rt-eassynth PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    PkgConfig::PULSE
    sonivox::sonivox
    Drumstick::RT
)

target_include_directories(drumstick-rt-eassynth PRIVATE
    ${Drumstick_SOURCE_DIR}/library/include
)

set_target_properties(drumstick-rt-eassynth PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/${DRUMSTICK_PLUGINS_DIR})

install( TARGETS drumstick-rt-eassynth
    EXPORT drumstick-rt-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${DRUMSTICK_PLUGINS_DIR} )
