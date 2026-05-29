QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET   = alkotest
TEMPLATE = app

SOURCES += \
    main.cpp \
    logindialog.cpp \
    dialog.cpp

HEADERS += \
    logindialog.h \
    dialog.h

FORMS += \
    logindialog.ui \
    dialog.ui

RESOURCES += \
    resources.qrc

LIBS += -lwiringPi -lwiringPiDev

target.path = /home/pi/alkotest
INSTALLS += target
