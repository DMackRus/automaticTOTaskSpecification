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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/davidrussell/catkin_ws/src/physicsSimSwitching

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/davidrussell/catkin_ws/src/physicsSimSwitching

# Include any dependencies generated for this target.
include CMakeFiles/physicsSimSwitching.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/physicsSimSwitching.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/physicsSimSwitching.dir/flags.make

CMakeFiles/physicsSimSwitching.dir/main.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/main.cpp.o: main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/physicsSimSwitching.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/main.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/main.cpp

CMakeFiles/physicsSimSwitching.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/main.cpp > CMakeFiles/physicsSimSwitching.dir/main.cpp.i

CMakeFiles/physicsSimSwitching.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/main.cpp -o CMakeFiles/physicsSimSwitching.dir/main.cpp.s

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.o: physicsSimulators/physicsSimulator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/physicsSimulator.cpp

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/physicsSimulator.cpp > CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.i

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/physicsSimulator.cpp -o CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.s

CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.o: stdInclude/stdInclude.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/stdInclude/stdInclude.cpp

CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/stdInclude/stdInclude.cpp > CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.i

CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/stdInclude/stdInclude.cpp -o CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.s

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.o: physicsSimulators/MuJoCoHelper.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/MuJoCoHelper.cpp

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/MuJoCoHelper.cpp > CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.i

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/MuJoCoHelper.cpp -o CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.s

CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.o: modelTranslator/modelTranslator.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/modelTranslator/modelTranslator.cpp

CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/modelTranslator/modelTranslator.cpp > CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.i

CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/modelTranslator/modelTranslator.cpp -o CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.s

CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.o: visualizer/visualizer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/visualizer/visualizer.cpp

CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/visualizer/visualizer.cpp > CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.i

CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/visualizer/visualizer.cpp -o CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.s

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.o: physicsSimulators/bulletHelper.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/bulletHelper.cpp

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/bulletHelper.cpp > CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.i

CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/physicsSimulators/bulletHelper.cpp -o CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.s

CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.o: CMakeFiles/physicsSimSwitching.dir/flags.make
CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.o: modelTranslator/doublePendulum.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.o -c /home/davidrussell/catkin_ws/src/physicsSimSwitching/modelTranslator/doublePendulum.cpp

CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/davidrussell/catkin_ws/src/physicsSimSwitching/modelTranslator/doublePendulum.cpp > CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.i

CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/davidrussell/catkin_ws/src/physicsSimSwitching/modelTranslator/doublePendulum.cpp -o CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.s

# Object files for target physicsSimSwitching
physicsSimSwitching_OBJECTS = \
"CMakeFiles/physicsSimSwitching.dir/main.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.o" \
"CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.o"

# External object files for target physicsSimSwitching
physicsSimSwitching_EXTERNAL_OBJECTS =

devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/main.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/physicsSimulators/physicsSimulator.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/stdInclude/stdInclude.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/physicsSimulators/MuJoCoHelper.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/modelTranslator/modelTranslator.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/visualizer/visualizer.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/physicsSimulators/bulletHelper.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/modelTranslator/doublePendulum.cpp.o
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/build.make
devel/lib/physicsSimSwitching/physicsSimSwitching: /home/davidrussell/catkin_ws/src/mujoco/lib/libmujoco.so.2.3.2
devel/lib/physicsSimSwitching/physicsSimSwitching: CMakeFiles/physicsSimSwitching.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX executable devel/lib/physicsSimSwitching/physicsSimSwitching"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/physicsSimSwitching.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/physicsSimSwitching.dir/build: devel/lib/physicsSimSwitching/physicsSimSwitching

.PHONY : CMakeFiles/physicsSimSwitching.dir/build

CMakeFiles/physicsSimSwitching.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/physicsSimSwitching.dir/cmake_clean.cmake
.PHONY : CMakeFiles/physicsSimSwitching.dir/clean

CMakeFiles/physicsSimSwitching.dir/depend:
	cd /home/davidrussell/catkin_ws/src/physicsSimSwitching && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/davidrussell/catkin_ws/src/physicsSimSwitching /home/davidrussell/catkin_ws/src/physicsSimSwitching /home/davidrussell/catkin_ws/src/physicsSimSwitching /home/davidrussell/catkin_ws/src/physicsSimSwitching /home/davidrussell/catkin_ws/src/physicsSimSwitching/CMakeFiles/physicsSimSwitching.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/physicsSimSwitching.dir/depend

