# Install script for directory: /home/dainius/Projects/azure-iot-sdk-c/uamqp

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/azure_uamqp_c" TYPE FILE FILES
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_role.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sender_settle_mode.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_receiver_settle_mode.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_handle.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_seconds.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_milliseconds.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_delivery_tag.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sequence_no.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_delivery_number.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_transfer_number.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_message_format.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_ietf_language_tag.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_fields.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_error.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_amqp_error.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_connection_error.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_session_error.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_link_error.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_open.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_begin.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_attach.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_flow.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_transfer.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_disposition.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_detach.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_end.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_close.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sasl_code.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sasl_mechanisms.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sasl_init.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sasl_challenge.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sasl_response.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_sasl_outcome.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_terminus_durability.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_node_properties.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_filter_set.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_source.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_target.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_annotations.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_message_id_ulong.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_message_id_uuid.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_message_id_binary.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_message_id_string.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_address_string.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_header.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_delivery_annotations.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_message_annotations.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_application_properties.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_data.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_amqp_sequence.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_amqp_value.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_footer.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_properties.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_received.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_accepted.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_rejected.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_released.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions_modified.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_definitions.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_frame_codec.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_management.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqp_types.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqpvalue.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/amqpvalue_to_string.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/async_operation.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/cbs.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/connection.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/frame_codec.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/header_detect_io.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/link.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/message.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/message_receiver.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/message_sender.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/messaging.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_anonymous.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_frame_codec.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_mechanism.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_server_mechanism.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_mssbcbs.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_plain.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/saslclientio.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/sasl_server_io.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/server_protocol_io.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/session.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/socket_listener.h"
    "/home/dainius/Projects/azure-iot-sdk-c/uamqp/./inc/azure_uamqp_c/uamqp.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/dainius/Projects/azure-iot-sdk-c/cmake/uamqp/libuamqp.a")
endif()

