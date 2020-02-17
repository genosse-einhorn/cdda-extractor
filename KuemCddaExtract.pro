#-------------------------------------------------
#
# Project created by QtCreator 2017-04-01T11:12:27
#
#-------------------------------------------------

QT       += core gui concurrent widgets svg
win32: QT += winextras

TARGET = KuemCddaExtract
TEMPLATE = app

CONFIG += link_pkgconfig c++11

QMAKE_CXXFLAGS_DEBUG += -Wall -Wextra -Wconversion -Wshadow

msvc: QMAKE_CXXFLAGS += /utf-8 /wd4200
win32: LIBS += -luser32 -lshell32

# paranoia code contains lots of unused parameters and troublesome conversions
msvc {
    QMAKE_CFLAGS += /wd4244
} else {
    QMAKE_CFLAGS += -Wno-unused
}

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
    musicbrainz/releasefinder.cpp \
    musicbrainzaskdialog.cpp \
    tasklib/syncurldownload.cpp \
    tasklib/taskrunner.cpp \
    tracklistmodel.cpp \
    extractparametersdialog.cpp \
    libcdda/toc_finder.cpp \
    libcdda/sense.cpp \
    extractrunner.cpp \
    extractrunnerworker.cpp \
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
    libcdda/drive_handle.h \
    libcdda/toc.h \
    musicbrainz/releasefinder.h \
    musicbrainzaskdialog.h \
    tasklib/syncurldownload.h \
    tasklib/taskrunner.h \
    tracklistmodel.h \
    extractparametersdialog.h \
    libcdda/toc_finder.h \
    libcdda/sense.h \
    extractrunner.h \
    extractrunnerworker.h \
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
    encoder/lame_backend.h \
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
    progressdialog.ui \
    uiutil/extendederrordialog.ui

RESOURCES += \
    data/data.qrc

DISTFILES += \
    paranoia/README

# MP3
load(configure)
qtCompileTest(lame) {
    SOURCES += \
        encoder/lame_backend_linked.cpp
    LIBS += -lmp3lame
} else {
    SOURCES += \
        encoder/lame_backend_dll.cpp
}

# FLAC
packagesExist(flac) {
    PKGCONFIG += flac
} else {
    SOURCES_FLAC += \
        libFLAC/src/bitmath.c \
        libFLAC/src/bitreader.c \
        libFLAC/src/bitwriter.c \
        libFLAC/src/cpu.c \
        libFLAC/src/crc.c \
        libFLAC/src/fixed.c \
        libFLAC/src/fixed_intrin_sse2.c \
        libFLAC/src/fixed_intrin_ssse3.c \
        libFLAC/src/float.c \
        libFLAC/src/format.c \
        libFLAC/src/lpc.c \
        libFLAC/src/lpc_intrin_avx2.c \
        libFLAC/src/lpc_intrin_sse.c \
        libFLAC/src/lpc_intrin_sse2.c \
        libFLAC/src/lpc_intrin_sse41.c \
        libFLAC/src/md5.c \
        libFLAC/src/memory.c \
        libFLAC/src/metadata_iterators.c \
        libFLAC/src/metadata_object.c \
        libFLAC/src/stream_decoder.c \
        libFLAC/src/stream_encoder.c \
        libFLAC/src/stream_encoder_framing.c \
        libFLAC/src/stream_encoder_intrin_avx2.c \
        libFLAC/src/stream_encoder_intrin_sse2.c \
        libFLAC/src/stream_encoder_intrin_ssse3.c \
        libFLAC/src/window.c

    win32 {
        SOURCES_FLAC += \
            libFLAC/src/windows_unicode_filenames.c
    }

    FLAC_PACKAGE_VERSION = 1.3.2
    FLAC_CFLAGS = -DFLAC__HAS_OGG=0 -DFLAC__ALIGN_MALLOC_DATA -DHAVE_LROUND -DHAVE_STDINT_H -DHAVE_FSEEKO \
        -DFLAC__NO_DLL -DPACKAGE_VERSION=\\\"$${FLAC_PACKAGE_VERSION}\\\" \
        -I$${PWD}/libFLAC/src/include
    msvc {
        FLAC_CFLAGS += /wd4267 /wd4334 /wd4244 -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS
    } else {
        FLAC_CFLAGS += -Wno-implicit-fallthrough
    }

    flac.name = flac
    flac.input = SOURCES_FLAC
    flac.dependency_type = TYPE_C
    flac.variable_out = OBJECTS
    flac.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_IN_BASE}$${first(QMAKE_EXT_OBJ)}
    flac.commands = $${QMAKE_CC} $(CFLAGS) $${FLAC_CFLAGS} $(INCPATH) -c ${QMAKE_FILE_IN}
    msvc: flac.commands += -Fo${QMAKE_FILE_OUT}
    else: flac.commands += -o ${QMAKE_FILE_OUT}
    QMAKE_EXTRA_COMPILERS += flac

    INCLUDEPATH += libFLAC/include
    DEFINES += FLAC__NO_DLL

    DISTFILES +=  \
        libFLAC/COPYING.Xiph
}
