set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(drumstick-rt-dummy-out_QTOBJ_SRCS
    ../../include/rtmidioutput.h
    dummyoutput.h
)

set(drumstick-rt-dummy-out_SRCS
    dummyoutput.cpp
)

add_definitions(-DQT_PLUGIN)

qt5_wrap_cpp(drumstick-rt-dummy-out_MOC_SRCS ${drumstick-rt-dummy-out_QTOBJ_SRCS})

add_library(drumstick-rt-dummy-out MODULE
    ${drumstick-rt-dummy-out_MOC_SRCS}
    ${drumstick-rt-dummy-out_SRCS}
)

set_target_properties(drumstick-rt-dummy-out PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/backends)


install(TARGETS drumstick-rt-dummy-out
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        LIBRARY DESTINATION lib${LIB_SUFFIX})
