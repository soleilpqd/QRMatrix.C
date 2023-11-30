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
#include "shiftjisstring.h"
#include "shiftjisstringmap.h"

void SjDestroy(ShiftJisString* string) {
    if (string->byteCount > 0) {
        if (string->raw != NULL) {
            DEALLOC(string->raw);
        }
        if (string->charsMap != NULL) {
            DEALLOC(string->charsMap);
        }
    }
}

ShiftJisString SjDuplicate(ShiftJisString other) {
    ShiftJisString result;
    result.isValid = other.isValid;
    result.charCount = other.charCount;
    result.byteCount = other.byteCount;
    result.minBytesPerChar = other.minBytesPerChar;
    result.maxBytesPerChar = other.maxBytesPerChar;
    if (result.byteCount > 0 && result.charCount > 0) {
        ALLOC_(UnsignedByte, result.raw, result.byteCount);
        for (unsigned int index = 0; index < result.byteCount; index += 1) {
            result.raw[index] = other.raw[index];
        }
        ALLOC_(UnsignedByte, result.charsMap, result.charCount);
        for (unsigned int index = 0; index < result.charCount; index += 1) {
            result.charsMap[index] = other.charsMap[index];
        }
    } else {
        result.isValid = false;
        result.byteCount = 0;
        result.charCount = 0;
        result.raw = NULL;
        result.charsMap = NULL;
    }
    return result;
}

ShiftJisString SjCreate(
    const UnsignedByte* raw,
    unsigned int length
) {
    ShiftJisString result;
    result.isValid = true;
    result.charCount = 0;
    result.byteCount = length;
    result.minBytesPerChar = 2;
    result.maxBytesPerChar = 0;
    if (length > 0 && raw != NULL) {
        ALLOC_(UnsignedByte, result.charsMap, length);
        ALLOC_(UnsignedByte, result.raw, length);
        unsigned int indexToCheck = 0;
        for (unsigned int index = 0; index < length; index += 1) {
            UnsignedByte byte = raw[index];
            if (index == indexToCheck) {
                UnsignedByte charSize = 0;
                if ((byte >= 0x20 && byte <= 0x7E) || (byte >= 0xA1 && byte <= 0xDF) || byte == '\r' || byte == '\n' || byte == '\t') {
                    // Single byte character
                    charSize = 1;
                } else if ((byte >= 0x81 && byte <= 0x9F) || (byte >= 0xE0 && byte <= 0xFC)) {
                    // Double bytes character
                    charSize = 2;
                } else {
                    LOG("ERROR: Given data contains invalid byte.");
                    result.isValid = false;
                    break;
                }
                result.charsMap[result.charCount] = charSize;
                result.charCount += 1;
                if (result.maxBytesPerChar < charSize) {
                    result.maxBytesPerChar = charSize;
                }
                if (result.minBytesPerChar > charSize) {
                    result.minBytesPerChar = charSize;
                }
                indexToCheck += charSize;
            } else {
                // Second byte of double bytes characters
                if ((byte < 0x40) || (byte == 0x7F) || (byte > 0xFC)) {
                    LOG("ERROR: Given data contains invalid byte.");
                    result.isValid = false;
                    break;
                }
            }
            result.raw[index] = byte;
        }
    } else {
        result.isValid = false;
        result.byteCount = 0;
        result.charCount = 0;
        result.charsMap = NULL;
        result.raw = NULL;
    }
    return result;
}

ShiftJisString SjCreateFromUnicodes(const UnicodePoint unicodes) {
    ShiftJisString result;
    result.isValid = false;
    result.charCount = unicodes.length;
    result.byteCount = 0;
    result.minBytesPerChar = 2;
    result.maxBytesPerChar = 0;

    if (result.charCount == 0) {
        result.charsMap = NULL;
        result.raw = NULL;
        return result;
    }

    Unsigned2Bytes (*map1)[188] = ShiftJisString_KanjiUnicode1Map(); // 31
    Unsigned2Bytes (*map2)[188] = ShiftJisString_KanjiUnicode2Map(); // 29

    ALLOC_(UnsignedByte, result.charsMap, result.charCount);

    ALLOC(UnsignedByte, buffer, result.charCount * 2);
    UnsignedByte* bufferPtr = buffer;

    const Unsigned4Bytes *points = unicodes.raw;
    for (unsigned int index = 0; index < result.charCount; index += 1) {
        Unsigned4Bytes point = points[index];
        UnsignedByte charSize = 0;
        // Special case
        if (point == 0xA5) { // '¥'
            *bufferPtr = 0x5C; // '\'
            charSize = 1;
        } else if (point == 0x203E) { // '‾'
            *bufferPtr = 0x7E; // '~'
            charSize = 1;
        } else if (point == 0x7E) { // '~'
            *bufferPtr = 0x81;
            *(bufferPtr + 1) = 0x60; // '～'
            charSize = 2;
        } else  if (point == 0x5C) { // '\'
            *bufferPtr = 0x81;
            *(bufferPtr + 1) = 0x5F; // '＼'
            charSize = 2;
        } else if ((point >= 0x20 && point < 0x7E) || (point >= 0xFF61 && point <= 0xFF9F) || point == '\r' || point == '\n' || point == '\t') {
            // Single byte character
            charSize = 1;
            if (point < 0x7E) {
                *bufferPtr = (UnsignedByte)point;
            } else {
                Unsigned4Bytes tmp = point - 0xFF61 + 0xA1;
                *bufferPtr = (UnsignedByte)tmp;
            }
        } else {
            UnsignedByte firstByte = 0;
            UnsignedByte secondByte = 0;
            for (UnsignedByte idx = 0; idx < 31; idx += 1) {
                for (UnsignedByte jdx = 0; jdx < 188; jdx += 1) {
                    Unsigned2Bytes code = map1[idx][jdx];
                    if (point == code) {
                        firstByte = idx + 0x81;
                        secondByte = jdx + 0x40;
                        if (secondByte >= 0x7F) {
                            secondByte += 1;
                        }
                    }
                }
            }
            if (firstByte == 0) {
                for (UnsignedByte idx = 0; idx < 29; idx += 1) {
                    for (UnsignedByte jdx = 0; jdx < 188; jdx += 1) {
                        Unsigned2Bytes code = map2[idx][jdx];
                        if (point == code) {
                            firstByte = idx + 0xE0;
                            secondByte = jdx + 0x40;
                            if (secondByte >= 0x7F) {
                                secondByte += 1;
                            }
                        }
                    }
                }
            }
            if (firstByte == 0) {
                LOG("ERROR: Given data contains invalid character.");
                break;
            }
            // Double bytes character
            charSize = 2;
            *bufferPtr = firstByte;
            *(bufferPtr + 1) = secondByte;
        }
        result.charsMap[index] = charSize;
        if (result.maxBytesPerChar < charSize) {
            result.maxBytesPerChar = charSize;
        }
        if (result.minBytesPerChar > charSize) {
            result.minBytesPerChar = charSize;
        }
        result.byteCount += charSize;
        bufferPtr += 2;
    }

    ALLOC_(UnsignedByte, result.raw, result.byteCount);
    UnsignedByte *rawPtr = result.raw;
    bufferPtr = buffer;

    for (unsigned int index = 0; index < result.charCount; index += 1) {
        UnsignedByte charSize = result.charsMap[index];
        *rawPtr = *bufferPtr;
        if (charSize == 2) {
            *(rawPtr + 1) = *(bufferPtr + 1);
        }
        rawPtr += charSize;
        bufferPtr += 2;
    }

    DEALLOC(buffer);
    result.isValid = true;
    return result;
}

unsigned int SJGetCharacterByte(
    ShiftJisString source,
    unsigned int index,
    UnsignedByte* charSize
) {
    if (index >= source.charCount) {
        LOG("ERROR: Position must be in range of string charactersCount");
        return 0;
    }
    if (!source.isValid) {
        LOG("ERROR: This object is invalid so can not perform the requested action.");
        return 0;
    }
    unsigned int posBytes = 0;
    for (unsigned int idx = 0; idx < index; idx += 1) {
        UnsignedByte curCharBytes = source.charsMap[idx];
        posBytes += curCharBytes;
    }
    *charSize = source.charsMap[index];
    return posBytes;
}

UnicodePoint SjToUnicodes(ShiftJisString source) {
    if (!source.isValid && source.charCount == 0) {
        LOG("ERROR: This object is invalid so can not perform the requested action.");
        return UPCreateEmpty(0);
    }
    UnicodePoint result = UPCreateEmpty(source.charCount);
    UnsignedByte* charPtr = source.raw;
    Unsigned2Bytes (*map1)[188] = ShiftJisString_KanjiUnicode1Map();
    Unsigned2Bytes (*map2)[188] = ShiftJisString_KanjiUnicode2Map();

    for (unsigned int index = 0; index < source.charCount; index += 1) {
        UnsignedByte charSize = source.charsMap[index];
        UnsignedByte curByte = *charPtr;
        Unsigned4Bytes point = '?';
        if (charSize == 1) {
            if (curByte == 0x5C) { // '\'
                point = 0xA5; // '¥'
            } else if (curByte == 0x7E) { // '~'
                point = 0x203E; // '‾'
            } else if (curByte < 0x7E) {
                point = curByte;
            } else if (curByte >= 0xA1 && curByte <= 0xDF) {
                point = curByte - 0xA1 + 0xFF61;
            }
        } else {
            UnsignedByte nextByte = *(charPtr + 1);
            UnsignedByte secondIndex = nextByte - 0x40;
            if (nextByte > 0x7F) {
                secondIndex -= 1;
            }
            if (curByte >= 0xE0) {
                UnsignedByte firstIndex = curByte - 0xE0;
                Unsigned2Bytes value = map2[firstIndex][secondIndex];
                if (value > 0) {
                    point = (Unsigned4Bytes)value;
                }
            } else if (curByte >= 0x81) {
                UnsignedByte firstIndex = curByte - 0x81;
                Unsigned2Bytes value = map1[firstIndex][secondIndex];
                if (value > 0) {
                    point = (Unsigned4Bytes)value;
                }
            }
        }
        result.raw[index] = point;
        charPtr += charSize;
    }
    return result;
}
