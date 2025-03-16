QT       += core gui widgets network printsupport
CONFIG   += c++17
TARGET = Control_GUI
TEMPLATE = app

# Try to use the pre-built MQTT module if available
qtHaveModule(mqtt) {
    QT += mqtt
    message("Using pre-built Qt MQTT module")
} else {
    message("Pre-built Qt MQTT module not found, using local source files")

    # MQTT Integration from source
    QTMQTT_ROOT = $$PWD/thirdparty/qtmqtt
    MQTT_SRC = $$QTMQTT_ROOT/src/mqtt

    # Important: Add mqtt source directory to include path
    INCLUDEPATH += $$MQTT_SRC

    # Define that we're building the MQTT library
    DEFINES += QT_BUILD_MQTT_LIB

    # MQTT Source Files
    SOURCES += \
        $$MQTT_SRC/qmqttclient.cpp \
        $$MQTT_SRC/qmqttconnection.cpp \
        $$MQTT_SRC/qmqttcontrolpacket.cpp \
        $$MQTT_SRC/qmqttmessage.cpp \
        $$MQTT_SRC/qmqttsubscription.cpp \
        $$MQTT_SRC/qmqtttopicfilter.cpp \
        $$MQTT_SRC/qmqtttopicname.cpp

    # MQTT Header Files
    HEADERS += \
        $$MQTT_SRC/qmqttclient.h \
        $$MQTT_SRC/qmqttconnection_p.h \
        $$MQTT_SRC/qmqttcontrolpacket_p.h \
        $$MQTT_SRC/qmqttglobal.h \
        $$MQTT_SRC/qmqttmessage.h \
        $$MQTT_SRC/qmqttsubscription.h \
        $$MQTT_SRC/qmqtttopicfilter.h \
        $$MQTT_SRC/qmqtttopicname.h
}

# Application source files
SOURCES += \
    main.cpp \
    gui.cpp
HEADERS += \
    gui.h
FORMS += \
    gui.ui

# Configuration file copy
CONFIG_SOURCE = $$PWD/config.json
CONFIG_DEST = $$OUT_PWD/config.json

# MQTT DLL copy (only needed when using pre-built module)
qtHaveModule(mqtt) {
    win32 {
        MQTT_DLL_SOURCE = $$[QT_INSTALL_BINS]/Qt6Mqtt.dll
        MQTT_DLL_DEST = $$OUT_PWD/Qt6Mqtt.dll
    }
}

# For release builds
CONFIG(release, debug|release) {
    CONFIG_DEST = $$OUT_PWD/release/config.json
    qtHaveModule(mqtt):win32 {
        MQTT_DLL_DEST = $$OUT_PWD/release/Qt6Mqtt.dll
    }
} else {
    CONFIG(debug, debug|release) {
        CONFIG_DEST = $$OUT_PWD/debug/config.json
        qtHaveModule(mqtt):win32 {
            MQTT_DLL_DEST = $$OUT_PWD/debug/Qt6Mqtt.dll
        }
    }
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

# Copy commands
copyconfig.commands = $$QMAKE_COPY_FILE \"$$replace(CONFIG_SOURCE, /, $$QMAKE_DIR_SEP)\" \"$$replace(CONFIG_DEST, /, $$QMAKE_DIR_SEP)\"

# Only create MQTT copy command if using pre-built module
qtHaveModule(mqtt):win32 {
    copymqttdll.commands = $$QMAKE_COPY_FILE \"$$replace(MQTT_DLL_SOURCE, /, $$QMAKE_DIR_SEP)\" \"$$replace(MQTT_DLL_DEST, /, $$QMAKE_DIR_SEP)\"
    first.depends = $(first) copyconfig copymqttdll
    export(copymqttdll.commands)
} else {
    first.depends = $(first) copyconfig
}

export(first.depends)
export(copyconfig.commands)
QMAKE_EXTRA_TARGETS += first copyconfig
qtHaveModule(mqtt):win32: QMAKE_EXTRA_TARGETS += copymqttdll

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
