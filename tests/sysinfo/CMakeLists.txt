SET(sysinfo_SRCS
    sysinfo.cpp
)

ADD_EXECUTABLE(aseqmm-sysinfo
    ${sysinfo_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-sysinfo precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-sysinfo
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

INSTALL(TARGETS aseqmm-sysinfo 
        RUNTIME DESTINATION bin)
