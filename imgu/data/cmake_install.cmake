# Install script for directory: /home/jon/lighttwist/imgu/data

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu" TYPE FILE FILES "/home/jon/lighttwist/imgu/data/rca_indian_head_test_pattern.jpg")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu" TYPE FILE FILES "/home/jon/lighttwist/imgu/data/damier_768_480.png")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu" TYPE FILE FILES "/home/jon/lighttwist/imgu/data/coeffDisto_d60.data")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu" TYPE FILE FILES "/home/jon/lighttwist/imgu/data/paint.frag")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu" TYPE FILE FILES "/home/jon/lighttwist/imgu/data/horloge10.avi")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu/debruijn" TYPE FILE FILES
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_2sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_11sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_8sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_9sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_10sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_14sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_12sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_10sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_6sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_8sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_7sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_2sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_9sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_4sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_5sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_3sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_13sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_3sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_15sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_16sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_5sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_11sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_6sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_4bandes_4sym.png"
    "/home/jon/lighttwist/imgu/data/debruijn/debruijn_3bandes_7sym.png"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu/shaders" TYPE FILE FILES
    "/home/jon/lighttwist/imgu/data/shaders/shader_rgblut.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_yuvlutHLblend.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_rgb.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_rgblutblend.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_rgba.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_rgblutHL.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_yuv.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_test.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_gray.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_yuvclamp.glsl"
    "/home/jon/lighttwist/imgu/data/shaders/shader_yuvlutblend.glsl"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgu" TYPE FILE FILES "/home/jon/lighttwist/imgu/data/MinSWGray.png")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

