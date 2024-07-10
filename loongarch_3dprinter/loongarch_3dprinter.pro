QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    e_window.cpp \
    fan_window.cpp \
    fire_c_window.cpp \
    fire_e_window.cpp \
    main.cpp \
    mainwindow.cpp \
    x_window.cpp \
    y_window.cpp \
    z_window.cpp

HEADERS += \
    e_window.h \
    fan_window.h \
    fire_c_window.h \
    fire_e_window.h \
    mainwindow.h \
    x_window.h \
    y_window.h \
    z_window.h

FORMS += \
    e_window.ui \
    fan_window.ui \
    fire_c_window.ui \
    fire_e_window.ui \
    mainwindow.ui \
    x_window.ui \
    y_window.ui \
    z_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    picture.qrc
