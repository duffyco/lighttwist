# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jon/lighttwist/imgu

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jon/lighttwist/imgu

# Include any dependencies generated for this target.
include examples/CMakeFiles/imguFit.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/imguFit.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/imguFit.dir/flags.make

examples/CMakeFiles/imguFit.dir/imguFit.o: examples/CMakeFiles/imguFit.dir/flags.make
examples/CMakeFiles/imguFit.dir/imguFit.o: examples/imguFit.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jon/lighttwist/imgu/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/CMakeFiles/imguFit.dir/imguFit.o"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/imguFit.dir/imguFit.o   -c /home/jon/lighttwist/imgu/examples/imguFit.c

examples/CMakeFiles/imguFit.dir/imguFit.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/imguFit.dir/imguFit.i"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/jon/lighttwist/imgu/examples/imguFit.c > CMakeFiles/imguFit.dir/imguFit.i

examples/CMakeFiles/imguFit.dir/imguFit.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/imguFit.dir/imguFit.s"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/jon/lighttwist/imgu/examples/imguFit.c -o CMakeFiles/imguFit.dir/imguFit.s

examples/CMakeFiles/imguFit.dir/imguFit.o.requires:
.PHONY : examples/CMakeFiles/imguFit.dir/imguFit.o.requires

examples/CMakeFiles/imguFit.dir/imguFit.o.provides: examples/CMakeFiles/imguFit.dir/imguFit.o.requires
	$(MAKE) -f examples/CMakeFiles/imguFit.dir/build.make examples/CMakeFiles/imguFit.dir/imguFit.o.provides.build
.PHONY : examples/CMakeFiles/imguFit.dir/imguFit.o.provides

examples/CMakeFiles/imguFit.dir/imguFit.o.provides.build: examples/CMakeFiles/imguFit.dir/imguFit.o

# Object files for target imguFit
imguFit_OBJECTS = \
"CMakeFiles/imguFit.dir/imguFit.o"

# External object files for target imguFit
imguFit_EXTERNAL_OBJECTS =

examples/imguFit: examples/CMakeFiles/imguFit.dir/imguFit.o
examples/imguFit: src/libimgu.so
examples/imguFit: /usr/lib/i386-linux-gnu/libpng.so
examples/imguFit: /usr/lib/i386-linux-gnu/libz.so
examples/imguFit: /usr/lib/i386-linux-gnu/libjpeg.so
examples/imguFit: /usr/lib/libnetpbm.so
examples/imguFit: /usr/lib/i386-linux-gnu/libavcodec.so
examples/imguFit: /usr/lib/i386-linux-gnu/libavutil.so
examples/imguFit: /usr/lib/i386-linux-gnu/libavformat.so
examples/imguFit: /usr/lib/i386-linux-gnu/libswscale.so
examples/imguFit: /usr/lib/libfftw3_threads.so
examples/imguFit: /usr/lib/libfftw3.so
examples/imguFit: /usr/lib/libosg.so
examples/imguFit: /usr/lib/libosgFX.so
examples/imguFit: /usr/lib/libosgViewer.so
examples/imguFit: /usr/lib/libosgText.so
examples/imguFit: /usr/lib/libosgGA.so
examples/imguFit: /usr/lib/libosgDB.so
examples/imguFit: /usr/lib/libOpenThreads.so
examples/imguFit: /usr/lib/libosgUtil.so
examples/imguFit: src/libmatrixmath.so
examples/imguFit: examples/CMakeFiles/imguFit.dir/build.make
examples/imguFit: examples/CMakeFiles/imguFit.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable imguFit"
	cd /home/jon/lighttwist/imgu/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imguFit.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/imguFit.dir/build: examples/imguFit
.PHONY : examples/CMakeFiles/imguFit.dir/build

examples/CMakeFiles/imguFit.dir/requires: examples/CMakeFiles/imguFit.dir/imguFit.o.requires
.PHONY : examples/CMakeFiles/imguFit.dir/requires

examples/CMakeFiles/imguFit.dir/clean:
	cd /home/jon/lighttwist/imgu/examples && $(CMAKE_COMMAND) -P CMakeFiles/imguFit.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/imguFit.dir/clean

examples/CMakeFiles/imguFit.dir/depend:
	cd /home/jon/lighttwist/imgu && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jon/lighttwist/imgu /home/jon/lighttwist/imgu/examples /home/jon/lighttwist/imgu /home/jon/lighttwist/imgu/examples /home/jon/lighttwist/imgu/examples/CMakeFiles/imguFit.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/imguFit.dir/depend
