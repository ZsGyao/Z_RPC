# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zgys/workspace/zRPC

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zgys/workspace/zRPC/build

# Include any dependencies generated for this target.
include CMakeFiles/test_log.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_log.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_log.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_log.dir/flags.make

CMakeFiles/test_log.dir/tests/test_log.cpp.o: CMakeFiles/test_log.dir/flags.make
CMakeFiles/test_log.dir/tests/test_log.cpp.o: ../tests/test_log.cpp
CMakeFiles/test_log.dir/tests/test_log.cpp.o: CMakeFiles/test_log.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zgys/workspace/zRPC/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_log.dir/tests/test_log.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_log.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_log.dir/tests/test_log.cpp.o -MF CMakeFiles/test_log.dir/tests/test_log.cpp.o.d -o CMakeFiles/test_log.dir/tests/test_log.cpp.o -c /home/zgys/workspace/zRPC/tests/test_log.cpp

CMakeFiles/test_log.dir/tests/test_log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_log.dir/tests/test_log.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_log.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zgys/workspace/zRPC/tests/test_log.cpp > CMakeFiles/test_log.dir/tests/test_log.cpp.i

CMakeFiles/test_log.dir/tests/test_log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_log.dir/tests/test_log.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_log.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zgys/workspace/zRPC/tests/test_log.cpp -o CMakeFiles/test_log.dir/tests/test_log.cpp.s

# Object files for target test_log
test_log_OBJECTS = \
"CMakeFiles/test_log.dir/tests/test_log.cpp.o"

# External object files for target test_log
test_log_EXTERNAL_OBJECTS =

../bin/test_log: CMakeFiles/test_log.dir/tests/test_log.cpp.o
../bin/test_log: CMakeFiles/test_log.dir/build.make
../bin/test_log: ../lib/libsrc.so
../bin/test_log: CMakeFiles/test_log.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zgys/workspace/zRPC/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test_log"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_log.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_log.dir/build: ../bin/test_log
.PHONY : CMakeFiles/test_log.dir/build

CMakeFiles/test_log.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_log.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_log.dir/clean

CMakeFiles/test_log.dir/depend:
	cd /home/zgys/workspace/zRPC/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zgys/workspace/zRPC /home/zgys/workspace/zRPC /home/zgys/workspace/zRPC/build /home/zgys/workspace/zRPC/build /home/zgys/workspace/zRPC/build/CMakeFiles/test_log.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_log.dir/depend

