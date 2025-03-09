#ifndef QTMQTTEXPORTS_H
#define QTMQTTEXPORTS_H

#include <QtCore/qglobal.h>

#if defined(QT_SHARED) || !defined(QT_STATIC)
#  if defined(QT_BUILD_MQTT_LIB)
#    define Q_MQTT_EXPORT Q_DECL_EXPORT
#  else
#    define Q_MQTT_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_MQTT_EXPORT
#endif

#endif // QTMQTTEXPORTS_H