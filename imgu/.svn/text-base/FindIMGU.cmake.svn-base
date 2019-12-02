# - Find IMGU
# Find the native IMGU includes and library
# This module defines
#  IMGU_INCLUDE_DIR, where to find jpeglib.h, etc.
#  IMGU_LIBRARIES, the libraries needed to use IMGU.
#  IMGU_FOUND, If false, do not try to use IMGU.
# also defined, but not for general use are
#  IMGU_LIBRARY, where to find the IMGU library.

FIND_PATH(IMGU_INCLUDE_DIR imgu/imgu.h
/usr/local/include
/usr/include
)

SET(IMGU_NAMES ${IMGU_NAMES} imgu)
FIND_LIBRARY(IMGU_LIBRARY
  NAMES ${IMGU_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (IMGU_LIBRARY AND IMGU_INCLUDE_DIR)
    SET(IMGU_LIBRARIES ${IMGU_LIBRARY})
    SET(IMGU_FOUND "YES")
ELSE (IMGU_LIBRARY AND IMGU_INCLUDE_DIR)
  SET(IMGU_FOUND "NO")
ENDIF (IMGU_LIBRARY AND IMGU_INCLUDE_DIR)


IF (IMGU_FOUND)
   IF (NOT IMGU_FIND_QUIETLY)
      MESSAGE(STATUS "Found IMGU: ${IMGU_LIBRARIES}")
   ENDIF (NOT IMGU_FIND_QUIETLY)
ELSE (IMGU_FOUND)
   IF (IMGU_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find IMGU library")
   ENDIF (IMGU_FIND_REQUIRED)
ENDIF (IMGU_FOUND)

# Deprecated declarations.
SET (NATIVE_IMGU_INCLUDE_PATH ${IMGU_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_IMGU_LIB_PATH ${IMGU_LIBRARY} PATH)

MARK_AS_ADVANCED(
  IMGU_LIBRARY
  IMGU_INCLUDE_DIR
  )

