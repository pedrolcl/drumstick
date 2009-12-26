SET(timertest_SRCS
    timertest.cpp
    timertest.h
)

SET(timertest_qtobject_SRCS
    timertest.h
)

QT4_WRAP_CPP(timertest_moc_SRCS ${timertest_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-timertest
    ${timertest_SRCS}
    ${timertest_moc_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-timertest precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-timertest
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS aseqmm-timertest 
        RUNTIME DESTINATION bin)
