QT       += core gui widgets network printsupport mqtt

CONFIG   += c++17
TARGET = Control_GUI
TEMPLATE = app

# Application source files
SOURCES += \
    main.cpp \
    gui.cpp
HEADERS += \
    gui.h
FORMS += \
    gui.ui

# QCustomPlot
QCUSTOMPLOT_DIR = $$PWD/thirdparty/qcustomplot
INCLUDEPATH += $$QCUSTOMPLOT_DIR
SOURCES += $$QCUSTOMPLOT_DIR/qcustomplot.cpp
HEADERS += $$QCUSTOMPLOT_DIR/qcustomplot.h

# QCGaugeWidget
QCGAUGEWIDGET_DIR = $$PWD/thirdparty/qcgaugewidget/source
INCLUDEPATH += $$QCGAUGEWIDGET_DIR
SOURCES += $$QCGAUGEWIDGET_DIR/qcgaugewidget.cpp
HEADERS += $$QCGAUGEWIDGET_DIR/qcgaugewidget.h

# PicoJSON (header-only)
PICOJSON_DIR = $$PWD/thirdparty/picojson
INCLUDEPATH += $$PICOJSON_DIR

# Resources file for icons
RESOURCES += \
    icons/icons.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
