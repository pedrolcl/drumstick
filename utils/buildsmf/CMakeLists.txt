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

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-buildsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-buildsmf
    ${QT_LIBRARIES} 
    drumstick-common
    drumstick-file
)

INSTALL(TARGETS drumstick-buildsmf 
        RUNTIME DESTINATION bin)
