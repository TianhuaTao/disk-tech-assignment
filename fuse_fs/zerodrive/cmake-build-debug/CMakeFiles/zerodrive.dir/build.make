# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /home/sam/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/201.7223.86/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/sam/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/201.7223.86/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/zerodrive.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/zerodrive.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/zerodrive.dir/flags.make

CMakeFiles/zerodrive.dir/DriveAgent.cpp.o: CMakeFiles/zerodrive.dir/flags.make
CMakeFiles/zerodrive.dir/DriveAgent.cpp.o: ../DriveAgent.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/zerodrive.dir/DriveAgent.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zerodrive.dir/DriveAgent.cpp.o -c /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/DriveAgent.cpp

CMakeFiles/zerodrive.dir/DriveAgent.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zerodrive.dir/DriveAgent.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/DriveAgent.cpp > CMakeFiles/zerodrive.dir/DriveAgent.cpp.i

CMakeFiles/zerodrive.dir/DriveAgent.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zerodrive.dir/DriveAgent.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/DriveAgent.cpp -o CMakeFiles/zerodrive.dir/DriveAgent.cpp.s

CMakeFiles/zerodrive.dir/op.cpp.o: CMakeFiles/zerodrive.dir/flags.make
CMakeFiles/zerodrive.dir/op.cpp.o: ../op.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/zerodrive.dir/op.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zerodrive.dir/op.cpp.o -c /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/op.cpp

CMakeFiles/zerodrive.dir/op.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zerodrive.dir/op.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/op.cpp > CMakeFiles/zerodrive.dir/op.cpp.i

CMakeFiles/zerodrive.dir/op.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zerodrive.dir/op.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/op.cpp -o CMakeFiles/zerodrive.dir/op.cpp.s

CMakeFiles/zerodrive.dir/zerodrive.cpp.o: CMakeFiles/zerodrive.dir/flags.make
CMakeFiles/zerodrive.dir/zerodrive.cpp.o: ../zerodrive.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/zerodrive.dir/zerodrive.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/zerodrive.dir/zerodrive.cpp.o -c /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/zerodrive.cpp

CMakeFiles/zerodrive.dir/zerodrive.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/zerodrive.dir/zerodrive.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/zerodrive.cpp > CMakeFiles/zerodrive.dir/zerodrive.cpp.i

CMakeFiles/zerodrive.dir/zerodrive.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/zerodrive.dir/zerodrive.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/zerodrive.cpp -o CMakeFiles/zerodrive.dir/zerodrive.cpp.s

# Object files for target zerodrive
zerodrive_OBJECTS = \
"CMakeFiles/zerodrive.dir/DriveAgent.cpp.o" \
"CMakeFiles/zerodrive.dir/op.cpp.o" \
"CMakeFiles/zerodrive.dir/zerodrive.cpp.o"

# External object files for target zerodrive
zerodrive_EXTERNAL_OBJECTS =

zerodrive: CMakeFiles/zerodrive.dir/DriveAgent.cpp.o
zerodrive: CMakeFiles/zerodrive.dir/op.cpp.o
zerodrive: CMakeFiles/zerodrive.dir/zerodrive.cpp.o
zerodrive: CMakeFiles/zerodrive.dir/build.make
zerodrive: CMakeFiles/zerodrive.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable zerodrive"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/zerodrive.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/zerodrive.dir/build: zerodrive

.PHONY : CMakeFiles/zerodrive.dir/build

CMakeFiles/zerodrive.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/zerodrive.dir/cmake_clean.cmake
.PHONY : CMakeFiles/zerodrive.dir/clean

CMakeFiles/zerodrive.dir/depend:
	cd /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug /home/sam/dev/disk-tech-assignment/fuse_fs/zerodrive/cmake-build-debug/CMakeFiles/zerodrive.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/zerodrive.dir/depend
