/*
    QRMatrix - QR pixels presentation.
    Copyright © 2023 duongpq/soleilpqd.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the “Software”), to deal in
    the Software without restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
    Software, and to permit persons to whom the Software is furnished to do so, subject
    to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies
    or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
    FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define LOGABLE 0
#define LOG_MEM 0

#if LOGABLE

#include <stdio.h>
#include "../DevTools/devtools.h"

#ifdef ANDROID
#include <android/log.h>

#define LOG_TAG "QRMaxtrix.C"

#define LOG(...);       __android_log_print(ANDROID_LOG_VERBOSE , LOG_TAG, "📌 [%d] %s:", __LINE__, __PRETTY_FUNCTION__); __android_log_print(ANDROID_LOG_VERBOSE , LOG_TAG, __VA_ARGS__);
#define LOG_HEX_S(...); __android_log_print(ANDROID_LOG_VERBOSE , LOG_TAG, "Log not implemented");
#define LOG_HEX(...);   __android_log_print(ANDROID_LOG_VERBOSE , LOG_TAG, "Log not implemented");
#define LOG_BIN_S(...); __android_log_print(ANDROID_LOG_VERBOSE , LOG_TAG, "Log not implemented");
#define LOG_BIN(...);   __android_log_print(ANDROID_LOG_VERBOSE , LOG_TAG, "Log not implemented");

#else

#define LOG(...);       printf("📌 [%d] %s:\n", __LINE__, __PRETTY_FUNCTION__); printf(__VA_ARGS__); printf("\n🏁\n");
#define LOG_HEX_S(...); printf("📌 [%d] %s:\n", __LINE__, __PRETTY_FUNCTION__); DevPrintSingleHex(__VA_ARGS__); printf("🏁\n");
#define LOG_HEX(...);   printf("📌 [%d] %s:\n", __LINE__, __PRETTY_FUNCTION__); DevPrintHex(__VA_ARGS__); printf("🏁\n");
#define LOG_BIN_S(...); printf("📌 [%d] %s:\n", __LINE__, __PRETTY_FUNCTION__); DevPrintSingleBin(__VA_ARGS__); printf("🏁\n");
#define LOG_BIN(...);   printf("📌 [%d] %s:\n", __LINE__, __PRETTY_FUNCTION__); DevPrintBin(__VA_ARGS__); printf("🏁\n");

#endif
#else

#define LOG(...);
#define LOG_HEX_S(...);
#define LOG_HEX(...);
#define LOG_BIN_S(...);
#define LOG_BIN(...);

#endif

#if LOGABLE && LOG_MEM

#define ALLOC(T, V, L);     T* V = (T*)calloc(L, sizeof(T)); printf("📌 ALLOC [%d] %s: %p (%d x %lu = %lu) 🏁\n", __LINE__, __FUNCTION__, V, L, sizeof(T), L * sizeof(T));
#define ALLOC_(T, V, L);    V = (T*)calloc(L, sizeof(T)); printf("📌 ALLOC [%d] %s: %p (%d x %lu = %lu) 🏁\n", __LINE__, __FUNCTION__, V, L, sizeof(T), L * sizeof(T));
#define REALLOC(T, V, L);   V = (T*)realloc(V, L * sizeof(T)); printf("📌 REALLOC [%d] %s: %p (%d x %lu = %lu) 🏁\n", __LINE__, __FUNCTION__, V, L, sizeof(T), L * sizeof(T));
#define DEALLOC(V);         printf("📌 DEALLOC [%d] %s: %p 🏁\n", __LINE__, __FUNCTION__, V); free(V); V = NULL;

#else

#define ALLOC(T, V, L);     T* V = (T*)calloc(L, sizeof(T));
#define ALLOC_(T, V, L);    V = (T*)calloc(L, sizeof(T));
#define REALLOC(T, V, L);   V = (T*)realloc(V, L * sizeof(T));
#define DEALLOC(V);         free(V); V = NULL;

#endif

#define VERSION "1.1.0"

#include <stdbool.h>

typedef unsigned char UnsignedByte;
typedef unsigned short Unsigned2Bytes;
typedef unsigned int Unsigned4Bytes;

/// QR Encoding Mode
typedef enum {
    /// 0-9
    EModeNumeric = 0b0001,
    /// 0-9, A-Z, $, %, *, +, -, ., /, :, ' '
    EModeAlphaNumeric = 0b0010,
    /// ISO-8859-1 (Unicode Latin-1)
    EModeByte = 0b0100,
    /// Shift JIS double-byte characters
    EModeKanji = 0b1000
} QrmEncodingMode;

/// QR Error correction level
typedef enum {
    /// L 7%
    ELevelLow = 0b01,
    /// M 15%
    ELevelMedium = 0b00,
    /// Q 25%
    ELevelQuarter = 0b11,
    /// H 30%
    ELevelHigh = 0b10
} QrmErrorCorrectionLevel;

#endif // CONSTANTS_H
