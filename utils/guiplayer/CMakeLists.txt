SET(guiplayer_forms_SRCS
    guiplayer.ui
)

SET(guiplayer_SRCS
    guiplayer.cpp
    guiplayer.h
    player.cpp
    player.h
    song.cpp
    song.h
    playermain.cpp
    ../common/cmdlineargs.h
    ../common/cmdlineargs.cpp
)

SET(guiplayer_qtobject_SRCS
    player.h
    guiplayer.h
)

QT4_WRAP_UI(guiplayer_ui_SRCS ${guiplayer_forms_SRCS})

QT4_WRAP_CPP(guiplayer_moc_SRCS ${guiplayer_qtobject_SRCS})

QT4_ADD_RESOURCES(guiplayer_resources guiplayer.qrc)

ADD_EXECUTABLE(drumstick-guiplayer
    ${guiplayer_resources}
    ${guiplayer_ui_SRCS}
    ${guiplayer_moc_SRCS}
    ${guiplayer_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-guiplayer precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-guiplayer
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-file
    drumstick-alsa
)

INSTALL(TARGETS drumstick-guiplayer
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-guiplayer.desktop
        DESTINATION share/applications)
