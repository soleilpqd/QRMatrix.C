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

#include <stdlib.h>
#include <string.h>
#include "unicodepoint.h"
#include "shiftjisstring.h"
#include "utf8string.h"

void UPDestroy(UnicodePoint* points) {
    if (points->length > 0 && points->raw != NULL) {
        DEALLOC(points->raw);
    }
}

UnicodePoint UPDuplicate(UnicodePoint points) {
    UnicodePoint result;
    result.length = points.length;
    if (result.length > 0) {
        ALLOC_(Unsigned4Bytes, result.raw, result.length);
        for (int index = 0; index < result.length; index += 1) {
            result.raw[index] = points.raw[index];
        }
    } else {
        result.raw = NULL;
    }
    return result;
}

UnicodePoint UPCreate(const Unsigned4Bytes* codes, const unsigned int length) {
    UnicodePoint result;
    result.length = length;
    if (result.length > 0) {
        ALLOC_(Unsigned4Bytes, result.raw, result.length);
        for (int index = 0; index < result.length; index += 1) {
            result.raw[index] = codes[index];
        }
    } else {
        result.raw = NULL;
    }
    return result;
}

UnicodePoint UPCreateEmpty(const unsigned int length) {
    UnicodePoint result;
    result.length = length;
    if (result.length > 0) {
        ALLOC_(Unsigned4Bytes, result.raw, result.length);
    } else {
        result.raw = NULL;
    }
    return result;
}

bool UPIsEqual(UnicodePoint points1, UnicodePoint points2) {
    if (points1.length == points2.length) {
        for (int index = 0; index < points1.length; index += 1) {
            if (points1.raw[index] != points2.raw[index]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

UnicodePoint UPSubstring(UnicodePoint source, unsigned int startIndex, unsigned int length) {
    UnicodePoint result = UPCreateEmpty(length);
    for (unsigned int index = 0; index < length; index += 1) {
        result.raw[index] = source.raw[index + startIndex];
    }
    return result;
}

bool UnicodePoint_testKanji(Unsigned4Bytes point) {
    UnicodePoint unicodes = UPCreate(&point, 1);
    ShiftJisString shiftjis = SjCreateFromUnicodes(unicodes);
    bool result = shiftjis.isValid && shiftjis.minBytesPerChar > 1 && shiftjis.maxBytesPerChar > 1;
    SjDestroy(&shiftjis);
    UPDestroy(&unicodes);
    return result;
}

bool UnicodePoint_testByte(Unsigned4Bytes point) {
    return point <= 0xFF;
}

bool UnicodePoint_testNumeric(Unsigned4Bytes point) {
    return point >= '0' && point <= '9';
}

bool UnicodePoint_testAlphaNumeric(Unsigned4Bytes point) {
    if (UnicodePoint_testByte(point)) {
        static const char *table = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
        char character = (char)point;
        const char* targetPtr = strchr(table, character);
        return targetPtr != NULL;
    }
    return false;
}

QrmEncodingMode UnicodePoint_testMode(
    Unsigned4Bytes* points,
    unsigned int length,
    unsigned int curIndex,
    bool isMicro,
    unsigned int* outLength
) {
    *outLength = 1;
    if (UnicodePoint_testKanji(points[curIndex])) {
        unsigned int limit = isMicro ? 5 : 7;
        unsigned int kanjiCount = 1;
        for (unsigned int index = curIndex + 1; index < length; index += 1) {
            if (UnicodePoint_testKanji(points[index])) {
                kanjiCount += 1;
            } else {
                break;
            }
        }
        if (kanjiCount >= limit) {
            *outLength = kanjiCount;
            return EModeKanji;
        }
        return EModeByte;
    }
    if (UnicodePoint_testNumeric(points[curIndex])) {
        unsigned int limit = isMicro ? 4 : 6;
        unsigned int numericCount = 1;
        for (unsigned int index = curIndex + 1; index < length; index += 1) {
            if (UnicodePoint_testNumeric(points[index])) {
                numericCount += 1;
            } else {
                break;
            }
        }
        if (numericCount >= limit) {
            *outLength = numericCount;
            return EModeNumeric;
        }
    }
    if (UnicodePoint_testAlphaNumeric(points[curIndex])) {
        unsigned int limit = isMicro ? 6 : 8;
        unsigned int alphaNumCount = 1;
        for (unsigned int index = curIndex + 1; index < length; index += 1) {
            if (UnicodePoint_testAlphaNumeric(points[index])) {
                alphaNumCount += 1;
            } else {
                break;
            }
        }
        if (alphaNumCount >= limit) {
            *outLength = alphaNumCount;
            return EModeAlphaNumeric;
        }
    }
    return EModeByte;
}

QrmSegment* UPMakeSegments(UnicodePoint points, QrmErrorCorrectionLevel level, unsigned int* length, bool isMicro) {
    if (points.length == 0) {
        return NULL;
    }
    if (isMicro && level == ELevelHigh) {
        LOG("Invalid Error Correction Level for MicroQR");
        return NULL;
    }

    ALLOC(QrmEncodingMode, segmentModes, points.length);
    ALLOC(unsigned int, segmentLengths, points.length);
    unsigned int segmentIndex = 0;
    unsigned int byteModeCount = 0;

    unsigned int segmentLen = 0;
    QrmEncodingMode mode = UnicodePoint_testMode(points.raw, points.length, 0, isMicro, &segmentLen);

    if (mode != EModeByte) {
        segmentModes[segmentIndex] = mode;
        segmentLengths[segmentIndex] = segmentLen;
        segmentIndex += 1;
    } else {
        byteModeCount = 1;
    }
    for (unsigned int index = segmentLen; index < points.length; index += 1) {
        mode = UnicodePoint_testMode(points.raw, points.length, index, isMicro, &segmentLen);
        if (mode != EModeByte) {
            if (byteModeCount > 0) {
                segmentModes[segmentIndex] = EModeByte;
                segmentLengths[segmentIndex] = byteModeCount;
                segmentIndex += 1;
                byteModeCount = 0;
            }
            segmentModes[segmentIndex] = mode;
            segmentLengths[segmentIndex] = segmentLen;
            segmentIndex += 1;
            index = index + segmentLen - 1;
        } else {
            byteModeCount += 1;
        }
    }
    if (byteModeCount > 0) {
        segmentModes[segmentIndex] = EModeByte;
        segmentLengths[segmentIndex] = byteModeCount;
        segmentIndex += 1;
    }

    ALLOC(QrmSegment, result, segmentIndex);
    unsigned int offset = 0;
    for (unsigned int index = 0; index < segmentIndex; index += 1) {
        QrmEncodingMode segMode = segmentModes[index];
        unsigned int segLen = segmentLengths[index];
        UnicodePoint segCodes = UPSubstring(points, offset, segLen);
        switch (segMode) {
        case EModeNumeric:
        case EModeAlphaNumeric:
        case EModeByte: {
            Utf8String utf8 = U8CreateFromUnicodes(segCodes);
            QrmSegFill(&result[index], segMode, utf8.raw, utf8.byteCount, DEFAULT_ECI_ASSIGMENT);
            U8Destroy(&utf8);
        }
            break;
        case EModeKanji: {
            ShiftJisString shiftjis = SjCreateFromUnicodes(segCodes);
            QrmSegFill(&result[index], segMode, shiftjis.raw, shiftjis.byteCount, DEFAULT_ECI_ASSIGMENT);
            SjDestroy(&shiftjis);
        }
            break;
        }
        UPDestroy(&segCodes);
        offset += segLen;
    }
    *length = segmentIndex;
    DEALLOC(segmentModes);
    DEALLOC(segmentLengths);;
    return result;
}
