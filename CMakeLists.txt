cmake_minimum_required(VERSION 3.16)
project(Control_GUI)
cmake_policy(SET CMP0135 NEW)

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
endif()

find_package(Qt6 COMPONENTS Core Widgets PrintSupport Network REQUIRED)

if(${Qt6_VERSION} VERSION_LESS 6.3)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
else()
  qt_standard_project_setup()
endif()


# QtMqtt

include(FetchContent)

set(QTMQTT_PREFIX ${CMAKE_BINARY_DIR}/thirdparty/qtmqtt)
set(QTMQTT_INSTALL_DIR ${QTMQTT_PREFIX}/install)
FetchContent_Declare(
    QtMqttSrc
    GIT_REPOSITORY https://github.com/qt/qtmqtt.git
    GIT_TAG v${Qt6_VERSION}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/thirdparty/qtmqtt
)
FetchContent_MakeAvailable(QtMqttSrc)

# QCustomPlot

FetchContent_Declare(
    QCustomPlotSrc
    URL https://www.qcustomplot.com/release/2.1.1/QCustomPlot-source.tar.gz
    URL_HASH MD5=dec87d6ad14745683ebe166b5936bab1
    SOURCE_DIR ${CMAKE_BINARY_DIR}/thirdparty/qcustomplot
)
FetchContent_MakeAvailable(QCustomPlotSrc)

qt_add_library(QtCustomPlot STATIC ${qcustomplotsrc_SOURCE_DIR}/qcustomplot.cpp)
target_link_libraries(QtCustomPlot PRIVATE Qt6::Widgets Qt6::PrintSupport)

# QCGaugeWidget

FetchContent_Declare(
    QCGaugeWidgetSrc
    GIT_REPOSITORY https://github.com/ska/Qt-custom-gauge-widget.git
    GIT_TAG b3ff33df11995ae8b8b4e8b57883bf87fa652cc5
    SOURCE_DIR ${CMAKE_BINARY_DIR}/thirdparty/qcgaugewidget
)
FetchContent_MakeAvailable(QCGaugeWidgetSrc)

qt_add_library(QCGaugeWidget STATIC ${qcgaugewidgetsrc_SOURCE_DIR}/source/qcgaugewidget.cpp)
target_link_libraries(QCGaugeWidget PRIVATE Qt6::Widgets)

# PicoJSON
include(ExternalProject)

set(JSON_ROOT_DIR ${CMAKE_BINARY_DIR}/thirdparty/picojson)
set(JSON_INCLUDE_DIRS ${JSON_ROOT_DIR}/src/picojson)
ExternalProject_Add(picojson
    GIT_REPOSITORY https://github.com/kazuho/picojson
    GIT_TAG v1.3.0
    UPDATE_DISCONNECTED ON
    PREFIX ${JSON_ROOT_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(CMAKE_AUTOUIC ON)

qt_add_executable(${PROJECT_NAME}
    gui.cpp gui.h gui.ui
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Mqtt
)
#target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::Mqtt)

target_link_libraries(${PROJECT_NAME} PRIVATE QtCustomPlot QCGaugeWidget)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Mqtt_INCLUDE_DIRS}
    ${qcustomplotsrc_SOURCE_DIR}
    ${qcgaugewidgetsrc_SOURCE_DIR}/source
    ${JSON_INCLUDE_DIRS}
)

if (WIN32)
    install(TARGETS ${PROJECT_NAME} 
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${PROJECT_NAME})

    # workaround to make windeployqt find Qt6Mqtt.dll
    install(TARGETS Mqtt RUNTIME DESTINATION "${Qt6_DIR}/../../../bin")

    qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    install(SCRIPT ${deploy_script} COMPONENT ${PROJECT_NAME})
endif()

set(CPACK_COMPONENTS_ALL ${PROJECT_NAME})
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION 1.0.0)
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")

include(CPack)