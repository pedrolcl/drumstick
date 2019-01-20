VERSION = 1.1.3
DEFINES += VERSION=$$VERSION
VER_MAJ = 1
VER_MIN = 1
VER_PAT = 3

defineReplace(drumstickLib) {
    LIBRARY_NAME = $$1
    win32: LIBRARY_NAME = $$LIBRARY_NAME$$VER_MAJ
    return($$LIBRARY_NAME)
}
