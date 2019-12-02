# - Find Doxygen
# Find the native V4L includes and library
# This module defines
#  DOXYGEN, the program (empty if not found)
# DOXYGEN_FOUND at yes or no

#should we also check for linux/videodev.h?
FIND_PROGRAM(DOXYGEN doxygen /usr/bin /usr/local/bin)

IF (DOXYGEN)
    SET(DOXYGEN_FOUND "YES")
ELSE (DOXYGEN)
  SET(DOXYGEN_FOUND "NO")
ENDIF (DOXYGEN)

IF (DOXYGEN_FOUND)
      MESSAGE(STATUS "Found Doxygen: ${DOXYGEN}")
ELSE(DOXYGEN_FOUND)
      MESSAGE(STATUS "Could not find Doxygen")
ENDIF (DOXYGEN_FOUND)

MARK_AS_ADVANCED(
  DOXYGEN_INCLUDE_DIR
  DOXYGEN
)


