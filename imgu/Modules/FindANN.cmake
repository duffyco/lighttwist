# - Find ANN
# Find the native ANN includes and library
# This module defines
#  ANN_INCLUDE_DIR, where to find jpeglib.h, etc.
#  ANN_LIBRARIES, the libraries needed to use ANN.
#  ANN_FOUND, If false, do not try to use ANN.
# also defined, but not for general use are
#  ANN_LIBRARY, where to find the ANN library.

FIND_PATH(ANN_INCLUDE_DIR ANN/ANN.h PATHS /usr/local/include /usr/include)

SET(ANN_NAMES ${ANN_NAMES} ANN ann)
FIND_LIBRARY(ANN_LIBRARY
  NAMES ${ANN_NAMES}
  PATHS /usr/local/lib /usr/lib
  )

IF (ANN_LIBRARY AND ANN_INCLUDE_DIR)
    SET(ANN_LIBRARIES ${ANN_LIBRARY})
    SET(ANN_FOUND "YES")
ELSE (ANN_LIBRARY AND ANN_INCLUDE_DIR)
  SET(ANN_FOUND "NO")
ENDIF (ANN_LIBRARY AND ANN_INCLUDE_DIR)


IF (ANN_FOUND)
   IF (NOT ANN_FIND_QUIETLY)
      MESSAGE(STATUS "Found ANN: ${ANN_LIBRARIES}")
   ENDIF (NOT ANN_FIND_QUIETLY)
ELSE (ANN_FOUND)
   IF (ANN_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ANN library")
   ENDIF (ANN_FIND_REQUIRED)
ENDIF (ANN_FOUND)

# Deprecated declarations.
SET (NATIVE_ANN_INCLUDE_PATH ${ANN_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_ANN_LIB_PATH ${ANN_LIBRARY} PATH)

MARK_AS_ADVANCED(
  ANN_LIBRARY
  ANN_INCLUDE_DIR
  )

