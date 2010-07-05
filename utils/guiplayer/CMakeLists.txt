SET(guiplayer_forms_SRCS
    guiplayer.ui
    playerabout.ui
)

SET(guiplayer_SRCS
    guiplayer.cpp
    guiplayer.h
    player.cpp
    player.h
    song.cpp
    song.h
    playerabout.cpp
    playerabout.h
    playermain.cpp
)

SET(guiplayer_qtobject_SRCS
    player.h
    playerabout.h
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
    drumstick-common
    drumstick-file
    drumstick-alsa
)

INSTALL(TARGETS drumstick-guiplayer
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-guiplayer.desktop
        DESTINATION share/applications)
