# Install script for directory: /home/dainius/Projects/azure-iot-sdk-c/iothub_client

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/azureiot" TYPE FILE FILES
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client_core.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client_core_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client_core_common.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_diagnostic.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client_options.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_private.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_client_version.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_device_client.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_device_client_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_module_client.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_module_client_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_transport_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_message.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/blob.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_ll_uploadtoblob.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_authorization.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_retry_control.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothubtransporthttp.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothub_transport_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_authorization.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_retry_control.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_common.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_device.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_cbs_auth.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_connection.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_telemetry_messenger.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_twin_messenger.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_amqp_messenger.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransportamqp_methods.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/message_queue.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/uamqp_messaging.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothubtransportamqp.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothubtransportamqp_websockets.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_authorization.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_retry_control.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_mqtt_common.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothubtransportmqtt.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_authorization.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothub_client_retry_control.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/internal/iothubtransport_mqtt_common.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_client/./inc/iothubtransportmqtt_websockets.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/libiothub_client.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/libiothub_client_http_transport.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/libiothub_client_amqp_transport.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/libiothub_client_amqp_ws_transport.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/libiothub_client_mqtt_transport.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/libiothub_client_mqtt_ws_transport.a")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/cmake_install.cmake")
  include("/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/tests/cmake_install.cmake")

endif()

