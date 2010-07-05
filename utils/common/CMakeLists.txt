SET ( common_SRCS
    cmdlineargs.cpp
    cmdlineargs.h
)

ADD_LIBRARY( drumstick-common STATIC
    ${common_SRCS}
)

IF(NOT WIN32)
    SET_TARGET_PROPERTIES( drumstick-common 
        PROPERTIES COMPILE_FLAGS -fPIC
	)
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES( drumstick-common
    ${QT_LIBRARIES}
)
