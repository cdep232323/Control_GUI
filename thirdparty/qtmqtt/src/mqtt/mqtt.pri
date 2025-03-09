QT += core network

HEADERS += \
    $$PWD/qmqttclient.h \
    $$PWD/qmqttglobal.h \
    $$PWD/qmqttmessage.h \
    $$PWD/qmqttsubscription.h \
    $$PWD/qmqtttopicfilter.h \
    $$PWD/qmqtttopicname.h \
    $$PWD/qmqttconnection_p.h \
    $$PWD/qmqttcontrolpacket_p.h

SOURCES += \
    $$PWD/qmqttclient.cpp \
    $$PWD/qmqttconnection.cpp \
    $$PWD/qmqttcontrolpacket.cpp \
    $$PWD/qmqttmessage.cpp \
    $$PWD/qmqttsubscription.cpp \
    $$PWD/qmqtttopicfilter.cpp \
    $$PWD/qmqtttopicname.cpp

INCLUDEPATH += $$PWD