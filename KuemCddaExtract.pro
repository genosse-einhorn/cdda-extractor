#-------------------------------------------------
#
# Project created by QtCreator 2017-04-01T11:12:27
#
#-------------------------------------------------

QT       += core gui concurrent widgets svg

TARGET = KuemCddaExtract
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += flac

QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wconversion -Wshadow

# paranoia code contains lots of unused parameters
QMAKE_CFLAGS += -Wno-unused

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
    musicbrainz/releasefinder.cpp \
    paranoia/gap.c \
    paranoia/isort.c \
    paranoia/overlap.c \
    paranoia/p_block.c \
    paranoia/paranoia.c \
    encoder/flacencoder.cpp

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
    musicbrainz/releasefinder.h \
    paranoia/gap.h \
    paranoia/isort.h \
    paranoia/overlap.h \
    paranoia/p_block.h \
    paranoia/paranoia.h \
    paranoia/cdda_interface.h \
    encoder/flacencoder.h

FORMS    += mainwindow.ui \
    extendederrordialog.ui \
    extractparametersdialog.ui \
    progressdialog.ui

RESOURCES += \
    data/data.qrc

DISTFILES += \
    paranoia/README
