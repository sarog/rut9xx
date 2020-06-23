### This is make file for CLION ###
#include(${CMAKE_CURRENT_SOURCE_DIR}/lib.cmake)

# Setting package name
SET_PROJECT(libmdcollectd)

COMPILE_LIB(libmdcollectd shared "mdcollectd_data.c;include/mdcollectd_data.h")
INCLUDE_DIR("include")
ADD_LIB_TO_EXEC("libmdcollectd" "m;uci;tlt_uci;sqlite3;pthread")
SET_CFLAGS("-O2")

# Create Makefile for compilation, from this CMAKE
#FILL_MAKE_FILE()

# Use make command, to build package
#MAKE_PACKAGE()

# ONLY if SSHPASS is installed
##COPY_TO_ROUTER_LOC(false /usr/lib/ 192.168.1.1 root admin01)