# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

# Project-level configuration.
set(PROJECT_NAME "qrmatrixexample")
project(${PROJECT_NAME} LANGUAGES C)

set(PROJECT_SOURCES
  # ${CMAKE_SOURCE_DIR}/../../../../DevTools/devtools.h
  # ${CMAKE_SOURCE_DIR}/../../../../DevTools/devtools.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/constants.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/common.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/common.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixsegment.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixsegment.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixextramode.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixextramode.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixboard.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixboard.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixencoder.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/qrmatrixencoder.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Encoder/numericencoder.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Encoder/numericencoder.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Encoder/kanjiencoder.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Encoder/kanjiencoder.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Encoder/alphanumericencoder.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Encoder/alphanumericencoder.c
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Polynomial/polynomial.h
  ${CMAKE_SOURCE_DIR}/../../../../QRMatrix/Polynomial/polynomial.c
  ${CMAKE_SOURCE_DIR}/../../../../String/utf8string.c
  ${CMAKE_SOURCE_DIR}/../../../../String/utf8string.h
  ${CMAKE_SOURCE_DIR}/../../../../String/unicodepoint.c
  ${CMAKE_SOURCE_DIR}/../../../../String/unicodepoint.h
  ${CMAKE_SOURCE_DIR}/../../../../String/latinstring.c
  ${CMAKE_SOURCE_DIR}/../../../../String/latinstring.h
  ${CMAKE_SOURCE_DIR}/../../../../String/shiftjisstring.c
  ${CMAKE_SOURCE_DIR}/../../../../String/shiftjisstring.h
  ${CMAKE_SOURCE_DIR}/../../../../String/shiftjisstringmap.c
  ${CMAKE_SOURCE_DIR}/../../../../String/shiftjisstringmap.h
)

add_library(qrmatrixexample SHARED
  ${PROJECT_SOURCES}
)

target_compile_definitions(qrmatrixexample PUBLIC DART_SHARED_LIB)

# List of absolute paths to libraries that should be bundled with the plugin.
# This list could contain prebuilt libraries, or libraries created by an
# external build triggered from this build file.
set(qrmatrixexample_bundled_libraries
  # Defined in ../src/CMakeLists.txt.
  # This can be changed to accommodate different builds.
  $<TARGET_FILE:qrmatrixexample>
  PARENT_SCOPE
)
