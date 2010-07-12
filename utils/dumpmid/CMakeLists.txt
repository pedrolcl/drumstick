SET(dumpmid_SRCS
    dumpmid.cpp
    dumpmid.h
)

SET(dumpmid_qtobject_SRCS
    dumpmid.h
)

QT4_WRAP_CPP(dumpmid_moc_SRCS ${dumpmid_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpmid
    ${dumpmid_moc_SRCS}
    ${dumpmid_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-dumpmid
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpmid 
        RUNTIME DESTINATION bin)
