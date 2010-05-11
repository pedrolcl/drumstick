SET(drumgrid_forms_SRCS
    drumgrid.ui
)

SET(drumgrid_SRCS
    drumgrid.cpp
    drumgrid.h
    drumgridmodel.cpp
    drumgridmodel.h
    main.cpp
)

SET(drumgrid_qtobject_SRCS
    drumgridmodel.h
    drumgrid.h
)

QT4_WRAP_UI(drumgrid_ui_SRCS ${drumgrid_forms_SRCS})

QT4_WRAP_CPP(drumgrid_moc_SRCS ${drumgrid_qtobject_SRCS})

ADD_EXECUTABLE(drumstick-drumgrid
    ${drumgrid_ui_SRCS}
    ${drumgrid_moc_SRCS}
    ${drumgrid_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(drumstick-drumgrid precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(drumstick-drumgrid
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-alsa
)

INSTALL(TARGETS drumstick-drumgrid 
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-drumgrid.desktop
        DESTINATION share/applications)
        