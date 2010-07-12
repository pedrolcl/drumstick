SET(dumpwrk_SRCS
    dumpwrk.cpp
    dumpwrk.h
)

SET(dumpwrk_qtobject_SRCS
    dumpwrk.h
)

QT4_WRAP_CPP(dumpwrk_moc_SRCS ${dumpwrk_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpwrk 
    ${dumpwrk_moc_SRCS}
    ${dumpwrk_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-dumpwrk
    ${QT_LIBRARIES} 
    drumstick-common
    drumstick-file
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpwrk 
        RUNTIME DESTINATION bin)
