/*
    QRMatrix - QR pixels presentation.
    Copyright © 2023 duongpq/soleilpqd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../QRMatrix/qrmatrixencoder.h"
#include "qrmatrixsvg.h"
#include "../../String/utf8string.h"

#if __linux__
#define OUTPUT_PREFIX "/home/soleilpqd/Desktop/"
#elif __APPLE__
#define OUTPUT_PREFIX "/Users/soleilpqd/Desktop/"
#endif

void encodeQR(const char* path, const UnsignedByte* raw, QrmEncodingMode mode, unsigned int eci, bool isMicro, const char* detailPath) {
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
    QrmSvgDraw(board, path, 10, isMicro);
    if (detailPath != NULL) {
        QrmSvgDrawDetail(
            board, detailPath, 10, isMicro,
            "white",
            "black",
            "blue",
            "green",
            "red",
            "purple",
            "orange",
            "magenta",
            "darkblue",
            "darkgreen"
        );
    }
    QrmBoardDestroy(&board);
}

/// @brief Numeric mode
void testNumeric() {
    // This must be QR Version 1 (with EC high, 17 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"12345678901234567";
    encodeQR(OUTPUT_PREFIX"test_numeric.svg", raw, EModeNumeric, 0, false, NULL);
}


/// @brief Alphanumeric mode
void testAlphaNumeric() {
    // This must be QR Version 1 (with EC high, 10 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"ABC$ 67890";
    encodeQR(OUTPUT_PREFIX"test_alphanumeric.svg", raw, EModeAlphaNumeric, 0, false, NULL);
}

/// @brief Byte mode with standard encoding (Latin1)
void testLatin() {
    // This must be QR Version 1 (with EC high, 7 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"L1! \xA9\xC2\xE2";
    encodeQR(OUTPUT_PREFIX"test_latin.svg", raw, EModeByte, 0, false, NULL);
}

/// @brief Kanji mode (with ShiftJIS encoding)
void testKanji() {
    // This must be QR Version 1 (with EC high, 4 chars maximum)
    // And when we add one or more digits to string, QR Version must increase
    const UnsignedByte* raw = (const UnsignedByte*)"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // "０Ａあア" ShiftJIS // \x88\x9f 亜
    encodeQR(OUTPUT_PREFIX"test_kanji.svg", raw, EModeKanji, 0, false, NULL);
}

/// @brief Byte mode using UTF-8 encoding directly
void testUtf8() {
    const UnsignedByte* raw = (const UnsignedByte*)"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    encodeQR(OUTPUT_PREFIX"test_utf8.svg", raw, EModeByte, 0, false, NULL);
}

/// @brief ECI mode with UTF-8 encoding
void testUtf8ECI() {
    const UnsignedByte* raw = (const UnsignedByte*)"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    encodeQR(OUTPUT_PREFIX"test_utf8eci.svg", raw, EModeByte, 26, false, OUTPUT_PREFIX"test_utf8eci_detail.svg");
}

/// @brief ECI mode with custom encoding (EUC-KR)
void testCustomECI() {
    const UnsignedByte* raw = (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21"; // "안녕하세요!" EUC-KR sequence
    encodeQR(OUTPUT_PREFIX"test_eci.svg", raw, EModeByte, 30, false, NULL); // 30 is ECI Indicator for KS X 1001 (which includes EUC-KR)
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
    QrmSvgDraw(board, OUTPUT_PREFIX"test_mixed.svg", 10, false);
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
    QrmSvgDraw(board, OUTPUT_PREFIX"test_mixed_auto.svg", 10, false);
    QrmBoardDestroy(&board);
}

/// @brief Numeric mode
void testMicroNumeric() {
    const UnsignedByte* raw = (const UnsignedByte*)"12345";
    encodeQR(OUTPUT_PREFIX"test_micro_numeric.svg", raw, EModeNumeric, 0, true, NULL);
}

/// @brief Alphanumeric mode
void testMicroAlphaNumeric() {
    const UnsignedByte* raw = (const UnsignedByte*)"A12345";
    encodeQR(OUTPUT_PREFIX"test_micro_alphanumeric.svg", raw, EModeAlphaNumeric, 0, true, NULL);
}

/// @brief Kanji mode (with ShiftJIS encoding)
void testMicroKanji() {
    const UnsignedByte* raw = (const UnsignedByte*)"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // "０Ａあア" ShiftJIS
    encodeQR(OUTPUT_PREFIX"test_micro_kanji.svg", raw, EModeKanji, 0, true, NULL);
}

/// @brief Byte mode using UTF-8 encoding directly
void testMicroByte() {
    const UnsignedByte* raw = (const UnsignedByte*)"안녕";
    encodeQR(OUTPUT_PREFIX"test_micro_byte.svg", raw, EModeByte, 0, true, NULL);
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
    QrmSvgDraw(board, OUTPUT_PREFIX"test_micro_mixed_auto.svg", 10, true);
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
    QrmSvgDraw(board, OUTPUT_PREFIX"test_fnc1_1st.svg", 10, false);
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
    QrmSvgDraw(board, OUTPUT_PREFIX"test_fnc1_2nd.svg", 10, false);
    QrmBoardDestroy(&board);
}

void testStructuredAppend() {
    QrmErrorCorrectionLevel ecLevel = ELevelLow;

    QrmSegment segment11 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"123", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment12 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"ABC", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments1[] = {segment11, segment12};
    QrmStructuredAppend  part1 = QrmStrAppCreate(segments1, 2, ecLevel);

    QrmSegment segment21 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"345", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment22 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"DEF", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments2[] = {segment21, segment22};
    QrmStructuredAppend  part2 = QrmStrAppCreate(segments2, 2, ecLevel);

    QrmStructuredAppend  allParts[] = {part1, part2};
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
        QrmSvgDraw(boards[0], OUTPUT_PREFIX"test_structured_append_1.svg", 10, false);
    } else {
        printf("STRUCTURED APPEND 1 failed\n");
    }
    QrmBoardDestroy(&boards[0]);

    if (boards[1].dimension > 0) {
        QrmBoardPrintDescription(boards[1], false);
        QrmSvgDraw(boards[1], OUTPUT_PREFIX"test_structured_append_2.svg", 10, false);
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
