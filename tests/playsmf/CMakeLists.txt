SET(playsmf_SRCS
    playsmf.cpp
    playsmf.h
)

SET(playsmf_qtobject_SRCS
    playsmf.h
)

QT4_WRAP_CPP(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-playsmf
    ${playsmf_moc_SRCS}
    ${playsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-playsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-playsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS aseqmm-playsmf
        RUNTIME DESTINATION bin)
