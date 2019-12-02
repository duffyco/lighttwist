

install(FILES rca_indian_head_test_pattern.jpg DESTINATION ${IMGU_SHARE})
install(FILES damier_768_480.png DESTINATION ${IMGU_SHARE})
install(FILES coeffDisto_d60.data DESTINATION ${IMGU_SHARE})
install(FILES paint.frag DESTINATION ${IMGU_SHARE})

install(FILES horloge10.avi DESTINATION ${IMGU_SHARE})

file (GLOB debrui "debruijn/*.png")
install (FILES ${debrui} DESTINATION ${IMGU_SHARE}/debruijn)

file (GLOB shaders "shaders/*.glsl")
install (FILES ${shaders} DESTINATION ${IMGU_SHARE}/shaders)


install(FILES MinSWGray.png DESTINATION ${IMGU_SHARE})

