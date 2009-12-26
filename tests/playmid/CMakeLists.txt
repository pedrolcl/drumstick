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

ADD_EXECUTABLE(aseqmm-smfplayer
    ${playmid_resources}
    ${playmid_ui_SRCS}
    ${playmid_moc_SRCS}
    ${playmid_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-smfplayer precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-smfplayer
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

INSTALL(TARGETS aseqmm-smfplayer
        RUNTIME DESTINATION bin)

INSTALL(FILES aseqmm-smfplayer.desktop
        DESTINATION share/applications)
