SET(testevents_SRCS
    testevents.cpp
    testevents.h
    ../common/cmdlineargs.h
    ../common/cmdlineargs.cpp
)

SET(testevents_qtobject_SRCS
    testevents.h
)

QT4_WRAP_CPP(testevents_moc_SRCS ${testevents_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-testevents
    ${testevents_moc_SRCS}
    ${testevents_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-testevents precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-testevents
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-alsa
)

# we should install some files...
INSTALL(TARGETS drumstick-testevents
        RUNTIME DESTINATION bin)
