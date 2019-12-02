# - Find CMINPACK
# Find the native CMINPACK includes and library
# This module defines
#  CMINPACK_INCLUDE_DIR, where to find jpeglib.h, etc.
#  CMINPACK_LIBRARIES, the libraries needed to use CMINPACK.
#  CMINPACK_FOUND, If false, do not try to use CMINPACK.
# also defined, but not for general use are
#  CMINPACK_LIBRARY, where to find the CMINPACK library.

FIND_PATH(CMINPACK_INCLUDE_DIR cminpack.h PATHS /usr/local/include /usr/include PATH_SUFFIXES cminpack cminpack-1 )

SET(CMINPACK_NAMES ${CMINPACK_NAMES} minpack cminpack)
FIND_LIBRARY(CMINPACK_LIBRARY
  NAMES ${CMINPACK_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (CMINPACK_LIBRARY AND CMINPACK_INCLUDE_DIR)
    SET(CMINPACK_LIBRARIES ${CMINPACK_LIBRARY})
    SET(CMINPACK_FOUND "YES")
ELSE (CMINPACK_LIBRARY AND CMINPACK_INCLUDE_DIR)
  SET(CMINPACK_FOUND "NO")
ENDIF (CMINPACK_LIBRARY AND CMINPACK_INCLUDE_DIR)


IF (CMINPACK_FOUND)
   IF (NOT CMINPACK_FIND_QUIETLY)
      MESSAGE(STATUS "Found CMINPACK: ${CMINPACK_LIBRARIES}")
   ENDIF (NOT CMINPACK_FIND_QUIETLY)
ELSE (CMINPACK_FOUND)
   IF (CMINPACK_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CMINPACK library")
   ENDIF (CMINPACK_FIND_REQUIRED)
ENDIF (CMINPACK_FOUND)

# Deprecated declarations.
SET (NATIVE_CMINPACK_INCLUDE_PATH ${CMINPACK_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_CMINPACK_LIB_PATH ${CMINPACK_LIBRARY} PATH)

MARK_AS_ADVANCED(
  CMINPACK_LIBRARY
  CMINPACK_INCLUDE_DIR
  )

