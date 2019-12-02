SET(CMAKE_INCLUDE_PATH /opt/local/include;/usr/local/include;/usr/include)
SET(CMAKE_LIBRARY_PATH /opt/local/lib;/usr/local/lib;/usr/lib)
SET(CMAKE_PROGRAM_PATH /opt/local/bin;/usr/local/bin;/usr/bin)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)

# voir cmake --help-policy CMP0003
cmake_policy(SET CMP0003 NEW)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Modules/")

#
# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their definition
# and dump it in the cache along with proper documentation, otherwise set CMAKE_BUILD_TYPE
# to Debug prior to calling PROJECT()
#
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Buid: None, Debug, Release, RelWithDebInfo, MinSizeRel")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Build: None Debug Release RelWithDebInfo MinSizeRel")
ENDIF()


project(IMGU)
set(IMGU_VERSION_MAJOR "2")
set(IMGU_VERSION_MINOR "1")
set(IMGU_VERSION_PATCH "0")
set(IMGU_VERSION "${IMGU_VERSION_MAJOR}.${IMGU_VERSION_MINOR}")

OPTION(IMGU_LIB_STATIC "Build static library" OFF)
OPTION(IMGU_LIB_8 "Build 8bits library" ON)
OPTION(IMGU_EXTRA "Build extra OSG functions" ON)
OPTION(IMGU_EXAMPLES "Build imgu examples" ON)
OPTION(IMGU_APPS "Build imgu applications" ON)
OPTION(IMGU_WARN "Add more compilation warnings" ON)
OPTION(IMGU_DOCUMENTATION "Documentation (doxygen)" ON)

#add definitions here (can be used in c code)

set(IMGU_BIN bin)
set(IMGU_LIB lib)
set(IMGU_INCLUDE include/imgu)
set(IMGU_SHARE share/imgu)

set(MATRIX_LIBS m)
set(IMGU_LIBS stdc++)

# Pour ne pas que les variables globales causent problemes avec
# les librairies dynamiques en MacOSX sur PPC
if(APPLE)
  if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "powerpc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common")        
  endif()
endif()


#if the O2 option is added afterward the IMGU_WARN option (set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O2")), then cmake adds a ';' that make the compiler crash
#for now, just set flags once and IMGU_WARN is ignored
#if(IMGU_WARN)
	# -Wno-write-strings is to remove "deprecated conversion from string constant to ‘char*’"
	# -Wconversion -Wwrite-strings -Wno-empty-body -Wshadow
	# -fshort-nums
    set(CMAKE_CXX_FLAGS "-g -O2 -Wall -Wno-write-strings")
    set(CMAKE_C_FLAGS "-g -O2 -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Wcast-align -Wnested-externs -fno-common -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable")
#endif(IMGU_WARN)

#-----------------------------------------------------------------------------
### dependency check
#-----------------------------------------------------------------------------
find_package(PNG)
if(PNG_FOUND)
  add_definitions(-DHAVE_LIBPNG)
  set(IMGU_LIBS ${IMGU_LIBS} ${PNG_LIBRARIES})
  include_directories(${PNG_INCLUDE_DIR})
  #add_definitions(${PNG_DEFINITIONS})
  link_directories(${PNG_LIBRARY})
else()
  #message(FATAL_ERROR "Cannot build without PNG")
  message("Could not find libpng12-dev")
endif()


find_package(ZLIB)
if(ZLIB_FOUND)
	add_definitions(-DHAVE_ZLIB)
	set(IMGU_LIBS ${IMGU_LIBS} ${ZLIB_LIBRARIES})
	include_directories(${ZLIB_INCLUDE_DIR})
	link_directories(${ZLIB_LIBRARY})
else()
  message("could not find zlib")
endif()

find_package(JPEG)
if(JPEG_FOUND)
  set(HAVE_JPEG 1)
  set(IMGU_LIBS ${IMGU_LIBS} ${JPEG_LIBRARIES})
  include_directories(${JPEG_INCLUDE_DIR})
  link_directories(${JPEG_LIBRARY})
else()
  #message(FATAL_ERROR "Cannot build without JPEG")
  message("Could not find JPEG (package libjpeg62-dev)")
endif()

find_package(NETPBM)
if(NETPBM_FOUND)
  set(HAVE_NETPBM 1)
  set(IMGU_LIBS ${IMGU_LIBS} ${NETPBM_LIBRARIES})
  include_directories(${NETPBM_INCLUDE_DIR})
  link_directories(${NETPBM_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without NETPBM")
  message("Could not find libnetpbm10-dev")
endif()

find_package(CMINPACK)
if(CMINPACK_FOUND)
  add_definitions(-DCMINPACK)
  set(MATRIX_LIBS ${MATRIX_LIBS} ${CMINPACK_LIBRARY} )
  include_directories(${CMINPACK_INCLUDE_DIR})
  link_directories(${CMINPACK_LINK_DIRECTORIES})
  message("Found CMINPACK: ${CMINPACK_LIBRARIES}")
else()
  #message(FATAL_ERROR "Cannot build without CMinpack")
  message("Could not find cminpack")
endif()

# this will also insure that pthread is there...
find_package(RQUEUE)
if(RQUEUE_FOUND)
  set(HAVE_RQUEUE 1)
  set(IMGU_LIBS ${IMGU_LIBS} ${RQUEUE_LIBRARIES})
  include_directories(${RQUEUE_INCLUDE_DIR})
  link_directories(${RQUEUE_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without pthread and rqueue.")
  message("Could not find rqueue. Building without thread support.")
endif()


find_package(PROSILICA)
if(PROSILICA_FOUND)
  add_definitions(-DHAVE_GIGE -D_LINUX -D_x86)
  set(IMGU_LIBS ${IMGU_LIBS} PvAPI)
  include_directories(${PROSILICA_INCLUDE_DIR})
  link_directories(${PROSILICA_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without Prosilica")
  message("Could not find prosilica")
endif()

find_package(V4L)
if(V4L_FOUND)
  add_definitions(-DHAVE_V4L)
  include_directories(${V4L_INCLUDE_DIR})
  link_directories(${V4L_LINK_DIRECTORIES})
else()
  #message(FATAL_ERROR "Cannot build without v4l")
  message("Could not find v4l")
endif()

find_package(OSG)
if(OSG_FOUND)
  set(HAVE_OSG 1)
  include_directories(${OSG_INCLUDE_DIR})
  link_directories(${OSG_LINK_DIRECTORIES})
	message("osg include dir is ${OSG_INCLUDE_DIR}")
else()
  #message(FATAL_ERROR "Cannot build without OSG")
  message("Could not find OSG (need libopenscenegraph-dev)")
endif()

#set(OpenCV_DIR /usr)
find_package(OpenCV)
if(OpenCV_FOUND)
  add_definitions(-DHAVE_OPENCV)

  if (APPLE)
    set(IMGU_LIBS ${IMGU_LIBS} ${OpenCV_LIBS})
    message("-- Found OpenCV libs: ${OpenCV_LIBS}")
  else()
    set(IMGU_LIBS ${IMGU_LIBS} ${OpenCV_LIBS})
    message("-- Found OpenCV libs: ${OpenCV_LIBS}")
    #set(IMGU_LIBS ${IMGU_LIBS} ${OpenCV_LIBRARIES})
    #message("-- Found OpenCV libs: ${OpenCV_LIBRARIES}")
  endif()

  include_directories(${OpenCV_INCLUDE_DIRS})
  link_directories(${OpenCV_LINK_DIRECTORIES})
  #message("found OpenCV (libs): ${OpenCV_LIBS}")
  #message("found OpenCV (link dirs): ${OpenCV_LINK_DIRECTORIES}")
  #message("found OpenCV (include dirs): ${OpenCV_INCLUDE_DIRS}")
else()
  message("Could not find OpenCV")
endif()

if(IMGU_EXTRA)
  if(OSG_FOUND)
	set(HAVE_IMGU_EXTRA 1)
  endif(OSG_FOUND)
endif(IMGU_EXTRA)

find_package(BMC)
if(BMC_FOUND)
  set(HAVE_BMC 1)
  set(IMGU_LIBS ${IMGU_LIBS} ${BMC_LIBRARIES})
  include_directories(${BMC_INCLUDE_DIR})
  link_directories(${BMC_LINK_DIRECTORIES})
  message("BMC libs is ${BMC_LIBRARIES}")
else()
  message("Could not find bmc (bimulticast)")
endif()

find_package(PT)
if(PT_FOUND)
  set(HAVE_PT 1)
  set(IMGU_LIBS ${IMGU_LIBS} pt)
  include_directories(${PT_INCLUDE_DIR})
  link_directories(${PT_LINK_DIRECTORIES})
else()
  message("Could not find pt (pantilt control)")
endif()

find_package(Epiphan)
if(EPIPHAN_FOUND)
  add_definitions(-DHAVE_EPIPHAN)
  set(IMGU_LIBS ${IMGU_LIBS} frmgrab slava)
  include_directories(${EPIPHAN_INCLUDE_DIR})
  link_directories(${EPIPHAN_LINK_DIRECTORIES})
else()
  message("Could not find Epiphan frame grabber API")
endif()

find_package(BlackMagic)
if(BLACKMAGIC_FOUND)
  add_definitions(-DHAVE_BLACKMAGIC)
  set(IMGU_LIBS ${IMGU_LIBS} ${BLACKMAGIC_LIBRARIES})
  include_directories(${BLACKMAGIC_INCLUDE_DIR})
  link_directories(${BLACKMAGIC_LINK_DIRECTORIES})
else()
  message("Could not find BlackMagic frame grabber API")
endif()

# BLACKMAGIC
# set(IMGU_LIBS ${IMGU_LIBS} decklink)

# this package is legacy. It will disapear soon. Use PROFILOMETRE instead.
#find_package(PROFILER)
#if(PROFILER_FOUND)
#  add_definitions(-DHAVE_PROFILER)
#  set(IMGU_LIBS ${IMGU_LIBS} profiler)
#  include_directories(${PROFILER_INCLUDE_DIR})
#  link_directories(${PROFILER_LINK_DIRECTORIES})
#else()
#  message("Could not find profiler")
#endif()


find_package(PROFILOMETRE)
if(PROFILOMETRE_FOUND)
  set(HAVE_PROFILOMETRE 1)
  set(IMGU_LIBS ${IMGU_LIBS} ${PROFILOMETRE_LIBRARIES})
  include_directories(${PROFILOMETRE_INCLUDE_DIR})
  link_directories(${PROFILOMETRE_LINK_DIRECTORIES})
else()
  message("Could not find profilometre")
endif()

find_package(ANN)
if(ANN_FOUND)
  add_definitions(-DHAVE_ANN)
  set(IMGU_LIBS ${IMGU_LIBS} ${ANN_LIBRARIES})
  include_directories(${ANN_INCLUDE_DIR})
  link_directories(${ANN_LINK_DIRECTORIES})
else()
  message("Could not find ANN library")
endif()


find_package(FFMPEG)
if(FFMPEG_FOUND)
  add_definitions(-DHAVE_AVUTIL)
  add_definitions(-DHAVE_AVCODEC)
  add_definitions(-DHAVE_AVFORMAT)
  add_definitions(-DHAVE_SWSCALE)
  add_definitions(-DHAVE_FFMPEG)
  set(IMGU_LIBS ${IMGU_LIBS} ${FFMPEG_LIBRARIES})
  #set(IMGU_LIBS ${IMGU_LIBS} avcodec avcore avdevice avfilter avformat avutil)
  include_directories(${FFMPEG_INCLUDE_DIRS})
  link_directories(${FFMPEG_LIBRARY_DIRS})
  #message("FFMPEG_LIBRARIES is ${FFMPEG_LIBRARIES}")
else()
  message("Could not find ffmpeg: no video playback")
endif()

find_package(FFTW3 COMPONENTS double threads)
if(FFTW3_FOUND)
  set(HAVE_FFTW3 1)
  set(IMGU_LIBS ${IMGU_LIBS} ${FFTW3_LIBRARIES})
  include_directories(${FFTW3_INCLUDE_DIR})
  #link_directories(${FFTW3_LINK_DIRECTORIES})
  message("-- Found FFT3W: ${FFTW3_LIBRARIES}")
else()
  message("Could not find FFTW3 library")
endif()


find_package(liblo)
if(LIBLO_FOUND)
#  set(HAVE_LIBLO 1)
#  add_definitions(-DHAVE_LIBLO)
#  set(IMGU_LIBS ${IMGU_LIBS} frmgrab slava)
  include_directories(${LIBLO_INCLUDE_DIR})
#  link_directories(${LIBLO_LINK_DIRECTORIES})
else()
  message("Could not find Epiphan frame grabber API")
endif()
#-----------------------------------------------------------------------------
### lib and path setup
#-----------------------------------------------------------------------------
include_directories(${IMGU_SOURCE_DIR} ${IMGU_SOURCE_DIR}/src ${IMGU_SOURCE_DIR}/src/libs ${IMGU_SOURCE_DIR}/src/extra ${IMGU_SOURCE_DIR}/src/gpu /usr/local/include)
set(IMGU_LIBS ${IMGU_LIBS} ${MATRIX_LIBS})

if(${OSG_LIBRARY_DIRS} MATCHES "/Library/Frameworks")
  set(OSG_LIBS "-framework osg -framework osgGA -framework osgViewer -framework osgText -framework OpenThreads -framework osgDB -framework osgUtil")
else()
  set(OSG_LIBS osg osgGA osgViewer osgText)
endif()

#
# generate include files
#

CONFIGURE_FILE(${IMGU_SOURCE_DIR}/src/imgu.h.in ${IMGU_SOURCE_DIR}/src/imgu.h)
CONFIGURE_FILE(${IMGU_SOURCE_DIR}/src/param.h.in ${IMGU_SOURCE_DIR}/src/param.h)

#-----------------------------------------------------------------------------
### subdirectories
#-----------------------------------------------------------------------------

add_subdirectory (src)
add_subdirectory (apps)
add_subdirectory (examples)
add_subdirectory (data)

#-----------------------------------------------------------------------------
### for packaging
#-----------------------------------------------------------------------------
set(CPACK_PACKAGE_VERSION ${IMGU_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${IMGU_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${IMGU_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${IMGU_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IMGU image library")
set(CPACK_PACKAGE_VENDOR "University of Montreal")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "imgu-${IMGU_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "${IMGU_SOURCE_DIR}/test/")
include(CPack)

#-----------------------------------------------------------------------------
### for uninstallation
#-----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P
"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


#
# Documentation doxygen
#
FIND_PACKAGE(Doxygen)
IF(IMGU_DOCUMENTATION)
    IF(DOXYGEN_FOUND)
	  ADD_CUSTOM_TARGET(doc ${DOXYGEN} ${IMGU_SOURCE_DIR}/Doxyfile)
    ELSE(DOXYGEN_FOUND)
	  #message(FATAL_ERROR "Cannot build documentation without doxygen")
    ENDIF(DOXYGEN_FOUND)
ENDIF(IMGU_DOCUMENTATION)


#-----------------------------------------------------------------------------
### installs FindIMGU.cmake to cmake Modules directory
#-----------------------------------------------------------------------------

install(FILES FindIMGU.cmake DESTINATION ${CMAKE_ROOT}/Modules)

#-----------------------------------------------------------------------------





