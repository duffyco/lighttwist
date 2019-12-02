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
include examples/CMakeFiles/imguMinGray.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/imguMinGray.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/imguMinGray.dir/flags.make

examples/CMakeFiles/imguMinGray.dir/imguMinGray.o: examples/CMakeFiles/imguMinGray.dir/flags.make
examples/CMakeFiles/imguMinGray.dir/imguMinGray.o: examples/imguMinGray.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jon/lighttwist/imgu/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/CMakeFiles/imguMinGray.dir/imguMinGray.o"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/imguMinGray.dir/imguMinGray.o   -c /home/jon/lighttwist/imgu/examples/imguMinGray.c

examples/CMakeFiles/imguMinGray.dir/imguMinGray.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/imguMinGray.dir/imguMinGray.i"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/jon/lighttwist/imgu/examples/imguMinGray.c > CMakeFiles/imguMinGray.dir/imguMinGray.i

examples/CMakeFiles/imguMinGray.dir/imguMinGray.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/imguMinGray.dir/imguMinGray.s"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/jon/lighttwist/imgu/examples/imguMinGray.c -o CMakeFiles/imguMinGray.dir/imguMinGray.s

examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.requires:
.PHONY : examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.requires

examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.provides: examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.requires
	$(MAKE) -f examples/CMakeFiles/imguMinGray.dir/build.make examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.provides.build
.PHONY : examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.provides

examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.provides.build: examples/CMakeFiles/imguMinGray.dir/imguMinGray.o

# Object files for target imguMinGray
imguMinGray_OBJECTS = \
"CMakeFiles/imguMinGray.dir/imguMinGray.o"

# External object files for target imguMinGray
imguMinGray_EXTERNAL_OBJECTS =

examples/imguMinGray: examples/CMakeFiles/imguMinGray.dir/imguMinGray.o
examples/imguMinGray: src/libimgu.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libpng.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libz.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libjpeg.so
examples/imguMinGray: /usr/lib/libnetpbm.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libavcodec.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libavutil.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libavformat.so
examples/imguMinGray: /usr/lib/i386-linux-gnu/libswscale.so
examples/imguMinGray: /usr/lib/libfftw3_threads.so
examples/imguMinGray: /usr/lib/libfftw3.so
examples/imguMinGray: /usr/lib/libosg.so
examples/imguMinGray: /usr/lib/libosgFX.so
examples/imguMinGray: /usr/lib/libosgViewer.so
examples/imguMinGray: /usr/lib/libosgText.so
examples/imguMinGray: /usr/lib/libosgGA.so
examples/imguMinGray: /usr/lib/libosgDB.so
examples/imguMinGray: /usr/lib/libOpenThreads.so
examples/imguMinGray: /usr/lib/libosgUtil.so
examples/imguMinGray: src/libmatrixmath.so
examples/imguMinGray: examples/CMakeFiles/imguMinGray.dir/build.make
examples/imguMinGray: examples/CMakeFiles/imguMinGray.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable imguMinGray"
	cd /home/jon/lighttwist/imgu/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imguMinGray.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/imguMinGray.dir/build: examples/imguMinGray
.PHONY : examples/CMakeFiles/imguMinGray.dir/build

examples/CMakeFiles/imguMinGray.dir/requires: examples/CMakeFiles/imguMinGray.dir/imguMinGray.o.requires
.PHONY : examples/CMakeFiles/imguMinGray.dir/requires

examples/CMakeFiles/imguMinGray.dir/clean:
	cd /home/jon/lighttwist/imgu/examples && $(CMAKE_COMMAND) -P CMakeFiles/imguMinGray.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/imguMinGray.dir/clean

examples/CMakeFiles/imguMinGray.dir/depend:
	cd /home/jon/lighttwist/imgu && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jon/lighttwist/imgu /home/jon/lighttwist/imgu/examples /home/jon/lighttwist/imgu /home/jon/lighttwist/imgu/examples /home/jon/lighttwist/imgu/examples/CMakeFiles/imguMinGray.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/imguMinGray.dir/depend

