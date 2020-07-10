QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    delegates.cpp \
    exceptions.cpp \
    filterproxymodel.cpp \
    geodesicdome.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow_archive.cpp \
    mainwindow_merge.cpp \
    mainwindow_nodeeditor.cpp \
    mainwindow_search.cpp \
    mainwindow_settings.cpp \
    mainwindow_transform.cpp \
    nodeeditor.cpp \
    nodefilter.cpp \
    opentrackdialog.cpp \
    prefabitem.cpp \
    searchfilterlayout.cpp \
    trackarchive.cpp \
    velodataparser.cpp \
    velodb.cpp

HEADERS += \
    delegates.h \
    exceptions.h \
    filterproxymodel.h \
    geodesicdome.h \
    mainwindow.h \
    nodeeditor.h \
    nodefilter.h \
    opentrackdialog.h \
    prefabitem.h \
    searchfilterlayout.h \
    sqlite3.h \
    trackarchive.h \
    velodataparser.h \
    velodb.h

FORMS += \
    mainwindow.ui \
    opentrackdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lsqlite3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lsqlite3
else:unix: LIBS += -L$$PWD/./ -lsqlite3

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

DISTFILES += \
  sqlite3.dll

RESOURCES += \
  icons.qrc

RC_ICONS = VeloTrackEditToolbox.ico
