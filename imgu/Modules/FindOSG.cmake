# Include the local modules directory

if( NOT OSG_FOUND)

  # Locate LibOSG files


  SET(OSG_PATH_SUFFIXE "" CACHE STRING "repertoire contenant osg")
  MARK_AS_ADVANCED( OSG_PATH_SUFFIXE )

  set(OSG_LIB_DIR "lib")
  
  find_path( OSG_INCLUDE_DIR
	osg/Version
    PATH_SUFFIXES osg ${OSG_PATH_SUFFIXE}/include/
    )
  MARK_AS_ADVANCED( OSG_INCLUDE_DIR)
    
  find_library( OSG_LIBRARY_OSG
    osg
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/lib
    )
  if (NOT OSG_LIBRARY_OSG)
      find_library( OSG_LIBRARY_OSG
        osg
        PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/lib ${OSG_PATH_SUFFIXE}/lib64
        )
  endif (NOT OSG_LIBRARY_OSG)

  MARK_AS_ADVANCED( OSG_LIBRARY_OSG)

  find_library( OSG_LIBRARY_GA
    osgGA
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_GA )

  find_library( OSG_LIBRARY_DB
    osgDB
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_DB )

  find_library( OSG_LIBRARY_TEXT
    osgText
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_TEXT )

  find_library( OSG_LIBRARY_VIEWER
    osgViewer
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_VIEWER )

  find_library( OSG_LIBRARY_FX
    osgFX
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_FX )

  find_library( OSG_LIBRARY_UTIL
    osgUtil
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_UTIL )

  find_library( OSG_LIBRARY_OPENTHREADS
    OpenThreads
    PATH_SUFFIXES ${OSG_PATH_SUFFIXE}/${OSG_LIB_DIR}
    )
  MARK_AS_ADVANCED( OSG_LIBRARY_OPENTHREADS )


  if (OSG_LIBRARY_OSG)
    get_filename_component( OSG_LIBRARY_DIR ${OSG_LIBRARY_OSG} PATH )
    MARK_AS_ADVANCED( OSG_LIBRARY_DIR)
  endif (OSG_LIBRARY_OSG)
  
  if( OSG_INCLUDE_DIR AND OSG_LIBRARY_OSG AND OSG_LIBRARY_FX AND OSG_LIBRARY_VIEWER AND OSG_LIBRARY_TEXT 
      AND OSG_LIBRARY_GA AND OSG_LIBRARY_DB AND OSG_LIBRARY_OPENTHREADS AND OSG_LIBRARY_UTIL)
    set( OSG_FOUND TRUE)
    set( OSG_INCLUDE_DIRS ${OSG_INCLUDE_DIR})
    set( OSG_LIBRARIES ${OSG_LIBRARY_OSG} ${OSG_LIBRARY_FX} ${OSG_LIBRARY_VIEWER} ${OSG_LIBRARY_TEXT} ${OSG_LIBRARY_GA} ${OSG_LIBRARY_DB} ${OSG_LIBRARY_OPENTHREADS} ${OSG_LIBRARY_UTIL})
    set( OSG_LIBRARY_DIRS ${OSG_LIBRARY_DIR})
  endif()
  
  if( OSG_FOUND)
    if( NOT OSG_FIND_QUIETLY)
      message( STATUS "Found OSG library: ${OSG_LIBRARIES}")
      message( STATUS "Found OSG library dirs: ${OSG_LIBRARY_DIRS}")
      message( STATUS "Found OSG inc dirs: ${OSG_INCLUDE_DIRS}")
    endif( NOT OSG_FIND_QUIETLY)
  else( OSG_FOUND)
    if( OSG_FIND_REQUIRED)
      message( FATAL_ERROR "Could not find OSG")
    else( OSG_FIND_REQUIRED)
      if( NOT OSG_FIND_QUIETLY)
	message( STATUS "Could not find OSG")
      endif( NOT OSG_FIND_QUIETLY)
    endif( OSG_FIND_REQUIRED)
  endif( OSG_FOUND)
  
endif( NOT OSG_FOUND)