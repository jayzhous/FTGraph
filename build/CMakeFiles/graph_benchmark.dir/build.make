# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build

# Include any dependencies generated for this target.
include CMakeFiles/graph_benchmark.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/graph_benchmark.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/graph_benchmark.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/graph_benchmark.dir/flags.make

CMakeFiles/graph_benchmark.dir/src/edge.cc.o: CMakeFiles/graph_benchmark.dir/flags.make
CMakeFiles/graph_benchmark.dir/src/edge.cc.o: ../src/edge.cc
CMakeFiles/graph_benchmark.dir/src/edge.cc.o: CMakeFiles/graph_benchmark.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/graph_benchmark.dir/src/edge.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/graph_benchmark.dir/src/edge.cc.o -MF CMakeFiles/graph_benchmark.dir/src/edge.cc.o.d -o CMakeFiles/graph_benchmark.dir/src/edge.cc.o -c /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/src/edge.cc

CMakeFiles/graph_benchmark.dir/src/edge.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/graph_benchmark.dir/src/edge.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/src/edge.cc > CMakeFiles/graph_benchmark.dir/src/edge.cc.i

CMakeFiles/graph_benchmark.dir/src/edge.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/graph_benchmark.dir/src/edge.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/src/edge.cc -o CMakeFiles/graph_benchmark.dir/src/edge.cc.s

CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o: CMakeFiles/graph_benchmark.dir/flags.make
CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o: ../apps/graph_benchmark.cc
CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o: CMakeFiles/graph_benchmark.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o -MF CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o.d -o CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o -c /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/apps/graph_benchmark.cc

CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/apps/graph_benchmark.cc > CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.i

CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/apps/graph_benchmark.cc -o CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.s

# Object files for target graph_benchmark
graph_benchmark_OBJECTS = \
"CMakeFiles/graph_benchmark.dir/src/edge.cc.o" \
"CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o"

# External object files for target graph_benchmark
graph_benchmark_EXTERNAL_OBJECTS =

graph_benchmark: CMakeFiles/graph_benchmark.dir/src/edge.cc.o
graph_benchmark: CMakeFiles/graph_benchmark.dir/apps/graph_benchmark.cc.o
graph_benchmark: CMakeFiles/graph_benchmark.dir/build.make
graph_benchmark: CMakeFiles/graph_benchmark.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable graph_benchmark"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/graph_benchmark.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/graph_benchmark.dir/build: graph_benchmark
.PHONY : CMakeFiles/graph_benchmark.dir/build

CMakeFiles/graph_benchmark.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/graph_benchmark.dir/cmake_clean.cmake
.PHONY : CMakeFiles/graph_benchmark.dir/clean

CMakeFiles/graph_benchmark.dir/depend:
	cd /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7 /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7 /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build /mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/build/CMakeFiles/graph_benchmark.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/graph_benchmark.dir/depend

