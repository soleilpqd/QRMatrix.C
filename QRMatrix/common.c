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

#include "common.h"
#include <stdlib.h>
#include <string.h>

UnsignedByte QrmInfoGroupCount(QrmSymbolInfo info) {
    return (info.group2Blocks == 0 ? 1 : 2);
}

UnsignedByte QrmInfoECBlockTotalCount(QrmSymbolInfo info) {
    return info.group1Blocks + info.group2Blocks;
}

unsigned int QrmInfoECCodewordsTotalCount(QrmSymbolInfo info) {
    return QrmInfoECBlockTotalCount(info) * info.ecCodewordsPerBlock;
}

bool qrmIsEnvValid = false;
bool qrmIsLittleEndian = false;

void QrmCheckEnv() {
    qrmIsEnvValid = (sizeof(UnsignedByte) == 1) && (sizeof(Unsigned2Bytes) == 2) && (sizeof(Unsigned4Bytes) == 4);
    unsigned int value = 0x00000001;
    unsigned char *ptr = (unsigned char*)&value;
    qrmIsLittleEndian = (*ptr == 1);
}

UnsignedByte QrmGetDimensionByVersion(UnsignedByte version, bool isMicro) {
    if (isMicro) {
        return (version - 1) * MICROQR_VERSION_OFFSET + MICROQR_MIN_DIMENSION;
    }
    return (version - 1) * QR_VERSION_OFFSET + QR_MIN_DIMENSION;
}

int QrmInfo_convertLevelToIndex(QrmErrorCorrectionLevel level) {
    switch (level) {
    case ELevelLow:
        return 0;
    case ELevelMedium:
        return 1;
    case ELevelQuarter:
        return 2;
    case ELevelHigh:
        return 3;
    }
    return -1;
}

void QrmSymbolInfoInit(QrmSymbolInfo* result) {
    result->version = 0;
    result->level = 0;
    result->codewords = 0;
    result->ecCodewordsPerBlock = 0;
    result->group1Blocks = 0;
    result->group2Blocks = 0;
    result->group1BlockCodewords = 0;
    result->group2BlockCodewords = 0;
}

QrmSymbolInfo QrmGetSymbolInfo(UnsignedByte version, QrmErrorCorrectionLevel level, bool isMicro) {
    static const Unsigned2Bytes map[40][4][6] = {
        {{  19,  7,  1,  19,  0,   0}, {  16, 10,  1,  16,  0,   0}, {  13, 13,  1,  13,  0,   0}, {   9, 17,  1,   9,  0,   0}},
        {{  34, 10,  1,  34,  0,   0}, {  28, 16,  1,  28,  0,   0}, {  22, 22,  1,  22,  0,   0}, {  16, 28,  1,  16,  0,   0}},
        {{  55, 15,  1,  55,  0,   0}, {  44, 26,  1,  44,  0,   0}, {  34, 18,  2,  17,  0,   0}, {  26, 22,  2,  13,  0,   0}},
        {{  80, 20,  1,  80,  0,   0}, {  64, 18,  2,  32,  0,   0}, {  48, 26,  2,  24,  0,   0}, {  36, 16,  4,   9,  0,   0}},
        {{ 108, 26,  1, 108,  0,   0}, {  86, 24,  2,  43,  0,   0}, {  62, 18,  2,  15,  2,  16}, {  46, 22,  2,  11,  2,  12}},
        {{ 136, 18,  2,  68,  0,   0}, { 108, 16,  4,  27,  0,   0}, {  76, 24,  4,  19,  0,   0}, {  60, 28,  4,  15,  0,   0}},
        {{ 156, 20,  2,  78,  0,   0}, { 124, 18,  4,  31,  0,   0}, {  88, 18,  2,  14,  4,  15}, {  66, 26,  4,  13,  1,  14}},
        {{ 194, 24,  2,  97,  0,   0}, { 154, 22,  2,  38,  2,  39}, { 110, 22,  4,  18,  2,  19}, {  86, 26,  4,  14,  2,  15}},
        {{ 232, 30,  2, 116,  0,   0}, { 182, 22,  3,  36,  2,  37}, { 132, 20,  4,  16,  4,  17}, { 100, 24,  4,  12,  4,  13}},
        {{ 274, 18,  2,  68,  2,  69}, { 216, 26,  4,  43,  1,  44}, { 154, 24,  6,  19,  2,  20}, { 122, 28,  6,  15,  2,  16}},
        {{ 324, 20,  4,  81,  0,   0}, { 254, 30,  1,  50,  4,  51}, { 180, 28,  4,  22,  4,  23}, { 140, 24,  3,  12,  8,  13}},
        {{ 370, 24,  2,  92,  2,  93}, { 290, 22,  6,  36,  2,  37}, { 206, 26,  4,  20,  6,  21}, { 158, 28,  7,  14,  4,  15}},
        {{ 428, 26,  4, 107,  0,   0}, { 334, 22,  8,  37,  1,  38}, { 244, 24,  8,  20,  4,  21}, { 180, 22, 12,  11,  4,  12}},
        {{ 461, 30,  3, 115,  1, 116}, { 365, 24,  4,  40,  5,  41}, { 261, 20, 11,  16,  5,  17}, { 197, 24, 11,  12,  5,  13}},
        {{ 523, 22,  5,  87,  1,  88}, { 415, 24,  5,  41,  5,  42}, { 295, 30,  5,  24,  7,  25}, { 223, 24, 11,  12,  7,  13}},
        {{ 589, 24,  5,  98,  1,  99}, { 453, 28,  7,  45,  3,  46}, { 325, 24, 15,  19,  2,  20}, { 253, 30,  3,  15, 13,  16}},
        {{ 647, 28,  1, 107,  5, 108}, { 507, 28, 10,  46,  1,  47}, { 367, 28,  1,  22, 15,  23}, { 283, 28,  2,  14, 17,  15}},
        {{ 721, 30,  5, 120,  1, 121}, { 563, 26,  9,  43,  4,  44}, { 397, 28, 17,  22,  1,  23}, { 313, 28,  2,  14, 19,  15}},
        {{ 795, 28,  3, 113,  4, 114}, { 627, 26,  3,  44, 11,  45}, { 445, 26, 17,  21,  4,  22}, { 341, 26,  9,  13, 16,  14}},
        {{ 861, 28,  3, 107,  5, 108}, { 669, 26,  3,  41, 13,  42}, { 485, 30, 15,  24,  5,  25}, { 385, 28, 15,  15, 10,  16}},
        {{ 932, 28,  4, 116,  4, 117}, { 714, 26, 17,  42,  0,   0}, { 512, 28, 17,  22,  6,  23}, { 406, 30, 19,  16,  6,  17}},
        {{1006, 28,  2, 111,  7, 112}, { 782, 28, 17,  46,  0,   0}, { 568, 30,  7,  24, 16,  25}, { 442, 24, 34,  13,  0,   0}},
        {{1094, 30,  4, 121,  5, 122}, { 860, 28,  4,  47, 14,  48}, { 614, 30, 11,  24, 14,  25}, { 464, 30, 16,  15, 14,  16}},
        {{1174, 30,  6, 117,  4, 118}, { 914, 28,  6,  45, 14,  46}, { 664, 30, 11,  24, 16,  25}, { 514, 30, 30,  16,  2,  17}},
        {{1276, 26,  8, 106,  4, 107}, {1000, 28,  8,  47, 13,  48}, { 718, 30,  7,  24, 22,  25}, { 538, 30, 22,  15, 13,  16}},
        {{1370, 28, 10, 114,  2, 115}, {1062, 28, 19,  46,  4,  47}, { 754, 28, 28,  22,  6,  23}, { 596, 30, 33,  16,  4,  17}},
        {{1468, 30,  8, 122,  4, 123}, {1128, 28, 22,  45,  3,  46}, { 808, 30,  8,  23, 26,  24}, { 628, 30, 12,  15, 28,  16}},
        {{1531, 30,  3, 117, 10, 118}, {1193, 28,  3,  45, 23,  46}, { 871, 30,  4,  24, 31,  25}, { 661, 30, 11,  15, 31,  16}},
        {{1631, 30,  7, 116,  7, 117}, {1267, 28, 21,  45,  7,  46}, { 911, 30,  1,  23, 37,  24}, { 701, 30, 19,  15, 26,  16}},
        {{1735, 30,  5, 115, 10, 116}, {1373, 28, 19,  47, 10,  48}, { 985, 30, 15,  24, 25,  25}, { 745, 30, 23,  15, 25,  16}},
        {{1843, 30, 13, 115,  3, 116}, {1455, 28,  2,  46, 29,  47}, {1033, 30, 42,  24,  1,  25}, { 793, 30, 23,  15, 28,  16}},
        {{1955, 30, 17, 115,  0,   0}, {1541, 28, 10,  46, 23,  47}, {1115, 30, 10,  24, 35,  25}, { 845, 30, 19,  15, 35,  16}},
        {{2071, 30, 17, 115,  1, 116}, {1631, 28, 14,  46, 21,  47}, {1171, 30, 29,  24, 19,  25}, { 901, 30, 11,  15, 46,  16}},
        {{2191, 30, 13, 115,  6, 116}, {1725, 28, 14,  46, 23,  47}, {1231, 30, 44,  24,  7,  25}, { 961, 30, 59,  16,  1,  17}},
        {{2306, 30, 12, 121,  7, 122}, {1812, 28, 12,  47, 26,  48}, {1286, 30, 39,  24, 14,  25}, { 986, 30, 22,  15, 41,  16}},
        {{2434, 30,  6, 121, 14, 122}, {1914, 28,  6,  47, 34,  48}, {1354, 30, 46,  24, 10,  25}, {1054, 30,  2,  15, 64,  16}},
        {{2566, 30, 17, 122,  4, 123}, {1992, 28, 29,  46, 14,  47}, {1426, 30, 49,  24, 10,  25}, {1096, 30, 24,  15, 46,  16}},
        {{2702, 30,  4, 122, 18, 123}, {2102, 28, 13,  46, 32,  47}, {1502, 30, 48,  24, 14,  25}, {1142, 30, 42,  15, 32,  16}},
        {{2812, 30, 20, 117,  4, 118}, {2216, 28, 40,  47,  7,  48}, {1582, 30, 43,  24, 22,  25}, {1222, 30, 10,  15, 67,  16}},
        {{2956, 30, 19, 118,  6, 119}, {2334, 28, 18,  47, 31,  48}, {1666, 30, 34,  24, 34,  25}, {1276, 30, 20,  15, 61,  16}}
    };
    static const Unsigned2Bytes mapMicro[4][4][2] = {
        {{  3, 2 }, {  3,  2 }, {  3,  2 }, { 3, 2 }},
        {{  5, 5 }, {  4,  6 }, {  0,  0 }, { 0, 0 }},
        {{ 11, 6 }, {  9,  8 }, {  0,  0 }, { 0, 0 }},
        {{ 16, 8 }, { 14, 10 }, { 10, 13 }, { 0, 0 }},
    };

    QrmSymbolInfo result; QrmSymbolInfoInit(&result);

    int versionIndex = version - 1;
    if (versionIndex < 0 || versionIndex >= (isMicro ? MICROQR_MAX_VERSION : QR_MAX_VERSION)) {
        return result;
    }
    int levelIndex = QrmInfo_convertLevelToIndex(level);
    if (levelIndex < 0) {
        return result;
    }

    result.version = version;
    result.level = level;

    if (isMicro) {
        const Unsigned2Bytes* data = mapMicro[versionIndex][levelIndex];
        result.codewords = data[0];
        result.ecCodewordsPerBlock = data[1];
        result.group1Blocks = 1;
        result.group1BlockCodewords = data[0];
        result.group2Blocks = 0;
        result.group2BlockCodewords = 0;
    } else {
        const Unsigned2Bytes* data = map[versionIndex][levelIndex];
        result.codewords = data[0];
        result.ecCodewordsPerBlock = data[1];
        result.group1Blocks = data[2];
        result.group1BlockCodewords = data[3];
        result.group2Blocks = data[4];
        result.group2BlockCodewords = data[5];
    }

    return result;
}

unsigned int QrmGetCharactersCountIndicatorLength(
    UnsignedByte version,
    QrmEncodingMode mode,
    bool isMicro
) {
    if (isMicro) {
        switch (mode) {
        case EModeNumeric:
            return version + 2;
        case EModeAlphaNumeric:
            return (version < 2) ? 0 : version + 1;
        case EModeByte:
            return (version < 3) ? 0 : version + 1;
        case EModeKanji:
            return (version < 3) ? 0 : version;
        default:
            return 0;
        }
        return 0;
    }
    if (1 <= version && version <= 9) {
        switch (mode) {
        case EModeNumeric:
            return 10;
        case EModeAlphaNumeric:
            return 9;
        case EModeByte:
            return 8;
        case EModeKanji:
            return 8;
        default:
            return 0;
        }
    } else if (10 <= version && version <= 26) {
        switch (mode) {
        case EModeNumeric:
            return 12;
        case EModeAlphaNumeric:
            return 11;
        case EModeByte:
            return 16;
        case EModeKanji:
            return 10;
        default:
            return 0;
        }
    } else if (27 <= version && version <= 40) {
        switch (mode) {
        case EModeNumeric:
            return 14;
        case EModeAlphaNumeric:
            return 13;
        case EModeByte:
            return 16;
        case EModeKanji:
            return 12;
        default:
            return 0;
        }
    }
    return 0;
}

unsigned int QrmGetMicroModeIndicatorLength(
    UnsignedByte version,
    QrmEncodingMode mode
) {
    if (version < 1 || version > MICROQR_MAX_VERSION) {
        return 0;
    }
    switch (mode) {
    case EModeNumeric:
    case EModeAlphaNumeric:
        return (version < 2) ? 0 : version - 1;
    case EModeByte:
    case EModeKanji:
        return (version < 3) ? 0 : version - 1;
    default:
        return 0;
    }
}

unsigned int QrmGetMicroTerminatorLength(
    UnsignedByte version
) {
    if (version < 1 || version > QR_MAX_VERSION) {
        return 0;
    }
    return version * 2 + 1;
}

UnsignedByte QrmGetMicroQREncodingModeValue(
    QrmEncodingMode mode
) {
    switch (mode) {
    case EModeNumeric:
        return 0;
    case EModeAlphaNumeric:
        return 1;
    case EModeByte:
        return 2;
    case EModeKanji:
        return 3;
    default:
        return 0;
    }
}

UnsignedByte QrmGetMicroQRErrorCorrectionLevelValue(
    QrmErrorCorrectionLevel level,
    UnsignedByte version
) {
    switch (version) {
    case 1:
        return 0;
    case 2:
        switch (level) {
        case ELevelLow:
            return 0b001;
        case ELevelMedium:
            return 0b010;
        default:
            break;
        }
    case 3:
        switch (level) {
        case ELevelLow:
            return 0b011;
        case ELevelMedium:
            return 0b100;
        default:
            break;
        }
    case 4:
        switch (level) {
        case ELevelLow:
            return 0b101;
        case ELevelMedium:
            return 0b110;
        case ELevelQuarter:
            return 0b111;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}

bool QrmCopySingleByte(
    UnsignedByte sourceByte,
    unsigned int sourceStartIndex,
    UnsignedByte* destination,
    unsigned int destStartIndex,
    unsigned int count
) {
    if (sourceStartIndex >= 8 || destStartIndex >= 8) {
        LOG("ERROR: Bit index must be 0...7.");
        return false;
    }
    if (count == 0 || count > (8 - destStartIndex) || count > (8 - sourceStartIndex)) {
        LOG("ERROR: Count must be > 0 and < 8 - sourceStartIndex and < 8 - destStartIndex.");
        return false;
    }
    // Pattern for bits from source
    UnsignedByte pattern = 0x80;
    for (int idx = 1; idx < count; idx += 1) {
        pattern = (pattern >> 1) | 0x80;
    }
    pattern = pattern >> sourceStartIndex;
    // Clean source
    UnsignedByte cleanSource = sourceByte & pattern;
    // Reposition
    if (sourceStartIndex > destStartIndex) {
        unsigned int offset = sourceStartIndex - destStartIndex;
        cleanSource = cleanSource << offset;
        pattern = pattern << offset;
    } else if (sourceStartIndex < destStartIndex) {
        unsigned int offset = destStartIndex - sourceStartIndex;
        cleanSource = cleanSource >> offset;
        pattern = pattern >> offset;
    }
    // Clean destination
    pattern = ~pattern;
    *destination = *destination & pattern;
    // apply
    *destination = *destination | cleanSource;
    return true;
}

bool QrmCopyBits(
    UnsignedByte* source,
    unsigned int sourceLength,
    unsigned int sourceStartIndex,
    bool isSourceOrderReversed,
    UnsignedByte* destination,
    unsigned int destStartIndex,
    unsigned int count
) {
    if (count == 0 || count > sourceLength * 8 - sourceStartIndex) {
        LOG("ERROR: Count must be > 0 and < sourceLength * 8 - sourceStartIndex (bits).");
        return false;
    }

    // Pointer to current destination byte
    UnsignedByte* curDestPtr = destination;
    // Pointer to current source byte
    UnsignedByte* sourcePtr = source;
    // Total bits to write
    unsigned int totalCount = count;

    // Move destination pointer to first byte
    unsigned int destByteIndex = destStartIndex / 8;
    curDestPtr += destByteIndex;
    // First destination bit to write to (from left to right)
    unsigned int destBitIndex = destStartIndex % 8;

    // Calculate direction to move in source bytes
    int step = 1;
    if (isSourceOrderReversed) {
        step = -1;
        sourcePtr = source + sourceLength - 1;
    }
    // Move source pointer to the highest byte
    sourcePtr += step * ((int)sourceStartIndex / 8);
    // Source bit index (from right to left)
    unsigned int sourceBitIndex = sourceStartIndex % 8;

    // Calculate number of bit to read then write for the 1st time
    unsigned int curCount = 8 - destBitIndex;
    unsigned int sourceCurCount =  8 - sourceBitIndex;
    if (curCount > sourceCurCount) {
        curCount = sourceCurCount;
    }
    if (curCount > count) {
        curCount = count;
    }

    while (totalCount > 0) {
        QrmCopySingleByte(*sourcePtr, sourceBitIndex, curDestPtr, destBitIndex, curCount);
        // Should move to next source byte
        sourceBitIndex += curCount;
        if (sourceBitIndex >= 8) {
            sourceBitIndex = 0;
            sourcePtr += step;
        }
        // Should move to next destination byte
        destBitIndex += curCount;
        if (destBitIndex >= 8) {
            curDestPtr += 1;
            destBitIndex = 0;
        }
        // Calculate number of bits will be read
        totalCount -= curCount;
        curCount = totalCount > 8 ? 8 : totalCount;
        if (curCount > count) {
            curCount = count;
        }
        unsigned int offset = 8 - destBitIndex;
        if (curCount > offset) {
            curCount = offset;
        }
        offset = 8 - sourceBitIndex;
        if (curCount > offset) {
            curCount = offset;
        }
    }

    return true;
}

const UnsignedByte* QrmGetAlignmentLocations(UnsignedByte version) {
    if (version < 2 || version > QR_MAX_VERSION) {
        LOG("ERROR: Version must be 2...40");
        return NULL;
    }
    static const UnsignedByte data[][6] = {
        {  18,    0,    0,    0,    0,    0},
        {  22,    0,    0,    0,    0,    0},
        {  26,    0,    0,    0,    0,    0},
        {  30,    0,    0,    0,    0,    0},
        {  34,    0,    0,    0,    0,    0},
        {  22,   38,    0,    0,    0,    0},
        {  24,   42,    0,    0,    0,    0},
        {  26,   46,    0,    0,    0,    0},
        {  28,   50,    0,    0,    0,    0},
        {  30,   54,    0,    0,    0,    0},
        {  32,   58,    0,    0,    0,    0},
        {  34,   62,    0,    0,    0,    0},
        {  26,   46,   66,    0,    0,    0},
        {  26,   48,   70,    0,    0,    0},
        {  26,   50,   74,    0,    0,    0},
        {  30,   54,   78,    0,    0,    0},
        {  30,   56,   82,    0,    0,    0},
        {  30,   58,   86,    0,    0,    0},
        {  34,   62,   90,    0,    0,    0},
        {  28,   50,   72,   94,    0,    0},
        {  26,   50,   74,   98,    0,    0},
        {  30,   54,   78,  102,    0,    0},
        {  28,   54,   80,  106,    0,    0},
        {  32,   58,   84,  110,    0,    0},
        {  30,   58,   86,  114,    0,    0},
        {  34,   62,   90,  118,    0,    0},
        {  26,   50,   74,   98,  122,    0},
        {  30,   54,   78,  102,  126,    0},
        {  26,   52,   78,  104,  130,    0},
        {  30,   56,   82,  108,  134,    0},
        {  34,   60,   86,  112,  138,    0},
        {  30,   58,   86,  114,  142,    0},
        {  34,   62,   90,  118,  146,    0},
        {  30,   54,   78,  102,  126,  150},
        {  24,   50,   76,  102,  128,  154},
        {  28,   54,   80,  106,  132,  158},
        {  32,   58,   84,  110,  136,  162},
        {  26,   54,   82,  110,  138,  166},
        {  30,   58,   86,  114,  142,  170}
    };
    return data[version - 2];
}
