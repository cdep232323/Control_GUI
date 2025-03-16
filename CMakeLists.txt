cmake_minimum_required(VERSION 3.16)
project(Control_GUI)

# Basic configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets Network PrintSupport REQUIRED)

# Options for MQTT building
option(BUILD_MQTT_LOCALLY "Build Qt MQTT module locally" ON)
option(QT_ALLOW_PRIVATE_INCLUDES "Allow including Qt private headers" OFF)

# Try to find Qt MQTT module through standard Qt mechanisms
find_package(Qt6 COMPONENTS Mqtt QUIET)

# Build MQTT from source if needed and allowed
if(NOT Qt6Mqtt_FOUND AND BUILD_MQTT_LOCALLY)
  message(STATUS "Building Qt MQTT from source...")
  # Define our MQTT library by directly including the source
  set(MQTT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src/mqtt")

  # Check if the MQTT source directory exists
  if(NOT EXISTS "${MQTT_SRC_DIR}")
      message(FATAL_ERROR "MQTT source not found at ${MQTT_SRC_DIR}. Please download MQTT source code to this location.")
  endif()

  # Find Qt private headers if enabled
  if(QT_ALLOW_PRIVATE_INCLUDES)
    # Try to find private headers in different potential locations
    set(QT_PRIVATE_INCLUDES
      # Qt installed from installer
      ${CMAKE_PREFIX_PATH}/include/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}
      ${CMAKE_PREFIX_PATH}/include/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}/QtCore
      # Ubuntu package locations
      /usr/include/x86_64-linux-gnu/qt6/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}
      /usr/include/x86_64-linux-gnu/qt6/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}/QtCore
      # General system locations
      /usr/include/qt6/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}
      /usr/include/qt6/QtCore/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH}/QtCore
    )
    
    # Check if we can find the private headers
    foreach(INCLUDE_DIR ${QT_PRIVATE_INCLUDES})
      if(EXISTS "${INCLUDE_DIR}/private/qobject_p.h")
        message(STATUS "Found Qt private headers at: ${INCLUDE_DIR}")
        set(QT_PRIVATE_HEADERS_FOUND TRUE)
        break()
      endif()
    endforeach()
    
    if(NOT QT_PRIVATE_HEADERS_FOUND)
      message(WARNING "Qt private headers not found. MQTT build may fail. Tried: ${QT_PRIVATE_INCLUDES}")
    endif()
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
  
  # Add qmqtttype.cpp if it exists
  if(EXISTS "${MQTT_SRC_DIR}/qmqtttype.cpp")
      list(APPEND MQTT_SOURCES "${MQTT_SRC_DIR}/qmqtttype.cpp")
  endif()

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
  
  # Add qmqtttype.h if it exists
  if(EXISTS "${MQTT_SRC_DIR}/qmqtttype.h")
      list(APPEND MQTT_HEADERS "${MQTT_SRC_DIR}/qmqtttype.h")
  endif()

  # Create MQTT library
  add_library(Qt6Mqtt STATIC ${MQTT_SOURCES} ${MQTT_HEADERS})
  
  # Set include directories
  target_include_directories(Qt6Mqtt PUBLIC ${MQTT_SRC_DIR})
  target_include_directories(Qt6Mqtt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src)
  
  # Add Qt private headers if allowed
  if(QT_ALLOW_PRIVATE_INCLUDES)
    target_include_directories(Qt6Mqtt PUBLIC ${QT_PRIVATE_INCLUDES})
  endif()
  
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

# If we still don't have MQTT and building locally is disabled, print an error
if(NOT Qt6Mqtt_FOUND AND NOT BUILD_MQTT_LOCALLY)
  message(FATAL_ERROR "Qt MQTT module not found and BUILD_MQTT_LOCALLY is disabled. Please install the Qt MQTT module.")
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

# Additional include for MQTT when building locally
if(BUILD_MQTT_LOCALLY)
  target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src
    ${CMAKE_BINARY_DIR}/include  # Add binary directory include path
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