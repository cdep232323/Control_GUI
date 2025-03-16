cmake_minimum_required(VERSION 3.16)
project(Control_GUI)

# Basic configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Option to control whether to build MQTT from source
option(BUILD_MQTT_LOCALLY "Build Qt MQTT module locally" OFF)

# Find required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets Network PrintSupport REQUIRED)

# Try to find pre-built MQTT module if not building locally
if(NOT BUILD_MQTT_LOCALLY)
  find_package(Qt6 COMPONENTS Mqtt QUIET)
endif()

# If MQTT wasn't found and we're not building locally, set the flag to build locally
if(NOT Qt6Mqtt_FOUND AND NOT BUILD_MQTT_LOCALLY)
  message(STATUS "Pre-built Qt6 MQTT module not found, will build from source")
  set(BUILD_MQTT_LOCALLY ON)
endif()

# Build MQTT from source if needed
if(BUILD_MQTT_LOCALLY)
  # Define our MQTT library by directly including the source
  set(MQTT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src/mqtt")

  # Check if the MQTT source directory exists
  if(NOT EXISTS "${MQTT_SRC_DIR}")
      message(FATAL_ERROR "MQTT source not found at ${MQTT_SRC_DIR}. Please download MQTT source code to this location.")
  endif()

  # Define MQTT source files
  set(MQTT_SOURCES
      ${MQTT_SRC_DIR}/qmqttclient.cpp
      ${MQTT_SRC_DIR}/qmqttconnection.cpp
      ${MQTT_SRC_DIR}/qmqttcontrolpacket.cpp
      ${MQTT_SRC_DIR}/qmqttmessage.cpp
      ${MQTT_SRC_DIR}/qmqttsubscription.cpp
      ${MQTT_SRC_DIR}/qmqtttopicfilter.cpp
      ${MQTT_SRC_DIR}/qmqtttopicname.cpp
  )

  # Define MQTT header files
  set(MQTT_HEADERS
      ${MQTT_SRC_DIR}/qmqttclient.h
      ${MQTT_SRC_DIR}/qmqttconnection_p.h
      ${MQTT_SRC_DIR}/qmqttcontrolpacket_p.h
      ${MQTT_SRC_DIR}/qmqttglobal.h
      ${MQTT_SRC_DIR}/qmqttmessage.h
      ${MQTT_SRC_DIR}/qmqttsubscription.h
      ${MQTT_SRC_DIR}/qmqtttopicfilter.h
      ${MQTT_SRC_DIR}/qmqtttopicname.h
  )

  # Create MQTT library
  add_library(Qt6Mqtt STATIC ${MQTT_SOURCES} ${MQTT_HEADERS})
  target_include_directories(Qt6Mqtt 
    PUBLIC 
      ${MQTT_SRC_DIR}
      ${CMAKE_PREFIX_PATH}/include/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}
      ${CMAKE_PREFIX_PATH}/include/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}/QtCore
  )
  target_link_libraries(Qt6Mqtt PUBLIC Qt6::Core Qt6::Network)
  target_compile_definitions(Qt6Mqtt PRIVATE QT_BUILD_MQTT_LIB)

  # Add an alias for the library to match the expected name
  add_library(Qt6::Mqtt ALIAS Qt6Mqtt)
  
  # Create the QtMqtt directory structure for includes
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/QtMqtt)
  
  # Copy all MQTT headers to the include directory
  file(GLOB MQTT_HEADERS "${MQTT_SRC_DIR}/*.h")
  foreach(HEADER ${MQTT_HEADERS})
    get_filename_component(FILENAME ${HEADER} NAME)
    configure_file(${HEADER} ${CMAKE_BINARY_DIR}/include/QtMqtt/${FILENAME} COPYONLY)
  endforeach()
endif()

# QCustomPlot
set(QCUSTOMPLOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qcustomplot")
if(NOT EXISTS "${QCUSTOMPLOT_DIR}/qcustomplot.cpp")
    message(FATAL_ERROR "QCustomPlot source not found. Please download QCustomPlot and extract to ${QCUSTOMPLOT_DIR}")
endif()

add_library(QtCustomPlot STATIC 
    "${QCUSTOMPLOT_DIR}/qcustomplot.cpp"
    "${QCUSTOMPLOT_DIR}/qcustomplot.h"
)
target_link_libraries(QtCustomPlot PRIVATE Qt6::Widgets Qt6::PrintSupport)

# QCGaugeWidget
set(QCGAUGEWIDGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qcgaugewidget/source")
if(NOT EXISTS "${QCGAUGEWIDGET_DIR}/qcgaugewidget.cpp")
    message(FATAL_ERROR "QCGaugeWidget source not found. Please download it to ${QCGAUGEWIDGET_DIR}")
endif()

add_library(QCGaugeWidget STATIC 
    "${QCGAUGEWIDGET_DIR}/qcgaugewidget.cpp"
    "${QCGAUGEWIDGET_DIR}/qcgaugewidget.h"
)
target_link_libraries(QCGaugeWidget PRIVATE Qt6::Widgets)

# PicoJSON
set(JSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/picojson")

# Define the resources
set(RESOURCE_FILES 
    icons/icons.qrc
)

# Create the main executable
qt_add_executable(${PROJECT_NAME}
    gui.cpp gui.h gui.ui
    main.cpp
    ${RESOURCE_FILES}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::PrintSupport
    Qt6::Mqtt
    QtCustomPlot
    QCGaugeWidget
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QCUSTOMPLOT_DIR}
    ${QCGAUGEWIDGET_DIR}
    ${JSON_INCLUDE_DIR}
)

# Add the include path for QtMqtt when building locally
if(BUILD_MQTT_LOCALLY)
  target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_BINARY_DIR}/include
  )
endif()

# Copy config.json to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json
    COMMENT "Copying config.json to build directory"
)

# Windows-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    
    # Installation rules
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION bin)
    
    # Deploy Qt files
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
    
    # Package settings
    set(CPACK_GENERATOR "WIX;ZIP")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    include(CPack)
endif()