QT       += core gui webenginewidgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DialogCreateProductPage.cpp \
    DialogVideoEditor.cpp \
    ResizableRect.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    DialogCreateProductPage.h \
    DialogVideoEditor.h \
    MainWindow.h \
    ResizableRect.h

FORMS += \
    DialogCreateProductPage.ui \
    DialogVideoEditor.ui \
    MainWindow.ui

include(model/model.pri)
include(../common/ffmpeg/ffmpeg.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
