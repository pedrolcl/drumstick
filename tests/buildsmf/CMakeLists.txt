SET(buildsmf_SRCS
    buildsmf.cpp
    buildsmf.h
)

SET(buildsmf_qtobject_SRCS
    buildsmf.h
)

QT4_WRAP_CPP(buildsmf_moc_SRCS ${buildsmf_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-buildsmf 
    ${buildsmf_moc_SRCS}
    ${buildsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-buildsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-buildsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

INSTALL(TARGETS aseqmm-buildsmf 
        RUNTIME DESTINATION bin)
