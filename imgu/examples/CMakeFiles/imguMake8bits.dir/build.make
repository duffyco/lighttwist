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
include examples/CMakeFiles/imguMake8bits.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/imguMake8bits.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/imguMake8bits.dir/flags.make

examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o: examples/CMakeFiles/imguMake8bits.dir/flags.make
examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o: examples/imguMake8bits.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jon/lighttwist/imgu/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/imguMake8bits.dir/imguMake8bits.o   -c /home/jon/lighttwist/imgu/examples/imguMake8bits.c

examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/imguMake8bits.dir/imguMake8bits.i"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /home/jon/lighttwist/imgu/examples/imguMake8bits.c > CMakeFiles/imguMake8bits.dir/imguMake8bits.i

examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/imguMake8bits.dir/imguMake8bits.s"
	cd /home/jon/lighttwist/imgu/examples && /usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /home/jon/lighttwist/imgu/examples/imguMake8bits.c -o CMakeFiles/imguMake8bits.dir/imguMake8bits.s

examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.requires:
.PHONY : examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.requires

examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.provides: examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.requires
	$(MAKE) -f examples/CMakeFiles/imguMake8bits.dir/build.make examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.provides.build
.PHONY : examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.provides

examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.provides.build: examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o

# Object files for target imguMake8bits
imguMake8bits_OBJECTS = \
"CMakeFiles/imguMake8bits.dir/imguMake8bits.o"

# External object files for target imguMake8bits
imguMake8bits_EXTERNAL_OBJECTS =

examples/imguMake8bits: examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o
examples/imguMake8bits: src/libimgu.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libpng.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libz.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libjpeg.so
examples/imguMake8bits: /usr/lib/libnetpbm.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libavcodec.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libavutil.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libavformat.so
examples/imguMake8bits: /usr/lib/i386-linux-gnu/libswscale.so
examples/imguMake8bits: /usr/lib/libfftw3_threads.so
examples/imguMake8bits: /usr/lib/libfftw3.so
examples/imguMake8bits: /usr/lib/libosg.so
examples/imguMake8bits: /usr/lib/libosgFX.so
examples/imguMake8bits: /usr/lib/libosgViewer.so
examples/imguMake8bits: /usr/lib/libosgText.so
examples/imguMake8bits: /usr/lib/libosgGA.so
examples/imguMake8bits: /usr/lib/libosgDB.so
examples/imguMake8bits: /usr/lib/libOpenThreads.so
examples/imguMake8bits: /usr/lib/libosgUtil.so
examples/imguMake8bits: src/libmatrixmath.so
examples/imguMake8bits: examples/CMakeFiles/imguMake8bits.dir/build.make
examples/imguMake8bits: examples/CMakeFiles/imguMake8bits.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable imguMake8bits"
	cd /home/jon/lighttwist/imgu/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imguMake8bits.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/imguMake8bits.dir/build: examples/imguMake8bits
.PHONY : examples/CMakeFiles/imguMake8bits.dir/build

examples/CMakeFiles/imguMake8bits.dir/requires: examples/CMakeFiles/imguMake8bits.dir/imguMake8bits.o.requires
.PHONY : examples/CMakeFiles/imguMake8bits.dir/requires

examples/CMakeFiles/imguMake8bits.dir/clean:
	cd /home/jon/lighttwist/imgu/examples && $(CMAKE_COMMAND) -P CMakeFiles/imguMake8bits.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/imguMake8bits.dir/clean

examples/CMakeFiles/imguMake8bits.dir/depend:
	cd /home/jon/lighttwist/imgu && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jon/lighttwist/imgu /home/jon/lighttwist/imgu/examples /home/jon/lighttwist/imgu /home/jon/lighttwist/imgu/examples /home/jon/lighttwist/imgu/examples/CMakeFiles/imguMake8bits.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/imguMake8bits.dir/depend

