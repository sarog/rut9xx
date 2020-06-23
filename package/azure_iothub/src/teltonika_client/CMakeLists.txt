#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for remote monitoring sample

compileAsC99()

set(teltonika_client_c_files
    azure_iothub.c cJSON.c
)

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

#Conditionally use the SDK trusted certs in the samples
#if(${use_sample_trusted_cert})
    add_definitions(-DSET_TRUSTED_CERT_IN_SAMPLES)
    include_directories(${PROJECT_SOURCE_DIR}/certs)
    set(remote_monitoring_c_files ${remote_monitoring_c_files} ${PROJECT_SOURCE_DIR}/certs/certs.c)
    #endif()

include_directories(.)
include_directories(${IOTHUB_CLIENT_INC_FOLDER})
include_directories((${PROJECT_SOURCE_DIR}/deps/parson))

add_executable(azure_iothub ${teltonika_client_c_files})

target_link_libraries(azure_iothub iothub_client;usb-1.0;uci;gsm;blobmsg_json;ubus;tlt_base;mosquitto)
linkSharedUtil(azure_iothub)


target_link_libraries(azure_iothub iothub_client_mqtt_transport)
linkMqttLibrary(azure_iothub)
add_definitions(-DUSE_MQTT)

