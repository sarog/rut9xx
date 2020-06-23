### This is make file for CLION ###
# Explanations:

# SET_PROJECT is used to define the project and if used for 'make package/SET_PROJECT/compile'
# If building shared/static library, use:
# COMPILE_LIB(NAME shared/static "objects")
# The lib will be named: libNAME.so
# If need to include, includes directory for compilation, use:
# INCLUDE_DIR(DIR_NAME) (this will include directory, starting point is cmake.cmake location)
# ADD_LIB("libs") (no need for -l) Will add the libraries, to all executables in this cmake project.
# ADD_LIB_TO_EXEC(EXEC_NAME "libs") Will add the libraries, only to this executable.
# Building normal program, use:
# COMPILE(name "objects")
# FILL_MAKE_FILE() function will create Makefile, used for openwrt to compile with 'make package/PROJECT/compile'
# MAKE_PACKAGE() function will call original 'make package/PROJECT/compile'
# COPY_TO_ROUTER_LOC(true/false(from original make, or from clion) pathWhereToPut IP user pass)

# Setting package name
SET_PROJECT(libgsm)

COMPILE_LIB(libgsm shared "modem.c;pduconv.c;sms.c;uci_function.c")
ADD_LIB("usb-1.0;uci;eventslog;tlt_base;tlt_socket_man")
INCLUDE_DIR("include")
SET_CFLAGS("-O2")

# Create Makefile for compilation, from this CMAKE
#FILL_MAKE_FILE()

# Use make command, to build package
#MAKE_PACKAGE()

# ONLY if SSHPASS is installed
#COPY_TO_ROUTER_LOC(false /usr/lib 192.168.1.1 root admin01)

# Cleaning variables
CLEAN()