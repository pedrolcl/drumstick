SET(dumpsmf_SRCS
    dumpsmf.cpp
    dumpsmf.h
)

SET(dumpsmf_qtobject_SRCS
    dumpsmf.h
)

QT4_WRAP_CPP(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-dumpsmf 
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-dumpsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-dumpsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS aseqmm-dumpsmf 
        RUNTIME DESTINATION bin)
