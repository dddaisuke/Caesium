QT       += core gui network

TARGET = Caesium
TEMPLATE = app


HEADERS += \
    updater.h \
    startupupdater.h \
    qdroptreewidget.h \
    preferences.h \
    loadpreview.h \
    global.h \
    exit.h \
    compressionthread.h \
    caesium.h \
    about.h

SOURCES += \
    updater.cpp \
    startupupdater.cpp \
    qdroptreewidget.cpp \
    preferences.cpp \
    main.cpp \
    loadpreview.cpp \
    global.cpp \
    exit.cpp \
    compressionthread.cpp \
    caesium.cpp \
    about.cpp

OTHER_FILES += \
    icon.rc \
    icon.ico \
    cpf.ico \
    clf.ico \
    caesium.rc \
    caesium.ico \
    caesium_tw.ts \
    caesium_sv.ts \
    caesium_pt.ts \
    caesium_ja.ts \
    caesium_it.ts \
    caesium_fr.ts \
    caesium_en.ts \
    caesium_de.ts \
    caesium_cn.ts \
    icon.icns

FORMS += \
    updater.ui \
    preferences.ui \
    form.ui \
    exit.ui \
    caesium.ui \
    about.ui

RESOURCES += \
    icons.qrc

win32:RC_FILE = caesium.rc
macx:RC_FILE = icon.icns

TRANSLATIONS = caesium_en.ts \
                caesium_it.ts \
                caesium_ja.ts \
                caesium_fr.ts \
                caesium_pt.ts \
                caesium_de.ts \
                caesium_tw.ts \
                caesium_sv.ts \
                caesium_cn.ts
