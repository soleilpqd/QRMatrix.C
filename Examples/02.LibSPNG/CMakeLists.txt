cmake_minimum_required(VERSION 3.5)

project(QRMatrixExample LANGUAGES C)

if (APPLE) # libspng from HomeBrew
    # /usr/local/include/spng.h
    include_directories(/usr/local/include)
    # /usr/local/lib/libspng.dylib
    link_directories(/usr/local/lib)
elseif (LINUX) # libspng from Fedora (x64) repository
    # /usr/include/spng.h
    include_directories(/usr/include)
    # /usr/lib64/libspng.so
    link_directories(/usr/lib64)
endif()

# Source
set(PROJECT_SOURCES
    main.c
    ../../DevTools/devtools.h
    ../../DevTools/devtools.c
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

target_link_libraries(QRMatrixExample spng)

install(TARGETS QRMatrixExample
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
