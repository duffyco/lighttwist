# - Find V4L
# Find the native V4L includes and library
# This module defines
#  V4L_INCLUDE_DIR, where to find jpeglib.h, etc.
#  V4L_LIBRARIES, the libraries needed to use V4L.
#  V4L_FOUND, If false, do not try to use V4L.
# also defined, but not for general use are
#  V4L_LIBRARY, where to find the V4L library.

#should we also check for linux/videodev.h?
FIND_PATH(V4L_INCLUDE_DIR linux/videodev2.h
/usr/local/include
/usr/include
)

IF (V4L_INCLUDE_DIR)
    SET(V4L_FOUND "YES")
ELSE (V4L_INCLUDE_DIR)
  SET(V4L_FOUND "NO")
ENDIF (V4L_INCLUDE_DIR)


IF (V4L_FOUND)
   IF (NOT V4L_FIND_QUIETLY)
      MESSAGE(STATUS "Found V4L: ${V4L_LIBRARIES}")
   ENDIF (NOT V4L_FIND_QUIETLY)
ELSE (V4L_FOUND)
   IF (V4L_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find V4L library")
   ENDIF (V4L_FIND_REQUIRED)
ENDIF (V4L_FOUND)

# Deprecated declarations.
SET (NATIVE_V4L_INCLUDE_PATH ${V4L_INCLUDE_DIR} )

MARK_AS_ADVANCED(
  V4L_INCLUDE_DIR
  )

