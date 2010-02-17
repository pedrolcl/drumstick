SET(sysinfo_SRCS
    sysinfo.cpp
)

ADD_EXECUTABLE(drumstick-sysinfo
    ${sysinfo_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-sysinfo precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-sysinfo
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-alsa
)

INSTALL(TARGETS drumstick-sysinfo 
        RUNTIME DESTINATION bin)
