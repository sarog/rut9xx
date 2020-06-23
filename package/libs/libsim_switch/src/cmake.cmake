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
SET_PROJECT(libsim_switch)

COMPILE_LIB(libsim_switch shared "sim_sw.c")
INCLUDE_DIR("include")

ADD_LIB("ubus")

# Use make command, to build package
#MAKE_PACKAGE()

# ONLY if SSHPASS is installed
#COPY_TO_ROUTER_LOC(false /usr/lib 192.168.1.1 root admin01)

# Cleaning variables
CLEAN()