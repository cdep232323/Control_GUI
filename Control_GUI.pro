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

CONFIG_SOURCE = $$PWD/config.json
CONFIG_DEST = $$OUT_PWD/config.json
CONFIG_DEST_2 = $$OUT_PWD/config.json

MQTT_DLL_SOURCE = C:/Qt/6.8.2/msvc2022_64/bin/Qt6Mqtt.dll
MQTT_DLL_DEST = $$OUT_PWD/Qt6Mqtt.dll

# For release builds
CONFIG(release, debug|release) {
    CONFIG_DEST = $$OUT_PWD/config.json
    CONFIG_DEST_2 = $$OUT_PWD/release/config.json
    MQTT_DLL_DEST = $$OUT_PWD/release/Qt6Mqtt.dll
}

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

copyconfig.commands = $$QMAKE_COPY_FILE \"$$replace(CONFIG_SOURCE, /, $$QMAKE_DIR_SEP)\" \"$$replace(CONFIG_DEST, /, $$QMAKE_DIR_SEP)\"
copyconfig2.commands = $$QMAKE_COPY_FILE \"$$replace(CONFIG_SOURCE, /, $$QMAKE_DIR_SEP)\" \"$$replace(CONFIG_DEST_2, /, $$QMAKE_DIR_SEP)\"
copymqttdll.commands = $$QMAKE_COPY_FILE \"$$replace(MQTT_DLL_SOURCE, /, $$QMAKE_DIR_SEP)\" \"$$replace(MQTT_DLL_DEST, /, $$QMAKE_DIR_SEP)\"

# Make sure these commands run as part of the build
first.depends = $(first) copyconfig copyconfig2 copymqttdll
export(first.depends)
export(copyconfig.commands)
export(copyconfig2.commands)
export(copymqttdll.commands)
QMAKE_EXTRA_TARGETS += first copyconfig copyconfig2 copymqttdll

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
