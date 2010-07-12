SET(drumgrid_forms_SRCS
    drumgridabout.ui
    drumgrid.ui
)

SET(drumgrid_SRCS
    drumgridabout.cpp
    drumgridabout.h
    drumgrid.cpp
    drumgrid.h
    drumgridmodel.cpp
    drumgridmodel.h
    drumgridmain.cpp
)

SET(drumgrid_qtobject_SRCS
    drumgridabout.h
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

TARGET_LINK_LIBRARIES(drumstick-drumgrid
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    drumstick-common
    drumstick-alsa
)

INSTALL(TARGETS drumstick-drumgrid 
        RUNTIME DESTINATION bin)

INSTALL(FILES drumstick-drumgrid.desktop
        DESTINATION share/applications)
        