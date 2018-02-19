#-------------------------------------------------
#
# Project created by QtCreator 2017-04-01T11:12:27
#
#-------------------------------------------------

QT       += core gui concurrent widgets svg

TARGET = KuemCddaExtract
TEMPLATE = app

CONFIG += link_pkgconfig

QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wconversion -Wshadow

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

win32 {
    LIBS += -lwininet
} else {
    QT += network
}

SOURCES += main.cpp\
        mainwindow.cpp \
    libcdda/drive_handle.cpp \
    libcdda/toc.cpp \
    tracklistmodel.cpp \
    extendederrordialog.cpp \
    extractparametersdialog.cpp \
    clickablebuddylabel.cpp \
    libcdda/toc_finder.cpp \
    libcdda/sense.cpp \
    extractrunner.cpp \
    extractrunnerworker.cpp \
    libcdda/os_util.cpp \
    encoder/abstractencoder.cpp \
    encoder/wavencoder.cpp \
    progressdialog.cpp \
    coverartwidget.cpp \
    musicbrainz/urldownloader.cpp \
    musicbrainz/releasefinder.cpp

HEADERS  += mainwindow.h \
    libcdda/drive_handle.h \
    libcdda/toc.h \
    tracklistmodel.h \
    extendederrordialog.h \
    extractparametersdialog.h \
    clickablebuddylabel.h \
    libcdda/toc_finder.h \
    libcdda/sense.h \
    extractrunner.h \
    extractrunnerworker.h \
    libcdda/os_util.h \
    encoder/abstractencoder.h \
    encoder/wavencoder.h \
    progressdialog.h \
    coverartwidget.h \
    musicbrainz/urldownloader.h \
    musicbrainz/releasefinder.h

FORMS    += mainwindow.ui \
    extendederrordialog.ui \
    extractparametersdialog.ui \
    progressdialog.ui

RESOURCES += \
    data/data.qrc
