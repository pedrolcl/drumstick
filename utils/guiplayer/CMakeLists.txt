SET(playmid_forms_SRCS
    guiplayer.ui
)

SET(playmid_SRCS
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

SET(playmid_qtobject_SRCS
    player.h
    guiplayer.h
)

QT4_WRAP_UI(playmid_ui_SRCS ${playmid_forms_SRCS})

QT4_WRAP_CPP(playmid_moc_SRCS ${playmid_qtobject_SRCS})

QT4_ADD_RESOURCES(playmid_resources guiplayer.qrc)

ADD_EXECUTABLE(drumstick-guiplayer
    ${playmid_resources}
    ${playmid_ui_SRCS}
    ${playmid_moc_SRCS}
    ${playmid_SRCS}
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
