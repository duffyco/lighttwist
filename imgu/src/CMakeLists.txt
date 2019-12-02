set(MATRIXMATH_SRC matrixmath.c libs/svd.c)
add_library (matrixmath SHARED ${MATRIXMATH_SRC})
if (IMGU_LIB_STATIC)
  add_library (matrixmath-static STATIC ${MATRIXMATH_SRC})
  set_target_properties(matrixmath-static PROPERTIES OUTPUT_NAME "matrixmath") 
endif()

set(IMGU_SRC imgu_core.c imgu_complex.c imgu_camera.c imgu_canny.c imgu_interpolate.c imgu_interpolate_view.c imgu_pyramid.c imgu_sequence.c imgu_matrix.c imgu_osg.c imgu_seam.cpp imgu_stereo.cpp imgu_sift.cpp imgu_draw.c imgu_args.c param.c libs/fftn.c libs/sift.cpp libs/graph.cpp libs/maxflow.cpp)
if(RQUEUE_FOUND)
	set(IMGU_SRC ${IMGU_SRC} imgu_recycle.c imgu_plugin.c)
    install(FILES imgu_recycle.h DESTINATION ${IMGU_INCLUDE})
	install(FILES imgu_plugin.h DESTINATION ${IMGU_INCLUDE})
endif(RQUEUE_FOUND)
 
if (BMC_FOUND)
  set(IMGU_SRC ${IMGU_SRC} imgu_stream.c)
  install(FILES imgu_stream.h DESTINATION ${IMGU_INCLUDE})
endif()


if(JPEG_FOUND)
	set(IMGU_SRC ${IMGU_SRC} imgu_jpeg.c)
	install(FILES imgu_jpeg.h DESTINATION ${IMGU_INCLUDE})
endif(JPEG_FOUND)

if(NETPBM_FOUND)
	set(IMGU_SRC ${IMGU_SRC} imgu_netpbm.c)
	install(FILES imgu_netpbm.h DESTINATION ${IMGU_INCLUDE})
endif(NETPBM_FOUND)

#
# imguextra, C++ viewer and OSG stuff
#

if(IMGU_EXTRA)
  if(OSG_FOUND)
   if(RQUEUE_FOUND)
    set(EXTRA_SRC extra/ImageViews.cpp  extra/IView.cpp  extra/ImguQueueTexture.cpp extra/ImageViewsEventManager.cpp  extra/pickmanager.cpp extra/annotations.cpp extra/parseShader.cpp)
    set(IMGU_SRC ${IMGU_SRC} ${EXTRA_SRC})
   endif(RQUEUE_FOUND)

    set(IMGU_LIBS ${IMGU_LIBS} ${OSG_LIBRARIES})
    install(FILES extra/imguextra.h extra/ImageViews.h extra/IView.h extra/ImguQueueTexture.h extra/ImageViewsEventManager.h extra/pickmanager.h extra/annotations.h extra/parseShader.h DESTINATION ${IMGU_INCLUDE})

    set(GPU_SRC gpu/Blur.cpp gpu/DistortionRadiale.cpp gpu/Distorsion.cpp gpu/Derivee.cpp gpu/FluxNormal.cpp gpu/gpuutils.cpp gpu/GPU.cpp gpu/Deform.cpp gpu/Layer.cpp gpu/Convolve.cpp)
    install(FILES gpu/GPU.h gpu/gpuutils.h gpu/Blur.h gpu/Distorsion.h gpu/DistortionRadiale.h plugins/disto-radiale.h gpu/Derivee.h gpu/FluxNormal.h gpu/Deform.h gpu/Layer.h gpu/Convolve.h DESTINATION ${IMGU_INCLUDE})
	set(IMGU_SRC ${IMGU_SRC} ${GPU_SRC})
  else(OSG_FOUND)
    #message(FATAL_ERROR "IMGU_EXTRA requires a working OpenSceneGraph.")
    message("IMGU_EXTRA requires a working OpenSceneGraph.")
  endif(OSG_FOUND)
endif(IMGU_EXTRA)



#####
##### Plugins
#####

#
# basic plugin information is here...
#
if(RQUEUE_FOUND)
  set(IMGU_SRC ${IMGU_SRC} plugins/allplugins.c)
  #
  # each plugin module goes here.
  #
  set(IMGU_SRC ${IMGU_SRC} plugins/dummy_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/group_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/merge_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/split_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/convolve_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/color_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/deinterlace_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/inverse_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/crop_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/blur_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/canny_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/load_camera.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/save_sink.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/pattern_camera.c plugins/stl.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/snapshot_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/scannereffect_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/frameblend_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/keying_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/sync_filter.c)
  set(IMGU_SRC ${IMGU_SRC} plugins/drip_filter.c)

  set(IMGU_SRC ${IMGU_SRC} plugins/setviewnum_filter.c)

  if(PROSILICA_FOUND)
    set(IMGU_SRC ${IMGU_SRC} plugins/gige_camera.c)
  endif()

  if(FFMPEG_FOUND)
    set(IMGU_SRC ${IMGU_SRC} plugins/ffmpeg_camera.c)
  endif()

  if(EPIPHAN_FOUND)
    set(IMGU_SRC ${IMGU_SRC} plugins/epiphan_camera.c)
  endif()

  if(BLACKMAGIC_FOUND)
    set(IMGU_SRC ${IMGU_SRC} plugins/BlackmagicInputCallback.cpp)
    set(IMGU_SRC ${IMGU_SRC} plugins/blackmagic_camera.cpp)
  endif()

  if(IMGU_EXTRA)
    if(OSG_FOUND)
	  set(IMGU_SRC ${IMGU_SRC} plugins/viewer_sink.cpp)
  	  set(IMGU_SRC ${IMGU_SRC} plugins/radialdist_gpufilter.cpp plugins/distorsion_gpufilter.cpp plugins/disto-radiale.c)
	  set(IMGU_SRC ${IMGU_SRC} plugins/blur_gpufilter.cpp)
	  set(IMGU_SRC ${IMGU_SRC} plugins/derivee_gpufilter.cpp)
      set(IMGU_SRC ${IMGU_SRC} plugins/crayon_camera.cpp)
      set(IMGU_SRC ${IMGU_SRC} plugins/crayon_filter.cpp)
#     set(IMGU_SRC ${IMGU_SRC} plugins/crayon_alpha_filter.cpp)
      set(IMGU_SRC ${IMGU_SRC} plugins/crayon.cpp)


      if(OpenCV_FOUND)
#        set(IMGU_SRC ${IMGU_SRC} plugins/dl1track_filter.cpp)
          set(IMGU_SRC ${IMGU_SRC} plugins/grid_calib_filter.cpp)
          set(IMGU_SRC ${IMGU_SRC} plugins/grid_stereo_calib_filter.cpp)
      endif()

    endif(OSG_FOUND)
  endif(IMGU_EXTRA)

  if(V4L_FOUND)
    set(IMGU_SRC ${IMGU_SRC} plugins/v4l2_camera.c plugins/v4l2_support.c plugins/v4l2_jpegutils.c)
  endif()

  if (BMC_FOUND)
    set(IMGU_SRC ${IMGU_SRC} plugins/streamin_camera.c)
    set(IMGU_SRC ${IMGU_SRC} plugins/streamout_sink.c)
#    set(IMGU_SRC ${IMGU_SRC} plugins/stream_support.c)
  endif()
endif()

#####
#####
#####


add_library (imgu SHARED ${IMGU_SRC})
if(IMGU_LIB_8)
  add_library (imgu8 SHARED ${IMGU_SRC})
endif()

if (IMGU_LIB_STATIC)
  add_library (imgu-static STATIC ${IMGU_SRC})
  set_target_properties(imgu-static PROPERTIES OUTPUT_NAME "imgu")
endif()




#
# matrixmath
#

target_link_libraries (matrixmath ${MATRIX_LIBS})
target_link_libraries (imgu ${IMGU_LIBS} matrixmath ${MATRIX_LIBS})
set_target_properties(matrixmath PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(imgu PROPERTIES CLEAN_DIRECT_OUTPUT 1)
install(TARGETS matrixmath LIBRARY DESTINATION ${IMGU_LIB})
install(TARGETS imgu LIBRARY DESTINATION ${IMGU_LIB})
install(FILES matrixmath.h DESTINATION ${IMGU_INCLUDE})
install(FILES libs/svd.h DESTINATION ${IMGU_INCLUDE})
install(FILES libs/graph.h DESTINATION ${IMGU_INCLUDE})
install(FILES libs/block.h DESTINATION ${IMGU_INCLUDE})
install(FILES libs/instances.inc DESTINATION ${IMGU_INCLUDE})

#
# static imgu
#

if (IMGU_LIB_STATIC)
  target_link_libraries (matrixmath-static ${MATRIX_LIBS})
  target_link_libraries (imgu-static ${IMGU_LIBS} matrixmath-static ${MATRIX_LIBS})
  set_target_properties(matrixmath-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  set_target_properties(imgu-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
  install(TARGETS matrixmath-static ARCHIVE DESTINATION ${IMGU_LIB})
  install(TARGETS imgu-static ARCHIVE DESTINATION ${IMGU_LIB})
endif()

#
# 8 bit imgu
#

if(IMGU_LIB_8)
  target_link_libraries (imgu8 ${IMGU_LIBS} matrixmath ${MATRIX_LIBS})
  set_target_properties(imgu8 PROPERTIES COMPILE_DEFINITIONS IMGU8)
  install(TARGETS imgu8 LIBRARY DESTINATION ${IMGU_LIB})
  install(FILES imgu8.h DESTINATION ${IMGU_INCLUDE})
endif()

#
# main imgu library
#

install(FILES imgu.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_core.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_canny.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_complex.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_draw.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_osg.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_matrix.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_sift.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_args.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_plugin.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_seam.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_stereo.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_camera.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_pyramid.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_interpolate.h DESTINATION ${IMGU_INCLUDE})
install(FILES imgu_interpolate_view.h DESTINATION ${IMGU_INCLUDE})
install(FILES param.h DESTINATION ${IMGU_INCLUDE})




