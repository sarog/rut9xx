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
include serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/depend.make

# Include the progress variables for this target.
include serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/progress.make

# Include the compile flags for this target's objects.
include serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/flags.make

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/flags.make
serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o: ../serializer/samples/simplesample_amqp/simplesample_amqp.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp/simplesample_amqp.c

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp/simplesample_amqp.c > CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.i

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp/simplesample_amqp.c -o CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.s

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.requires:

.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.requires

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.provides: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.requires
	$(MAKE) -f serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/build.make serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.provides.build
.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.provides

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.provides.build: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o


serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/flags.make
serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o: ../serializer/samples/simplesample_amqp/linux/main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/simplesample_amqp.dir/linux/main.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp/linux/main.c

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/simplesample_amqp.dir/linux/main.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp/linux/main.c > CMakeFiles/simplesample_amqp.dir/linux/main.c.i

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/simplesample_amqp.dir/linux/main.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp/linux/main.c -o CMakeFiles/simplesample_amqp.dir/linux/main.c.s

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.requires:

.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.requires

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.provides: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.requires
	$(MAKE) -f serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/build.make serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.provides.build
.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.provides

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.provides.build: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o


serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/flags.make
serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o: ../certs/certs.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/certs/certs.c

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/certs/certs.c > CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.i

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/certs/certs.c -o CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.s

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.requires:

.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.requires

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.provides: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.requires
	$(MAKE) -f serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/build.make serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.provides.build
.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.provides

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.provides.build: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o


# Object files for target simplesample_amqp
simplesample_amqp_OBJECTS = \
"CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o" \
"CMakeFiles/simplesample_amqp.dir/linux/main.c.o" \
"CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o"

# External object files for target simplesample_amqp
simplesample_amqp_EXTERNAL_OBJECTS =

serializer/samples/simplesample_amqp/simplesample_amqp: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o
serializer/samples/simplesample_amqp/simplesample_amqp: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o
serializer/samples/simplesample_amqp/simplesample_amqp: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o
serializer/samples/simplesample_amqp/simplesample_amqp: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/build.make
serializer/samples/simplesample_amqp/simplesample_amqp: serializer/libserializer.a
serializer/samples/simplesample_amqp/simplesample_amqp: iothub_client/libiothub_client.a
serializer/samples/simplesample_amqp/simplesample_amqp: iothub_client/libiothub_client_amqp_transport.a
serializer/samples/simplesample_amqp/simplesample_amqp: c-utility/libaziotsharedutil.a
serializer/samples/simplesample_amqp/simplesample_amqp: uamqp/libuamqp.a
serializer/samples/simplesample_amqp/simplesample_amqp: c-utility/libaziotsharedutil.a
serializer/samples/simplesample_amqp/simplesample_amqp: libparson.a
serializer/samples/simplesample_amqp/simplesample_amqp: iothub_client/libiothub_client_http_transport.a
serializer/samples/simplesample_amqp/simplesample_amqp: iothub_client/libiothub_client_amqp_ws_transport.a
serializer/samples/simplesample_amqp/simplesample_amqp: iothub_client/libiothub_client_mqtt_transport.a
serializer/samples/simplesample_amqp/simplesample_amqp: iothub_client/libiothub_client_mqtt_ws_transport.a
serializer/samples/simplesample_amqp/simplesample_amqp: umqtt/libumqtt.a
serializer/samples/simplesample_amqp/simplesample_amqp: c-utility/libaziotsharedutil.a
serializer/samples/simplesample_amqp/simplesample_amqp: /usr/lib/x86_64-linux-gnu/libssl.so
serializer/samples/simplesample_amqp/simplesample_amqp: /usr/lib/x86_64-linux-gnu/libcrypto.so
serializer/samples/simplesample_amqp/simplesample_amqp: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable simplesample_amqp"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simplesample_amqp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/build: serializer/samples/simplesample_amqp/simplesample_amqp

.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/build

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/requires: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/simplesample_amqp.c.o.requires
serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/requires: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/linux/main.c.o.requires
serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/requires: serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/__/__/__/certs/certs.c.o.requires

.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/requires

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/clean:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp && $(CMAKE_COMMAND) -P CMakeFiles/simplesample_amqp.dir/cmake_clean.cmake
.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/clean

serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/depend:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dainius/Projects/azure-iot-sdk-c /home/dainius/Projects/azure-iot-sdk-c/serializer/samples/simplesample_amqp /home/dainius/Projects/azure-iot-sdk-c/cmake /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp /home/dainius/Projects/azure-iot-sdk-c/cmake/serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : serializer/samples/simplesample_amqp/CMakeFiles/simplesample_amqp.dir/depend

