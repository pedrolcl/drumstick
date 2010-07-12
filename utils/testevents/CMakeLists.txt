SET(testevents_SRCS
    testevents.cpp
    testevents.h
)

SET(testevents_qtobject_SRCS
    testevents.h
)

QT4_WRAP_CPP(testevents_moc_SRCS ${testevents_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-testevents
    ${testevents_moc_SRCS}
    ${testevents_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-testevents
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
)

# we should install some files...
INSTALL(TARGETS drumstick-testevents
        RUNTIME DESTINATION bin)
