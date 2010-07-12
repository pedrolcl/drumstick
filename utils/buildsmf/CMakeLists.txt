SET(buildsmf_SRCS
    buildsmf.cpp
    buildsmf.h
)

SET(buildsmf_qtobject_SRCS
    buildsmf.h
)

QT4_WRAP_CPP(buildsmf_moc_SRCS ${buildsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-buildsmf 
    ${buildsmf_moc_SRCS}
    ${buildsmf_SRCS}
)

TARGET_LINK_LIBRARIES(drumstick-buildsmf
    ${QT_LIBRARIES} 
    drumstick-common
    drumstick-file
)

INSTALL(TARGETS drumstick-buildsmf 
        RUNTIME DESTINATION bin)
