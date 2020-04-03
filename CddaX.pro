#-------------------------------------------------
#
# Project created by QtCreator 2017-04-01T11:12:27
#
#-------------------------------------------------

QT       += core gui concurrent widgets svg network
win32: QT += winextras

TARGET = CddaX
TEMPLATE = app

CONFIG += link_pkgconfig c++11

!msvc:QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wconversion -Wshadow

msvc: QMAKE_CXXFLAGS += /utf-8 /wd4200
win32: LIBS += -luser32 -lshell32

# paranoia code contains lots of unused parameters and troublesome conversions
msvc {
    QMAKE_CFLAGS += /wd4244
} else {
    QMAKE_CFLAGS += -Wno-unused
}

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += main.cpp\
    encoderwriter.cpp \
    extractor.cpp \
    fileutil.cpp \
        mainwindow.cpp \
    libcdda/drive_handle.cpp \
    libcdda/toc.cpp \
    musicbrainz/releasefinder.cpp \
    musicbrainzaskdialog.cpp \
    ringbuffer.cpp \
    settingsdialog.cpp \
    tasklib/syncurldownload.cpp \
    tasklib/taskrunner.cpp \
    tracklistmodel.cpp \
    extractparametersdialog.cpp \
    libcdda/toc_finder.cpp \
    libcdda/sense.cpp \
    libcdda/os_util.cpp \
    encoder/abstractencoder.cpp \
    encoder/wavencoder.cpp \
    coverartwidget.cpp \
    paranoia/gap.c \
    paranoia/isort.c \
    paranoia/overlap.c \
    paranoia/p_block.c \
    paranoia/paranoia.c \
    paranoia/smallft.c \
    encoder/flacencoder.cpp \
    encoder/lameencoder.cpp \
    uiutil/futureprogressdialog.cpp \
    uiutil/progressdialog.cpp \
    uiutil/clickablebuddylabel.cpp \
    uiutil/extendederrordialog.cpp \
    uiutil/iconwidget.cpp \
    uiutil/win32iconloader.cpp

HEADERS  += mainwindow.h \
    encoderwriter.h \
    extractor.h \
    fileutil.h \
    libcdda/drive_handle.h \
    libcdda/toc.h \
    musicbrainz/releasefinder.h \
    musicbrainzaskdialog.h \
    ringbuffer.h \
    settingsdialog.h \
    tasklib/syncurldownload.h \
    tasklib/taskrunner.h \
    tracklistmodel.h \
    extractparametersdialog.h \
    libcdda/toc_finder.h \
    libcdda/sense.h \
    libcdda/os_util.h \
    encoder/abstractencoder.h \
    encoder/wavencoder.h \
    coverartwidget.h \
    paranoia/gap.h \
    paranoia/isort.h \
    paranoia/overlap.h \
    paranoia/p_block.h \
    paranoia/paranoia.h \
    paranoia/smallft.h \
    encoder/flacencoder.h \
    encoder/lameencoder.h \
    uiutil/clickablebuddylabel.h \
    uiutil/extendederrordialog.h \
    uiutil/futureprogressdialog.h \
    uiutil/iconwidget.h \
    uiutil/progressdialog.h \
    uiutil/win32iconloader.h

FORMS    += mainwindow.ui \
    extractparametersdialog.ui \
    musicbrainzaskdialog.ui \
    settingsdialog.ui \
    uiutil/extendederrordialog.ui \
    uiutil/progressdialog.ui

RESOURCES += \
    data/data.qrc

DISTFILES += \
    paranoia/README

# MP3
LIBS += -lmp3lame

# FLAC
packagesExist(flac) {
    PKGCONFIG += flac
} else {
    # link to it anyway
    LIBS += -lFLAC
}

RC_ICONS = data/logo.ico
