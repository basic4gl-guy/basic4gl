#-------------------------------------------------
#
# Project created by QtCreator 2016-03-05T16:54:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Basic4GL
TEMPLATE = app


SOURCES += main.cpp\
        basic4gleditor.cpp \
    sourcefileform.cpp \
    basic4gltextedit.cpp \
    plugindialog.cpp \
    createstandalonedialog.cpp \
    optionsdialog.cpp \
    aboutdialog.cpp \
    finddialog.cpp \
    argumentsdialog.cpp \
    virtualmachineviewdialog.cpp

HEADERS  += basic4gleditor.h \
    sourcefileform.h \
    LineNumberArea.h \
    plugindialog.h \
    createstandalonedialog.h \
    optionsdialog.h \
    aboutdialog.h \
    finddialog.h \
    argumentsdialog.h \
    virtualmachineviewdialog.h

FORMS    += basic4gleditor.ui \
    sourcefileform.ui \
    plugindialog.ui \
    createstandalonedialog.ui \
    optionsdialog.ui \
    aboutdialog.ui \
    finddialog.ui \
    argumentsdialog.ui \
    virtualmachineviewdialog.ui

RESOURCES += \
    ideresources.qrc
