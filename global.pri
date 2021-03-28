VERSION = 2.1.1
DEFINES += VERSION=$$VERSION
VER_MAJ = 2
VER_MIN = 1
VER_PAT = 1

defineReplace(drumstickLib) {
    LIBRARY_NAME = $$1
    !static:win32: LIBRARY_NAME = $$LIBRARY_NAME$$VER_MAJ
    return($$LIBRARY_NAME)
}

win32-msvc {
    QMAKE_CXXFLAGS += /utf-8 #/source-charset:utf-8 /execution-charset:utf-8
}
