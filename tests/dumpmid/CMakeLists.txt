SET(dumpmid_SRCS
    dumpmid.cpp
    dumpmid.h
)

SET(dumpmid_qtobject_SRCS
    dumpmid.h
)

QT4_WRAP_CPP(dumpmid_moc_SRCS ${dumpmid_qtobject_SRCS})

ADD_EXECUTABLE(aseqmm-dumpmid
    ${dumpmid_moc_SRCS}
    ${dumpmid_SRCS}
)

IF(USE_PCH)
    ADD_DEPENDENCIES(aseqmm-dumpmid precompiled_headers)
ENDIF(USE_PCH)

TARGET_LINK_LIBRARIES(aseqmm-dumpmid
    ${QT_LIBRARIES} 
    ${ALSA_LIBS} 
    aseqmm
)

# we should install some files...
INSTALL(TARGETS aseqmm-dumpmid 
        RUNTIME DESTINATION bin)
