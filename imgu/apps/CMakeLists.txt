if (IMGU_APPS)
  add_executable (pngls pngls.c)
  target_link_libraries (pngls imgu)

  add_executable (pngmoyenne pngmoyenne.c)
  target_link_libraries (pngmoyenne imgu)

  install(TARGETS pngls RUNTIME DESTINATION ${IMGU_BIN})
  install(TARGETS pngmoyenne RUNTIME DESTINATION ${IMGU_BIN})
endif()




