# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dainius/Projects/azure-iot-sdk-c

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dainius/Projects/azure-iot-sdk-c/cmake

# Include any dependencies generated for this target.
include iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/depend.make

# Include the progress variables for this target.
include iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/progress.make

# Include the compile flags for this target's objects.
include iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/flags.make

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/flags.make
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o: ../iothub_client/src/iothub_client_authorization.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothub_client_authorization.c

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothub_client_authorization.c > CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.i

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothub_client_authorization.c -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.s

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.requires:

.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.requires

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.provides: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.requires
	$(MAKE) -f iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build.make iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.provides.build
.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.provides

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.provides.build: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o


iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/flags.make
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o: ../iothub_client/src/iothub_client_retry_control.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothub_client_retry_control.c

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothub_client_retry_control.c > CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.i

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothub_client_retry_control.c -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.s

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.requires:

.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.requires

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.provides: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.requires
	$(MAKE) -f iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build.make iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.provides.build
.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.provides

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.provides.build: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o


iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/flags.make
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o: ../iothub_client/src/iothubtransport_mqtt_common.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothubtransport_mqtt_common.c

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothubtransport_mqtt_common.c > CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.i

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothubtransport_mqtt_common.c -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.s

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.requires:

.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.requires

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.provides: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.requires
	$(MAKE) -f iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build.make iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.provides.build
.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.provides

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.provides.build: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o


iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/flags.make
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o: ../iothub_client/src/iothubtransportmqtt.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothubtransportmqtt.c

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothubtransportmqtt.c > CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.i

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/iothub_client/src/iothubtransportmqtt.c -o CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.s

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.requires:

.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.requires

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.provides: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.requires
	$(MAKE) -f iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build.make iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.provides.build
.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.provides

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.provides.build: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o


# Object files for target iothub_client_mqtt_transport
iothub_client_mqtt_transport_OBJECTS = \
"CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o" \
"CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o" \
"CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o" \
"CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o"

# External object files for target iothub_client_mqtt_transport
iothub_client_mqtt_transport_EXTERNAL_OBJECTS =

iothub_client/libiothub_client_mqtt_transport.a: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o
iothub_client/libiothub_client_mqtt_transport.a: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o
iothub_client/libiothub_client_mqtt_transport.a: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o
iothub_client/libiothub_client_mqtt_transport.a: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o
iothub_client/libiothub_client_mqtt_transport.a: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build.make
iothub_client/libiothub_client_mqtt_transport.a: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C static library libiothub_client_mqtt_transport.a"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && $(CMAKE_COMMAND) -P CMakeFiles/iothub_client_mqtt_transport.dir/cmake_clean_target.cmake
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iothub_client_mqtt_transport.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build: iothub_client/libiothub_client_mqtt_transport.a

.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/build

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/requires: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_authorization.c.o.requires
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/requires: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothub_client_retry_control.c.o.requires
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/requires: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransport_mqtt_common.c.o.requires
iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/requires: iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/src/iothubtransportmqtt.c.o.requires

.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/requires

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/clean:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client && $(CMAKE_COMMAND) -P CMakeFiles/iothub_client_mqtt_transport.dir/cmake_clean.cmake
.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/clean

iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/depend:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dainius/Projects/azure-iot-sdk-c /home/dainius/Projects/azure-iot-sdk-c/iothub_client /home/dainius/Projects/azure-iot-sdk-c/cmake /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : iothub_client/CMakeFiles/iothub_client_mqtt_transport.dir/depend

