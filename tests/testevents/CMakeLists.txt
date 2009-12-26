SET(testevents_SRCS
    testevents.cpp
    testevents.h
)

SET(testevents_qtobject_SRCS
    testevents.h
)

QT4_WRAP_CPP(testevents_moc_SRCS ${testevents_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-testevents
    ${testevents_moc_SRCS}
    ${testevents_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-testevents precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-testevents
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS aseqmm-testevents
        RUNTIME DESTINATION bin)
