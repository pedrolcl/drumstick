SET(dumpove_SRCS
    dumpove.cpp
    dumpove.h
)

SET(dumpove_qtobject_SRCS
    dumpove.h
)

QT4_WRAP_CPP(dumpove_moc_SRCS ${dumpove_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpove 
    ${dumpove_moc_SRCS}
    ${dumpove_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-dumpove
    ${QT_LIBRARIES} 
    drumstick-common
    drumstick-file
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpove 
        RUNTIME DESTINATION bin)
