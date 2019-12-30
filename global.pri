VERSION = 2.0.0
DEFINES += VERSION=$$VERSION
VER_MAJ = 2
VER_MIN = 0
VER_PAT = 0

defineReplace(drumstickLib) {
    LIBRARY_NAME = $$1
    win32: LIBRARY_NAME = $$LIBRARY_NAME$$VER_MAJ
    return($$LIBRARY_NAME)
}
