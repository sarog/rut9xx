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
include iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/depend.make

# Include the progress variables for this target.
include iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/progress.make

# Include the compile flags for this target's objects.
include iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/flags.make

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/flags.make
iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o: ../iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob.c

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob.c > CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.i

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob.c -o CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.s

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.requires:

.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.requires

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.provides: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.requires
	$(MAKE) -f iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/build.make iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.provides.build
.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.provides

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.provides.build: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o


iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/flags.make
iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o: ../certs/certs.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/certs/certs.c

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/certs/certs.c > CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.i

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/certs/certs.c -o CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.s

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.requires:

.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.requires

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.provides: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.requires
	$(MAKE) -f iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/build.make iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.provides.build
.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.provides

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.provides.build: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o


# Object files for target iothub_client_sample_upload_to_blob
iothub_client_sample_upload_to_blob_OBJECTS = \
"CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o" \
"CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o"

# External object files for target iothub_client_sample_upload_to_blob
iothub_client_sample_upload_to_blob_EXTERNAL_OBJECTS =

iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/build.make
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/libiothub_client.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/libiothub_client_http_transport.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: c-utility/libaziotsharedutil.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/libiothub_client_amqp_transport.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/libiothub_client_amqp_ws_transport.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/libiothub_client_mqtt_transport.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/libiothub_client_mqtt_ws_transport.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: umqtt/libumqtt.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: c-utility/libaziotsharedutil.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: /usr/lib/x86_64-linux-gnu/libssl.so
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: /usr/lib/x86_64-linux-gnu/libcrypto.so
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: libparson.a
iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable iothub_client_sample_upload_to_blob"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iothub_client_sample_upload_to_blob.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/build: iothub_client/samples/iothub_client_sample_upload_to_blob/iothub_client_sample_upload_to_blob

.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/build

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/requires: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/iothub_client_sample_upload_to_blob.c.o.requires
iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/requires: iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/__/__/__/certs/certs.c.o.requires

.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/requires

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/clean:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob && $(CMAKE_COMMAND) -P CMakeFiles/iothub_client_sample_upload_to_blob.dir/cmake_clean.cmake
.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/clean

iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/depend:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dainius/Projects/azure-iot-sdk-c /home/dainius/Projects/azure-iot-sdk-c/iothub_client/samples/iothub_client_sample_upload_to_blob /home/dainius/Projects/azure-iot-sdk-c/cmake /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob /home/dainius/Projects/azure-iot-sdk-c/cmake/iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : iothub_client/samples/iothub_client_sample_upload_to_blob/CMakeFiles/iothub_client_sample_upload_to_blob.dir/depend

