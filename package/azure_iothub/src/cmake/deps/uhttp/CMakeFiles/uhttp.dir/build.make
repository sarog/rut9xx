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
include deps/uhttp/CMakeFiles/uhttp.dir/depend.make

# Include the progress variables for this target.
include deps/uhttp/CMakeFiles/uhttp.dir/progress.make

# Include the compile flags for this target's objects.
include deps/uhttp/CMakeFiles/uhttp.dir/flags.make

deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o: deps/uhttp/CMakeFiles/uhttp.dir/flags.make
deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o: ../deps/uhttp/src/uhttp.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/uhttp.dir/src/uhttp.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/deps/uhttp/src/uhttp.c

deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uhttp.dir/src/uhttp.c.i"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/deps/uhttp/src/uhttp.c > CMakeFiles/uhttp.dir/src/uhttp.c.i

deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uhttp.dir/src/uhttp.c.s"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/deps/uhttp/src/uhttp.c -o CMakeFiles/uhttp.dir/src/uhttp.c.s

deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.requires:

.PHONY : deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.requires

deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.provides: deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.requires
	$(MAKE) -f deps/uhttp/CMakeFiles/uhttp.dir/build.make deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.provides.build
.PHONY : deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.provides

deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.provides.build: deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o


# Object files for target uhttp
uhttp_OBJECTS = \
"CMakeFiles/uhttp.dir/src/uhttp.c.o"

# External object files for target uhttp
uhttp_EXTERNAL_OBJECTS =

deps/uhttp/libuhttp.a: deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o
deps/uhttp/libuhttp.a: deps/uhttp/CMakeFiles/uhttp.dir/build.make
deps/uhttp/libuhttp.a: deps/uhttp/CMakeFiles/uhttp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libuhttp.a"
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp && $(CMAKE_COMMAND) -P CMakeFiles/uhttp.dir/cmake_clean_target.cmake
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uhttp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
deps/uhttp/CMakeFiles/uhttp.dir/build: deps/uhttp/libuhttp.a

.PHONY : deps/uhttp/CMakeFiles/uhttp.dir/build

deps/uhttp/CMakeFiles/uhttp.dir/requires: deps/uhttp/CMakeFiles/uhttp.dir/src/uhttp.c.o.requires

.PHONY : deps/uhttp/CMakeFiles/uhttp.dir/requires

deps/uhttp/CMakeFiles/uhttp.dir/clean:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp && $(CMAKE_COMMAND) -P CMakeFiles/uhttp.dir/cmake_clean.cmake
.PHONY : deps/uhttp/CMakeFiles/uhttp.dir/clean

deps/uhttp/CMakeFiles/uhttp.dir/depend:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dainius/Projects/azure-iot-sdk-c /home/dainius/Projects/azure-iot-sdk-c/deps/uhttp /home/dainius/Projects/azure-iot-sdk-c/cmake /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp /home/dainius/Projects/azure-iot-sdk-c/cmake/deps/uhttp/CMakeFiles/uhttp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : deps/uhttp/CMakeFiles/uhttp.dir/depend

