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

#include <stdio.h>
#include <spng.h>
#include <string.h>
#include "../../QRMatrix/qrmatrixencoder.h"
#include "../../String/utf8string.h"

#if __linux__
#define OUTPUT_PREFIX "/home/soleilpqd/Desktop/"
#elif __APPLE__
#define OUTPUT_PREFIX "/Users/soleilpqd/Desktop/"
#endif

/// https://github.com/randy408/libspng/blob/v0.7.3/examples/example.c
void createPNG(UnsignedByte* image, unsigned int dimension, const char* path) {
    spng_ctx *context = spng_ctx_new(SPNG_CTX_ENCODER);
    spng_set_option(context, SPNG_ENCODE_TO_BUFFER, 1);

    struct spng_ihdr ihdr = { 0 };
    ihdr.width = dimension;
    ihdr.height = dimension;
    ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE;
    ihdr.bit_depth = 8; // 1 byte per pixel: refer to `image`
    spng_set_ihdr(context, &ihdr);

    unsigned int length = dimension * dimension;
    int fmt = SPNG_FMT_PNG;
    int result = spng_encode_image(context, image, length, fmt, SPNG_ENCODE_FINALIZE);
    if (result != 0) {
        printf("Encode PNG ERROR 1: %s\n%s\n", path, spng_strerror(result));
        spng_ctx_free(context);
        return;
    }

    size_t pngSize = 0;
    void *pngBuffer = spng_get_png_buffer(context, &pngSize, &result);
    if (pngBuffer == NULL) {
        printf("Encode PNG ERROR 2: %s\n%s\n", path, spng_strerror(result));
        spng_ctx_free(context);
        return;
    }

    FILE* outFile = fopen(path, "w");
    if (outFile != NULL) {
        fwrite((const void*)pngBuffer, pngSize, 1, outFile);
        fclose(outFile);
    } else {
        printf("Fail to pen file: %s\n", path);
    }

    free(pngBuffer);
    spng_ctx_free(context);
}

void fillCell(
    UnsignedByte* image,
    unsigned int dimension,
    unsigned int row,
    unsigned int column,
    UnsignedByte scale,
    UnsignedByte quiteZone
) {
    unsigned int rowOffset = (row + quiteZone) * scale * dimension;
    unsigned int colOffset = (column + quiteZone) * scale;
    for (unsigned int index = 0; index < scale; index += 1) {
        unsigned int pic = rowOffset + index * dimension + colOffset;
        for (unsigned jndex = 0; jndex < scale; jndex += 1) {
            image[pic] = 0;
            pic += 1;
        }
    }
}

void makeQR(QrmBoard board, const char* path, bool isMicro) {
    UnsignedByte quietZone = isMicro ? 2 : 4;
    static UnsignedByte scale = 10;
    unsigned int dimension = (board.dimension + 2 * quietZone) * scale;
    unsigned int length = dimension * dimension;
    ALLOC(UnsignedByte, image, length); // each byte is a image pixel
    // Fill white
    for (unsigned int index = 0; index < length; index += 1) {
        image[index] = 0xFF;
    }
    // Draw QR board (black cells)
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = board.buffer[row][column];
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet) {
                fillCell(image, dimension, row, column, scale, quietZone);
            }
        }
    }
    // Write PNG file
    createPNG(image, dimension, path);
    DEALLOC(image);
}

void encodeQR(const char* path, const UnsignedByte* raw, QrmEncodingMode mode, unsigned int eci, bool isMicro) {
    QrmSegment segment = QrmSegCreate(mode, raw, strlen((const char*)raw), eci != 0 ? eci : DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments[] = {segment};
    QrmErrorCorrectionLevel level = isMicro ? ELevelLow : ELevelHigh;
    QrmExtraEncodingInfo extra = isMicro ? QrmExtraCreate(XModeMicroQr) : QrmExtraCreateNone();
    QrmBoard board = QrmEncoderEncode(segments, 1, level, extra, 0, 0xFF);
    QrmSegDestroy(&segment);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        printf("FAILED: %s\n", path);
        QrmBoardDestroy(&board);
        return;
    }
    printf("QR: %s\n", path);
    QrmBoardPrintDescription(board, false);
    makeQR(board, path, isMicro);
    QrmBoardDestroy(&board);
}

/// @brief Numeric mode
void testNumeric() {
    // This must be QR Version 1 (with EC high, 17 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"12345678901234567";
    encodeQR(OUTPUT_PREFIX"test_numeric.png", raw, EModeNumeric, 0, false);
}


/// @brief Alphanumeric mode
void testAlphaNumeric() {
    // This must be QR Version 1 (with EC high, 10 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"ABC$ 67890";
    encodeQR(OUTPUT_PREFIX"test_alphanumeric.png", raw, EModeAlphaNumeric, 0, false);
}

/// @brief Byte mode with standard encoding (Latin1)
void testLatin() {
    // This must be QR Version 1 (with EC high, 7 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"L1! \xA9\xC2\xE2";
    encodeQR(OUTPUT_PREFIX"test_latin.png", raw, EModeByte, 0, false);
}

/// @brief Kanji mode (with ShiftJIS encoding)
void testKanji() {
    // This must be QR Version 1 (with EC high, 4 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // "０Ａあア" ShiftJIS // \x88\x9f 亜
    encodeQR(OUTPUT_PREFIX"test_kanji.png", raw, EModeKanji, 0, false);
}

/// @brief Byte mode using UTF-8 encoding directly
void testUtf8() {
    const UnsignedByte* raw = (const UnsignedByte*)"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    encodeQR(OUTPUT_PREFIX"test_utf8.png", raw, EModeByte, 0, false);
}

/// @brief ECI mode with UTF-8 encoding
void testUtf8ECI() {
    const UnsignedByte* raw = (const UnsignedByte*)"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    encodeQR(OUTPUT_PREFIX"test_utf8eci.png", raw, EModeByte, 26, false);
}

/// @brief ECI mode with custom encoding (EUC-KR)
void testCustomECI() {
    const UnsignedByte* raw = (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21"; // "안녕하세요!" EUC-KR sequence
    encodeQR(OUTPUT_PREFIX"test_eci.png", raw, EModeByte, 30, false); // 30 is ECI Indicator for KS X 1001 (which includes EUC-KR)
}

/// @brief Mixed modes
void testMixModes() {
    QrmSegment segment1 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"ABC123 ", 7, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment2 = QrmSegCreate(EModeByte, (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21", 11, 30);
    QrmSegment segments[] = {segment1, segment2};
    QrmExtraEncodingInfo extra = QrmExtraCreateNone();
    QrmBoard board = QrmEncoderEncode(segments, 2, ELevelHigh, extra, 0, 0xFF);
    printf("MIXED MODES:\n");
    QrmSegDestroy(&segment1);
    QrmSegDestroy(&segment2);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        printf("FAILED\n");
        QrmBoardDestroy(&board);
        return;
    }
    QrmBoardPrintDescription(board, false);
    makeQR(board, OUTPUT_PREFIX"test_mixed.png", false);
    QrmBoardDestroy(&board);
}

/// @brief Mixed modes auto
void testAutoMixModes() {
    const char* raw = "123456789 こんにちは世界！A56B 안녕하세요!";
    Utf8String text = U8Create((const UnsignedByte*)raw, strlen(raw));
    UnicodePoint unicodes = U8ToUnicodes(text);
    unsigned int segmentCount = 0;
    QrmErrorCorrectionLevel ecLevel = ELevelHigh;
    QrmSegment* segments = UPMakeSegments(unicodes, ecLevel, &segmentCount, false);
    printf("Mixed Auto: %d\n", segmentCount);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        printf("%d: %02X", index, segments[index].mode);
    }
    printf("\n");
    QrmExtraEncodingInfo extra = QrmExtraCreateNone();
    QrmBoard board = QrmEncoderEncode(segments, segmentCount, ecLevel, extra, 0, DEFAULT_ECI_ASSIGMENT);

    U8Destroy(&text);
    UPDestroy(&unicodes);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    DEALLOC(segments);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        printf("FAILED\n");
        QrmBoardDestroy(&board);
        return;
    }
    QrmBoardPrintDescription(board, false);
    makeQR(board, OUTPUT_PREFIX"test_mixed_auto.png", false);
    QrmBoardDestroy(&board);
}

/// @brief Numeric mode
void testMicroNumeric() {
    const UnsignedByte* raw = (const UnsignedByte*)"12345";
    encodeQR(OUTPUT_PREFIX"test_micro_numeric.png", raw, EModeNumeric, 0, true);
}

/// @brief Alphanumeric mode
void testMicroAlphaNumeric() {
    const UnsignedByte* raw = (const UnsignedByte*)"A12345";
    encodeQR(OUTPUT_PREFIX"test_micro_alphanumeric.png", raw, EModeAlphaNumeric, 0, true);
}

/// @brief Kanji mode (with ShiftJIS encoding)
void testMicroKanji() {
    const UnsignedByte* raw = (const UnsignedByte*)"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // "０Ａあア" ShiftJIS
    encodeQR(OUTPUT_PREFIX"test_micro_kanji.png", raw, EModeKanji, 0, true);
}

/// @brief Byte mode using UTF-8 encoding directly
void testMicroByte() {
    const UnsignedByte* raw = (const UnsignedByte*)"안녕";
    encodeQR(OUTPUT_PREFIX"test_micro_byte.png", raw, EModeByte, 0, true);
}

/// @brief Mixed modes auto
void testMicroMixModes() {
    const char* raw = "1234こんにちは";
    Utf8String text = U8Create((const UnsignedByte*)raw, strlen(raw));
    UnicodePoint unicodes = U8ToUnicodes(text);
    unsigned int segmentCount = 0;
    QrmErrorCorrectionLevel ecLevel = ELevelLow;
    QrmSegment* segments = UPMakeSegments(unicodes, ecLevel, &segmentCount, true);
    printf("Micro Mixed Auto: %d\n", segmentCount);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        printf("%d: %02X", index, segments[index].mode);
    }
    printf("\n");
    QrmExtraEncodingInfo extra = QrmExtraCreate(XModeMicroQr);
    QrmBoard board = QrmEncoderEncode(segments, segmentCount, ecLevel, extra, 0, DEFAULT_ECI_ASSIGMENT);

    U8Destroy(&text);
    UPDestroy(&unicodes);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    DEALLOC(segments);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        printf("FAILED\n");
        QrmBoardDestroy(&board);
        return;
    }
    QrmBoardPrintDescription(board, false);
    makeQR(board, OUTPUT_PREFIX"test_micro_mixed_auto.png", true);
    QrmBoardDestroy(&board);
}

void testFNC1First() {
    QrmSegment segment1 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"01049123451234591597033130128", 29, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment2 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"%10ABC123", 9, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments[] = {segment1, segment2};
    QrmExtraEncodingInfo extra = QrmExtraCreate(XModeFnc1First);
    QrmBoard board = QrmEncoderEncode(segments, 2, ELevelHigh, extra, 0, 0xFF);
    printf("FNC1 1st:\n");
    QrmSegDestroy(&segment1);
    QrmSegDestroy(&segment2);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        printf("FAILED\n");
        QrmBoardDestroy(&board);
        return;
    }
    QrmBoardPrintDescription(board, false);
    makeQR(board, OUTPUT_PREFIX"test_fnc1_1st.png", false);
    QrmBoardDestroy(&board);
}

void testFNC1Second() {
    QrmSegment segment1 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"AA1234BBB112", 12, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment2 = QrmSegCreate(EModeByte, (const UnsignedByte*)"text text text text\n", 20, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments[] = {segment1, segment2};
    QrmExtraEncodingInfo extra = QrmExtraCreateFnc1Second((UnsignedByte*)"37", 2);
    QrmBoard board = QrmEncoderEncode(segments, 2, ELevelHigh, extra, 0, 0xFF);
    printf("FNC1 2nd:\n");
    QrmSegDestroy(&segment1);
    QrmSegDestroy(&segment2);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        printf("FAILED\n");
        QrmBoardDestroy(&board);
        return;
    }
    QrmBoardPrintDescription(board, false);
    makeQR(board, OUTPUT_PREFIX"test_fnc1_2nd.png", false);
    QrmBoardDestroy(&board);
}

void testStructuredAppend() {
    QrmErrorCorrectionLevel ecLevel = ELevelLow;

    QrmSegment segment11 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"123", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment12 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"ABC", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments1[] = {segment11, segment12};
    QrmStructuredAppend part1 = QrmStrAppCreate(segments1, 2, ecLevel);

    QrmSegment segment21 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"345", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment22 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"DEF", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments2[] = {segment21, segment22};
    QrmStructuredAppend part2 = QrmStrAppCreate(segments2, 2, ecLevel);

    QrmStructuredAppend allParts[] = {part1, part2};
    QrmBoard* boards = QrmEncoderMakeStructuredAppend(allParts, 2);

    QrmSegDestroy(&segment11);
    QrmSegDestroy(&segment12);
    QrmSegDestroy(&segment21);
    QrmSegDestroy(&segment22);
    QrmStrAppDestroy(&part1);
    QrmStrAppDestroy(&part2);

    if (boards == NULL) {
        printf("STRUCTURED APPEND failed\n");
        return;
    }

    if (boards[0].dimension > 0) {
        QrmBoardPrintDescription(boards[0], false);
        makeQR(boards[0], OUTPUT_PREFIX"test_structured_append_1.png", false);
    } else {
        printf("STRUCTURED APPEND 1 failed\n");
    }
    QrmBoardDestroy(&boards[0]);

    if (boards[1].dimension > 0) {
        QrmBoardPrintDescription(boards[1], false);
        makeQR(boards[1], OUTPUT_PREFIX"test_structured_append_2.png", false);
    } else {
        printf("STRUCTURED APPEND 2 failed\n");
    }
    QrmBoardDestroy(&boards[1]);

    DEALLOC(boards);
}
int main() {
    QRMatrixInit();
    if (!qrmIsEnvValid) {
        printf("Invalid QRMatrix environment\n");
        return 1;
    }

    testNumeric();
    testAlphaNumeric();
    testLatin();
    testKanji();
    testUtf8();
    testUtf8ECI();
    testCustomECI();
    testMixModes();
    testAutoMixModes();

    testMicroNumeric();
    testMicroAlphaNumeric();
    testMicroByte();
    testMicroKanji();
    testMicroMixModes();

    testFNC1First();
    testFNC1Second();
    testStructuredAppend();

    return 0;
}

