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

# Try to find Qt MQTT module through standard Qt mechanisms
find_package(Qt6 COMPONENTS Mqtt QUIET)

# If we don't have Qt6Mqtt, try to find it in specific locations
if(NOT Qt6Mqtt_FOUND AND NOT BUILD_MQTT_LOCALLY)
  # Print the search paths to help with debugging
  message(STATUS "Qt6Mqtt not found via find_package. CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
  
  # Try to find the library manually in system paths
  find_library(MQTT_LIBRARY 
    NAMES Qt6Mqtt qt6mqtt QtMqtt qtmqtt libQt6Mqtt
    PATHS 
      ${CMAKE_PREFIX_PATH}/lib
      ${CMAKE_PREFIX_PATH}/lib/x86_64-linux-gnu
      /usr/lib/x86_64-linux-gnu 
      /usr/local/lib 
      /usr/lib 
      /usr/lib/qt6
  )
  
  find_path(MQTT_INCLUDE_DIR
    NAMES QtMqtt/qmqttclient.h
    PATHS
      ${CMAKE_PREFIX_PATH}/include
      /usr/include/qt6
      /usr/include/x86_64-linux-gnu/qt6
  )
  
  # If we found the library but it's in a non-standard location, create imported target
  if(MQTT_LIBRARY)
    message(STATUS "Found Qt MQTT library: ${MQTT_LIBRARY}")
    message(STATUS "Found Qt MQTT include directory: ${MQTT_INCLUDE_DIR}")
    
    # Create an imported target for MQTT
    add_library(Qt6Mqtt SHARED IMPORTED)
    set_target_properties(Qt6Mqtt PROPERTIES
      IMPORTED_LOCATION "${MQTT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MQTT_INCLUDE_DIR}"
    )
    
    # Create an alias to match the expected name
    add_library(Qt6::Mqtt ALIAS Qt6Mqtt)
  else()
    # If we couldn't find it externally and we're not building locally, error out
    if(NOT BUILD_MQTT_LOCALLY)
      message(FATAL_ERROR "Qt MQTT module not found and BUILD_MQTT_LOCALLY is disabled. Please install the Qt MQTT module or enable BUILD_MQTT_LOCALLY.")
    endif()
  endif()
endif()

# Build MQTT from source if needed and allowed
if(NOT Qt6Mqtt_FOUND AND NOT TARGET Qt6::Mqtt AND BUILD_MQTT_LOCALLY)
  message(STATUS "Building Qt MQTT from source...")
  
  # Define our MQTT library by directly including the source
  set(MQTT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src/mqtt")

  # Check if the MQTT source directory exists
  if(NOT EXISTS "${MQTT_SRC_DIR}")
    message(STATUS "MQTT source not found at ${MQTT_SRC_DIR}. Trying to clone from repository...")
    
    # Clone the repository if it doesn't exist
    find_package(Git REQUIRED)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} clone https://code.qt.io/qt/qtmqtt.git ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt
      RESULT_VARIABLE GIT_RESULT
    )
    
    # Check out the correct branch
    if(GIT_RESULT EQUAL 0)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt checkout 6.5
      )
      message(STATUS "Successfully cloned Qt MQTT repository")
    else()
      message(FATAL_ERROR "Failed to clone Qt MQTT repository")
    endif()
  endif()

  # Define MQTT source files
  file(GLOB MQTT_SOURCES "${MQTT_SRC_DIR}/*.cpp")
  file(GLOB MQTT_HEADERS "${MQTT_SRC_DIR}/*.h")
  
  # Create MQTT library
  add_library(Qt6Mqtt STATIC ${MQTT_SOURCES} ${MQTT_HEADERS})
  
  # Set include directories
  target_include_directories(Qt6Mqtt PUBLIC ${MQTT_SRC_DIR})
  target_include_directories(Qt6Mqtt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src)
  
  # Link against Qt
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

# Additional include for MQTT when building locally
if(BUILD_MQTT_LOCALLY)
  target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src
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