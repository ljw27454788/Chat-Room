QT += widgets

HEADERS     = dialog.h \
    talk-client.h

SOURCES     = dialog.cpp \
              main.cpp



# install

target.path = $$[QT_INSTALL_EXAMPLES]/widgets/layouts/basiclayouts

INSTALLS += target

