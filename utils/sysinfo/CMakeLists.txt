SET(sysinfo_SRCS
    sysinfo.cpp
)

ADD_EXECUTABLE(drumstick-sysinfo
    ${sysinfo_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-sysinfo
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
)

INSTALL(TARGETS drumstick-sysinfo 
        RUNTIME DESTINATION bin)
