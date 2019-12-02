# - Find BMC
# Find the native BMC includes and library
# This module defines
#  BMC_INCLUDE_DIR, where to find bimulticast.h, ...
#  BMC_LIBRARIES, the libraries needed (libbmc)
#  BMC_FOUND, If false, do not try to use V4L.
# also defined, but not for general use are
#  BMC_LIBRARY, where to find the BMC library.

# must find at least of one these header files
FIND_PATH(BMC_INCLUDE_DIR 
  NAMES bimulticast.h multicast.h stdincast.h udpcast.h
  PATHS /usr/local/include /usr/include
  PATH_SUFFIXES bmc
)

FIND_LIBRARY(BMC_LIBRARY
  NAMES bmc
  PATHS /usr/lib /usr/local/lib
)

IF (BMC_INCLUDE_DIR AND BMC_LIBRARY)
    SET(BMC_FOUND "YES")
    SET(BMC_LIBRARIES ${BMC_LIBRARY})
ELSE (BMC_INCLUDE_DIR AND BMC_LIBRARY)
  SET(BMC_FOUND "NO")
ENDIF (BMC_INCLUDE_DIR AND BMC_LIBRARY)


IF (BMC_FOUND)
   IF (NOT BMC_FIND_QUIETLY)
      MESSAGE(STATUS "Found BMC: ${BMC_LIBRARIES}")
   ENDIF (NOT BMC_FIND_QUIETLY)
ELSE (BMC_FOUND)
   IF (BMC_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find BMC library")
   ENDIF (BMC_FIND_REQUIRED)
ENDIF (BMC_FOUND)

# Deprecated declarations.
SET (NATIVE_BMC_INCLUDE_PATH ${BMC_INCLUDE_DIR} )

MARK_AS_ADVANCED(
  BMC_LIBRARY
  BMC_INCLUDE_DIR
)

