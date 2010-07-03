SET(dumpwrk_SRCS
    dumpwrk.cpp
    dumpwrk.h
    ../common/cmdlineargs.cpp
    ../common/cmdlineargs.h
)

SET(dumpwrk_qtobject_SRCS
    dumpwrk.h
)

QT4_WRAP_CPP(dumpwrk_moc_SRCS ${dumpwrk_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-dumpwrk 
    ${dumpwrk_moc_SRCS}
    ${dumpwrk_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-dumpwrk precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-dumpwrk
    ${QT_LIBRARIES} 
    drumstick-file
)

# we should install some files...
INSTALL(TARGETS drumstick-dumpwrk 
        RUNTIME DESTINATION bin)
