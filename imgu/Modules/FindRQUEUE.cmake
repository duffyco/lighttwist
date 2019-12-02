# - Find RQUEUE
# Find the native RQUEUE includes and library
# This module defines
#  RQUEUE_INCLUDE_DIR, where to find jpeglib.h, etc.
#  RQUEUE_LIBRARIES, the libraries needed to use RQUEUE.
#  RQUEUE_FOUND, If false, do not try to use RQUEUE.
# also defined, but not for general use are
#  RQUEUE_LIBRARY, where to find the RQUEUE library.

FIND_PATH(RQUEUE_INCLUDE_DIR rqueue.h
/usr/local/include
/usr/include
)

SET(RQUEUE_NAMES ${RQUEUE_NAMES} rqueue)
FIND_LIBRARY(RQUEUE_LIBRARY
  NAMES ${RQUEUE_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (RQUEUE_LIBRARY AND RQUEUE_INCLUDE_DIR)
    SET(RQUEUE_LIBRARIES ${RQUEUE_LIBRARY} pthread)
    SET(RQUEUE_FOUND "YES")
ELSE (RQUEUE_LIBRARY AND RQUEUE_INCLUDE_DIR)
  SET(RQUEUE_FOUND "NO")
ENDIF (RQUEUE_LIBRARY AND RQUEUE_INCLUDE_DIR)


IF (RQUEUE_FOUND)
   IF (NOT RQUEUE_FIND_QUIETLY)
      MESSAGE(STATUS "Found RQUEUE: ${RQUEUE_LIBRARIES}")
   ENDIF (NOT RQUEUE_FIND_QUIETLY)
ELSE (RQUEUE_FOUND)
   IF (RQUEUE_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find RQUEUE library")
   ENDIF (RQUEUE_FIND_REQUIRED)
ENDIF (RQUEUE_FOUND)

# Deprecated declarations.
SET (NATIVE_RQUEUE_INCLUDE_PATH ${RQUEUE_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_RQUEUE_LIB_PATH ${RQUEUE_LIBRARY} PATH)

MARK_AS_ADVANCED(
  RQUEUE_LIBRARY
  RQUEUE_INCLUDE_DIR
  )

