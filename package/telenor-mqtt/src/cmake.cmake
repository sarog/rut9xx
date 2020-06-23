### This is make file for CLION ###
#include(${CMAKE_CURRENT_SOURCE_DIR}/lib.cmake)

# Setting package name
SET_PROJECT(telenor_mqtt)

COMPILE(telenor_mqtt "telenor-mqtt.c;cJSON.c")
DEFINE(telenor_mqtt "RUT")

ADD_LIB("mosquitto;mnfinfo;usb-1.0;uci;gsm;gps;ubus;ubox;blobmsg_json;json-c;eventslog;tlt_uci;unhandler;tlt_base;crypto;pthread;curl;m;tlt_socket_man")


# Create Makefile for compilation, from this CMAKE
#FILL_MAKE_FILE()

# Use make command, to build package
#MAKE_PACKAGE()

# ONLY if SSHPASS is installed
COPY_TO_ROUTER_LOC(false /usr/sbin 192.168.1.1 root admin01)

# Cleaning variables
CLEAN()