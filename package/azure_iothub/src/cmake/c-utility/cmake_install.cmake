# Install script for directory: /home/dainius/Projects/azure-iot-sdk-c/c-utility

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/azure_c_shared_utility" TYPE FILE FILES
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/agenttime.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/base32.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/base64.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/buffer_.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/connection_string_parser.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/crt_abstractions.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/constmap.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/condition.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/const_defines.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/inc/azure_c_shared_utility/consolelogger.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/doublylinkedlist.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/envvariable.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/gballoc.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/gbnetwork.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/gb_stdio.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/gb_time.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/gb_rand.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/hmac.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/hmacsha256.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/http_proxy_io.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/singlylinkedlist.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/lock.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/macro_utils.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/map.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/optimize_size.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/platform.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/refcount.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/sastoken.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/sha-private.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/shared_util_options.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/sha.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/socketio.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/stdint_ce6.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/strings.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/strings_types.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/string_token.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/string_tokenizer.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/string_tokenizer_types.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/tlsio_options.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/tickcounter.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/threadapi.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/xio.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/umock_c_prod.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/uniqueid.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/uuid.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/urlencode.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/vector.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/vector_types.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/vector_types_internal.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/xlogging.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/constbuffer.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/tlsio.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/optionhandler.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./adapters/linux_time.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/wsio.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/uws_client.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/uws_frame_encoder.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/utf8_checker.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/ws_url.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/httpapi.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/httpapiex.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/httpapiexsas.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/httpheaders.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/tlsio_openssl.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./inc/azure_c_shared_utility/x509_openssl.h"
    "/home/dainius/Projects/azure-iot-sdk-c/c-utility/./pal/linux/refcount_os.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/c-utility/libaziotsharedutil.a")
endif()

