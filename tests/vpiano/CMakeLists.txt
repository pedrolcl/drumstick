SET(vpiano_forms_SRCS
    about.ui
    connections.ui
    preferences.ui
    vpiano.ui
)

QT4_WRAP_UI(vpiano_ui_SRCS ${vpiano_forms_SRCS})

SET(vpiano_SRCS
    pianokey.cpp
    pianokey.h
    pianoscene.cpp
    pianoscene.h
    pianokeybd.cpp
    pianokeybd.h
    keylabel.cpp
    keylabel.h
    rawkeybdapp.cpp
    rawkeybdapp.h
    about.cpp
    about.h
    connectionitem.h
    connections.cpp
    connections.h
    preferences.cpp
    preferences.h
    vpiano.cpp
    vpiano.h
    main.cpp
)

SET(vpiano_qtobject_SRCS
    pianoscene.h
    pianokeybd.h
    about.h
    connections.h
    preferences.h
    vpiano.h
)

QT4_WRAP_CPP(vpiano_moc_SRCS ${vpiano_qtobject_SRCS})

QT4_ADD_RESOURCES(vpiano_SRCS pianokeybd.qrc)

ADD_EXECUTABLE(drumstick-vpiano
    ${vpiano_ui_SRCS}
    ${vpiano_moc_SRCS}
    ${vpiano_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-vpiano precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-vpiano
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick
)

INSTALL(TARGETS drumstick-vpiano
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-vpiano.desktop
        DESTINATION share/applications)
