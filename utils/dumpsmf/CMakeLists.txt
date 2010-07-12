SET(dumpsmf_SRCS
    dumpsmf.cpp
    dumpsmf.h
)

SET(dumpsmf_qtobject_SRCS
    dumpsmf.h
)

QT4_WRAP_CPP(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpsmf 
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-dumpsmf
    ${QT_LIBRARIES} 
    drumstick-common
    drumstick-file
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpsmf 
        RUNTIME DESTINATION bin)
