cmake_minimum_required(VERSION 3.5)

project(QRMatrixExample LANGUAGES C)

set(PROJECT_SOURCES
    main.c
    qrmatrixsvg.h
    qrmatrixsvg.c
#    ../../DevTools/devtools.h
#    ../../DevTools/devtools.c
    ../../QRMatrix/constants.h
    ../../QRMatrix/common.h
    ../../QRMatrix/common.c
    ../../QRMatrix/qrmatrixsegment.h
    ../../QRMatrix/qrmatrixsegment.c
    ../../QRMatrix/qrmatrixextramode.h
    ../../QRMatrix/qrmatrixextramode.c
    ../../QRMatrix/qrmatrixboard.c
    ../../QRMatrix/qrmatrixboard.h
    ../../QRMatrix/qrmatrixencoder.c
    ../../QRMatrix/qrmatrixencoder.h
    ../../QRMatrix/Encoder/numericencoder.h
    ../../QRMatrix/Encoder/numericencoder.c
    ../../QRMatrix/Encoder/kanjiencoder.h
    ../../QRMatrix/Encoder/kanjiencoder.c
    ../../QRMatrix/Encoder/alphanumericencoder.h
    ../../QRMatrix/Encoder/alphanumericencoder.c
    ../../QRMatrix/Polynomial/polynomial.h
    ../../QRMatrix/Polynomial/polynomial.c
    ../../String/utf8string.c
    ../../String/utf8string.h
    ../../String/unicodepoint.c
    ../../String/unicodepoint.h
    ../../String/latinstring.c
    ../../String/latinstring.h
    ../../String/shiftjisstring.c
    ../../String/shiftjisstring.h
    ../../String/shiftjisstringmap.c
    ../../String/shiftjisstringmap.h
)

add_executable(QRMatrixExample ${PROJECT_SOURCES})

install(TARGETS QRMatrixExample
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
