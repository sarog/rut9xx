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
include CMakeFiles/parson.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/parson.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/parson.dir/flags.make

CMakeFiles/parson.dir/deps/parson/parson.c.o: CMakeFiles/parson.dir/flags.make
CMakeFiles/parson.dir/deps/parson/parson.c.o: ../deps/parson/parson.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/parson.dir/deps/parson/parson.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/parson.dir/deps/parson/parson.c.o   -c /home/dainius/Projects/azure-iot-sdk-c/deps/parson/parson.c

CMakeFiles/parson.dir/deps/parson/parson.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/parson.dir/deps/parson/parson.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/dainius/Projects/azure-iot-sdk-c/deps/parson/parson.c > CMakeFiles/parson.dir/deps/parson/parson.c.i

CMakeFiles/parson.dir/deps/parson/parson.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/parson.dir/deps/parson/parson.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/dainius/Projects/azure-iot-sdk-c/deps/parson/parson.c -o CMakeFiles/parson.dir/deps/parson/parson.c.s

CMakeFiles/parson.dir/deps/parson/parson.c.o.requires:

.PHONY : CMakeFiles/parson.dir/deps/parson/parson.c.o.requires

CMakeFiles/parson.dir/deps/parson/parson.c.o.provides: CMakeFiles/parson.dir/deps/parson/parson.c.o.requires
	$(MAKE) -f CMakeFiles/parson.dir/build.make CMakeFiles/parson.dir/deps/parson/parson.c.o.provides.build
.PHONY : CMakeFiles/parson.dir/deps/parson/parson.c.o.provides

CMakeFiles/parson.dir/deps/parson/parson.c.o.provides.build: CMakeFiles/parson.dir/deps/parson/parson.c.o


# Object files for target parson
parson_OBJECTS = \
"CMakeFiles/parson.dir/deps/parson/parson.c.o"

# External object files for target parson
parson_EXTERNAL_OBJECTS =

libparson.a: CMakeFiles/parson.dir/deps/parson/parson.c.o
libparson.a: CMakeFiles/parson.dir/build.make
libparson.a: CMakeFiles/parson.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libparson.a"
	$(CMAKE_COMMAND) -P CMakeFiles/parson.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/parson.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/parson.dir/build: libparson.a

.PHONY : CMakeFiles/parson.dir/build

CMakeFiles/parson.dir/requires: CMakeFiles/parson.dir/deps/parson/parson.c.o.requires

.PHONY : CMakeFiles/parson.dir/requires

CMakeFiles/parson.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/parson.dir/cmake_clean.cmake
.PHONY : CMakeFiles/parson.dir/clean

CMakeFiles/parson.dir/depend:
	cd /home/dainius/Projects/azure-iot-sdk-c/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dainius/Projects/azure-iot-sdk-c /home/dainius/Projects/azure-iot-sdk-c /home/dainius/Projects/azure-iot-sdk-c/cmake /home/dainius/Projects/azure-iot-sdk-c/cmake /home/dainius/Projects/azure-iot-sdk-c/cmake/CMakeFiles/parson.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/parson.dir/depend

