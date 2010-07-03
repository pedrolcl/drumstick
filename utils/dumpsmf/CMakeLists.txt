SET(dumpsmf_SRCS
    dumpsmf.cpp
    dumpsmf.h
    ../common/cmdlineargs.cpp
    ../common/cmdlineargs.h
)

SET(dumpsmf_qtobject_SRCS
    dumpsmf.h
)

QT4_WRAP_CPP(dumpsmf_moc_SRCS ${dumpsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpsmf 
    ${dumpsmf_moc_SRCS}
    ${dumpsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-dumpsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-dumpsmf
    ${QT_LIBRARIES} 
    drumstick-file
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpsmf 
        RUNTIME DESTINATION bin)