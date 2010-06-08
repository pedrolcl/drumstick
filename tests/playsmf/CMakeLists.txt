SET(playsmf_SRCS
    playsmf.cpp
    playsmf.h
    ../common/cmdlineargs.cpp
    ../common/cmdlineargs.h
)

SET(playsmf_qtobject_SRCS
    playsmf.h
)

QT4_WRAP_CPP(playsmf_moc_SRCS ${playsmf_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-playsmf
    ${playsmf_moc_SRCS}
    ${playsmf_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-playsmf precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-playsmf
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-alsa
    drumstick-file
)

# we should install some files...
INSTALL(TARGETS drumstick-playsmf
        RUNTIME DESTINATION bin)
