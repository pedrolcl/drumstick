SET(metronome_SRCS
    metronome.cpp
    metronome.h
    ../common/cmdlineargs.cpp
    ../common/cmdlineargs.h
)

SET(metronome_qtobject_SRCS
    metronome.h
)

QT4_WRAP_CPP(metronome_moc_SRCS ${metronome_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-metronome
    ${metronome_moc_SRCS}
    ${metronome_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-metronome precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-metronome
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-alsa
)

# we should install some files...
INSTALL(TARGETS drumstick-metronome
        RUNTIME DESTINATION bin)
