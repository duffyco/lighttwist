# - Find PT
# Find the native PT includes and library
# This module defines
#  PT_INCLUDE_DIR, where to find pt.h, etc.
#  PT_LIBRARIES, the libraries needed to use PT.
#  PT_FOUND, If false, do not try to use PT.
# also defined, but not for general use are
#  PT_LIBRARY, where to find the PT library.

FIND_PATH(PT_INCLUDE_DIR pt.h
/usr/local/include
/usr/include
)

SET(PT_NAMES ${PT_NAMES} pt)
FIND_LIBRARY(PT_LIBRARY
  NAMES ${PT_NAMES}
  PATHS /usr/lib /usr/local/lib
  )

IF (PT_LIBRARY AND PT_INCLUDE_DIR)
    SET(PT_LIBRARIES ${PT_LIBRARY})
    SET(PT_FOUND "YES")
ELSE (PT_LIBRARY AND PT_INCLUDE_DIR)
  SET(PT_FOUND "NO")
ENDIF (PT_LIBRARY AND PT_INCLUDE_DIR)


IF (PT_FOUND)
   IF (NOT PT_FIND_QUIETLY)
      MESSAGE(STATUS "Found PT: ${PT_LIBRARIES}")
   ENDIF (NOT PT_FIND_QUIETLY)
ELSE (PT_FOUND)
   IF (PT_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PT library")
   ENDIF (PT_FIND_REQUIRED)
ENDIF (PT_FOUND)

# Deprecated declarations.
SET (NATIVE_PT_INCLUDE_PATH ${PT_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_PT_LIB_PATH ${PT_LIBRARY} PATH)

MARK_AS_ADVANCED(
  PT_LIBRARY
  PT_INCLUDE_DIR
)

