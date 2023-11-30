cmake_minimum_required(VERSION 3.5)

project(QRMatrixExample VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QRMatrixExample
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(QRMatrixExample
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(QRMatrixExample PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(QRMatrixExample PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS QRMatrixExample
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QRMatrixExample)
endif()
