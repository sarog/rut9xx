# Install script for directory: /home/dainius/Projects/azure-iot-sdk-c/iothub_service_client

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iothub_service_client" TYPE FILE FILES
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_deviceconfiguration.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_devicemethod.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_devicetwin.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_messaging.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_messaging_ll.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_registrymanager.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_sc_version.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/./inc/iothub_service_client_auth.h"
    "/home/dainius/Projects/azure-iot-sdk-c/iothub_service_client/../iothub_client/inc/iothub_message.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_service_client/libiothub_service_client.a")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_service_client/samples/cmake_install.cmake")

endif()

