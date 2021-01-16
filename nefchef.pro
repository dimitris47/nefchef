QT += core gui
QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = nefchef
TEMPLATE = app
VERSION = 1.0
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    adaptor.cpp \
    collectioneditorwidget.cpp \
    combo.cpp \
    droplist.cpp \
    helpdialog.cpp \
    ingredient.cpp \
    ingredients.cpp \
    ingredientwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    masscalculatorwidget.cpp \
    startpage.cpp

HEADERS += \
    adaptor.h \
    collectioneditorwidget.h \
    collectionpage.h \
    combo.h \
    droplist.h \
    global.h \
    helpdialog.h \
    ingredient.h \
    ingredients.h \
    ingredientwidget.h \
    mainwindow.h \
    masscalculatorwidget.h \
    masslineedit.h \
    startpage.h

FORMS += \
    adaptor.ui \
    combo.ui \
    droplist.ui \
    helpdialog.ui \
    ingredientwidget.ui \
    mainwindow.ui \
    masscalculatorwidget.ui \
    startpage.ui

RESOURCES += \
    nefchef.qrc

OTHER_FILES += \
    combined.cal \
    instructions.txt

isEmpty(PREFIX) {
    PREFIX=/usr/local
}

DISTFILES += \
    LICENSE \
    combined.cal \
    nefchef.desktop

unix:!android {
    target.path = $${PREFIX}/bin
    documentation.path = $${PREFIX}/share/doc/$${TARGET}
    documentation.files = instructions.txt
    icon.path = /usr/share/pixmaps
    icon.files = icons/nefchef.png
    desktop.path = /usr/share/applications
    desktop.files = nefchef.desktop
    license.path = /usr/share/nefchef
    license.files = LICENSE
    INSTALLS += target documentation icon desktop license
}

win32 {
    VERSION = $${VERSION}.0.0
    QMAKE_TARGET_COMPANY = DP Software
    QMAKE_TARGET_DESCRIPTION = A cookbook creator with embedded calories calculator
    QMAKE_TARGET_COPYRIGHT = \\251 2020 Dimitris Psathas
    QMAKE_TARGET_PRODUCT = Recipe Calories
    RC_ICONS = icons/nefchef.ico
    RC_LANG = 0x408
    RC_CODEPAGE = 1253
}
