SET(playmid_forms_SRCS
    smfplayer.ui
)

SET(playmid_SRCS
    smfplayer.cpp
    smfplayer.h
    player.cpp
    player.h
    song.cpp
    song.h
    main.cpp
)

SET(playmid_qtobject_SRCS
    player.h
    smfplayer.h
)

QT4_WRAP_UI(playmid_ui_SRCS ${playmid_forms_SRCS})

QT4_WRAP_CPP(playmid_moc_SRCS ${playmid_qtobject_SRCS})

QT4_ADD_RESOURCES(playmid_resources smfplayer.qrc)

ADD_EXECUTABLE(drumstick-smfplayer
    ${playmid_resources}
    ${playmid_ui_SRCS}
    ${playmid_moc_SRCS}
    ${playmid_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-smfplayer precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-smfplayer
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick
)

INSTALL(TARGETS drumstick-smfplayer
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-smfplayer.desktop
        DESTINATION share/applications)
