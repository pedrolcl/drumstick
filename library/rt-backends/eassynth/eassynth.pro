TEMPLATE = subdirs
SUBDIRS += \
    sonivox \
    src
src.depends = sonivox
