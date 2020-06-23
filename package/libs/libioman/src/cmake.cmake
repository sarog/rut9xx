### This is make file for CLION ###
#include(${CMAKE_CURRENT_SOURCE_DIR}/lib.cmake)

# Setting package name
SET_PROJECT(libioman)

COMPILE_LIB(libioman shared "iomanl.c")
INCLUDE_DIR("include")
ADD_LIB("tlt_uci;sms_utils;gsm;usb-1.0;eventslog")
SET_CFLAGS("-O2")

# Create Makefile for compilation, from this CMAKE
#FILL_MAKE_FILE()

# Use make command, to build package
#MAKE_PACKAGE()

# ONLY if SSHPASS is installed
##COPY_TO_ROUTER_LOC(false /usr/lib/ 192.168.1.1 root admin01)