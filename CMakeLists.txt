cmake_minimum_required(VERSION 3.16)
project(Control_GUI)

# Basic configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
endif()

# Find required Qt components
find_package(Qt6 COMPONENTS Core Gui Widgets Network PrintSupport REQUIRED)

# Options for MQTT
option(BUILD_MQTT_LOCALLY "Build Qt MQTT module locally" ON)
option(QT_ALLOW_PRIVATE_INCLUDES "Allow including Qt private headers" OFF)

# First try to find Qt MQTT module through standard Qt mechanisms
find_package(Qt6 COMPONENTS Mqtt QUIET)

if(Qt6Mqtt_FOUND)
    message(STATUS "Found Qt6 MQTT package.")
    # We found MQTT via find_package, nothing more to do
elseif(NOT BUILD_MQTT_LOCALLY)
    # If not found via find_package and we're not building locally, 
    # try to find the library directly in system paths
    find_library(MQTT_LIBRARY 
        NAMES Qt6Mqtt qt6mqtt QtMqtt qtmqtt
        PATHS 
        /usr/lib/x86_64-linux-gnu 
        /usr/local/lib 
        /usr/lib 
        /usr/lib/qt6
        ${CMAKE_PREFIX_PATH}/lib
        NO_DEFAULT_PATH
    )
    
    find_path(MQTT_INCLUDE_DIR
        NAMES qmqttclient.h
        PATHS
        /usr/include/qt6/QtMqtt
        /usr/include/x86_64-linux-gnu/qt6/QtMqtt
        ${CMAKE_PREFIX_PATH}/include/QtMqtt
        NO_DEFAULT_PATH
    )
    
    if(MQTT_LIBRARY)
        message(STATUS "Found Qt MQTT library: ${MQTT_LIBRARY}")
        
        # If we found the library but not the include directory, set default paths
        if(NOT MQTT_INCLUDE_DIR)
            set(MQTT_INCLUDE_DIR /usr/include/qt6/QtMqtt)
            message(STATUS "MQTT include directory not found, using default: ${MQTT_INCLUDE_DIR}")
        else()
            message(STATUS "Found Qt MQTT include directory: ${MQTT_INCLUDE_DIR}")
        endif()
        
        # Create an imported target for MQTT if it wasn't found by find_package
        if(NOT TARGET Qt6::Mqtt)
            add_library(Qt6Mqtt SHARED IMPORTED)
            set_target_properties(Qt6Mqtt PROPERTIES
                IMPORTED_LOCATION "${MQTT_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${MQTT_INCLUDE_DIR}"
            )
            add_library(Qt6::Mqtt ALIAS Qt6Mqtt)
        endif()
    else()
        message(FATAL_ERROR "Qt MQTT module not found and BUILD_MQTT_LOCALLY is disabled. Please install the Qt MQTT module or enable BUILD_MQTT_LOCALLY.")
    endif()
else()
    # Build MQTT from source - check if we have the source locally first
    set(MQTT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src/mqtt")
    
    if(NOT EXISTS "${MQTT_SRC_DIR}")
        # Source not found locally, use FetchContent to get it
        message(STATUS "MQTT source not found locally, fetching from repository...")
        include(FetchContent)
        set(QTMQTT_PREFIX ${CMAKE_BINARY_DIR}/thirdparty/qtmqtt)
        FetchContent_Declare(
            QtMqttSrc
            GIT_REPOSITORY https://github.com/qt/qtmqtt.git
            GIT_TAG v${Qt6_VERSION}
            SOURCE_DIR ${CMAKE_BINARY_DIR}/thirdparty/qtmqtt
        )
        FetchContent_MakeAvailable(QtMqttSrc)
        set(MQTT_SRC_DIR "${CMAKE_BINARY_DIR}/thirdparty/qtmqtt/src/mqtt")
    else()
        message(STATUS "Using local MQTT source from: ${MQTT_SRC_DIR}")
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
    file(GLOB MQTT_SOURCES "${MQTT_SRC_DIR}/*.cpp")
    file(GLOB MQTT_HEADERS "${MQTT_SRC_DIR}/*.h")
    
    # Create MQTT library
    add_library(Qt6Mqtt STATIC ${MQTT_SOURCES} ${MQTT_HEADERS})
    
    # Set include directories
    target_include_directories(Qt6Mqtt PUBLIC ${MQTT_SRC_DIR})
    
    # If we used FetchContent, we need to include the src directory too
    if(EXISTS "${CMAKE_BINARY_DIR}/thirdparty/qtmqtt/src")
        target_include_directories(Qt6Mqtt PUBLIC ${CMAKE_BINARY_DIR}/thirdparty/qtmqtt/src)
    else()
        target_include_directories(Qt6Mqtt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src)
    endif()
    
    # Add Qt private headers if allowed
    if(QT_ALLOW_PRIVATE_INCLUDES AND QT_PRIVATE_HEADERS_FOUND)
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
if(BUILD_MQTT_LOCALLY AND NOT Qt6Mqtt_FOUND)
    if(EXISTS "${CMAKE_BINARY_DIR}/thirdparty/qtmqtt/src")
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${CMAKE_BINARY_DIR}/thirdparty/qtmqtt/src
            ${CMAKE_BINARY_DIR}/include
        )
    else()
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/qtmqtt/src
            ${CMAKE_BINARY_DIR}/include
        )
    endif()
endif()

# Copy config.json to build directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.json")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/config.json
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json
        COMMENT "Copying config.json to build directory"
    )
endif()

# Windows-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    
    # Installation rules
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.json")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION bin)
    endif()
    
    # Deploy Qt files when using the prebuilt MQTT module
    if(NOT BUILD_MQTT_LOCALLY AND TARGET Qt6::Mqtt)
        # Install MQTT library dll alongside the executable
        get_target_property(MQTT_LOCATION Qt6::Mqtt IMPORTED_LOCATION_RELEASE)
        if(MQTT_LOCATION)
            install(FILES ${MQTT_LOCATION} DESTINATION bin)
        endif()
    endif()
    
    # Package settings
    set(CPACK_GENERATOR "WIX;ZIP")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    set(CPACK_WIX_UPGRADE_GUID "43B8BE6D-3D79-41AC-831D-9B7E7932137A")
    include(CPack)
endif()