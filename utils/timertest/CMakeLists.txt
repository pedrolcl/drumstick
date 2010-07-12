SET(timertest_SRCS
    timertest.cpp
    timertest.h
)

SET(timertest_qtobject_SRCS
    timertest.h
)

QT4_WRAP_CPP(timertest_moc_SRCS ${timertest_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-timertest
    ${timertest_SRCS}
    ${timertest_moc_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-timertest
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
)

# we should install some files...
INSTALL(TARGETS drumstick-timertest 
        RUNTIME DESTINATION bin)
