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
#include "utf8string.h"
#include "../QRMatrix/common.h"

#define SINGLE_BYTE_MASK        0b10000000
#define SINGLE_BYTE_PREFIX      0b00000000
#define DOUBLE_BYTES_MASK       0b11100000
#define DOUBLE_BYTES_PREFIX     0b11000000
#define TRIPLE_BYTES_MASK       0b11110000
#define TRIPLE_BYTES_PREFIX     0b11100000
#define QUADRUPLE_BYTES_MASK    0b11111000
#define QUADRUPLE_BYTES_PREFIX  0b11110000

#define SECONDARY_BYTE_MASK     0b11000000
#define SECONDARY_BYTE_PREFIX   0b10000000

#define SINGLE_BYTE_LAST_CODEPOINT      0x0000007F
#define DOUBLE_BYTES_LAST_CODEPOINT     0x000007FF
#define TRIPLE_BYTES_LAST_CODEPOINT     0x0000FFFF
#define QUADRUPLE_BYTES_LAST_CODEPOINT  0x0010FFFF

void U8Destroy(Utf8String* string) {
    if (string->byteCount > 0) {
        if (string->raw != NULL) {
            DEALLOC(string->raw);
        }
        if (string->charsMap != NULL) {
            DEALLOC(string->charsMap);
        }
    }
}

Utf8String U8Copy(Utf8String other) {
    Utf8String result;
    result.isValid = other.isValid;
    result.byteCount = other.byteCount;
    result.charCount = other.charCount;
    result.maxBytesPerChar = other.maxBytesPerChar;
    if (result.byteCount > 0 && result.charCount > 0) {
        ALLOC_(UnsignedByte, result.raw, other.byteCount);
        ALLOC_(UnsignedByte, result.charsMap, other.charCount);
        for (unsigned int index = 0; index < result.charCount; index += 1) {
            result.charsMap[index] = other.charsMap[index];
        }
        for (unsigned int index = 0; index < result.byteCount; index += 1) {
            result.raw[index] = other.raw[index];
        }
    } else {
        result.isValid = false;
        result.charCount = 0;
        result.charsMap = NULL;
        result.byteCount = 0;
        result.raw = NULL;
    }

    return result;
}

Utf8String U8Create(
    const UnsignedByte* raw,
    const unsigned int length
) {
    Utf8String result;
    result.isValid = false;
    result.charCount = 0;
    result.maxBytesPerChar = 0;
    unsigned int bytesCount = length;
    if (bytesCount == 0) {
        bytesCount = (unsigned int)strlen((char*)raw);
    }
    result.byteCount = bytesCount;

    if (result.byteCount == 0 || raw == NULL) {
        result.byteCount = 0;
        result.charCount = 0;
        result.raw = NULL;
        result.charsMap = NULL;
        return result;
    }

    ALLOC_(UnsignedByte, result.raw, bytesCount + 1); // +1 for null terminator
    ALLOC_(UnsignedByte, result.charsMap, bytesCount);

    int byteToCheck = 0;
    for (int index = 0; index < bytesCount; index += 1) {
        UnsignedByte currentByte = raw[index];
        result.raw[index] = currentByte;
        if (index == byteToCheck) {
            UnsignedByte charSize = 0;
            if ((currentByte & SINGLE_BYTE_MASK) == SINGLE_BYTE_PREFIX) {
                charSize = 1;
            } else if ((currentByte & DOUBLE_BYTES_MASK) == DOUBLE_BYTES_PREFIX) {
                charSize = 2;
            } else if ((currentByte & TRIPLE_BYTES_MASK) == TRIPLE_BYTES_PREFIX) {
                charSize = 3;
            } else if ((currentByte & QUADRUPLE_BYTES_MASK) == QUADRUPLE_BYTES_PREFIX) {
                charSize = 4;
            } else {
                LOG("ERROR: Input bytes seem be not UTF-8.");
                return result;
            }
            byteToCheck += charSize;
            if (result.maxBytesPerChar < charSize) {
                result.maxBytesPerChar = charSize;
            }
            result.charsMap[result.charCount] = charSize;
            result.charCount += 1;
        } else if ((currentByte & SECONDARY_BYTE_MASK) != SECONDARY_BYTE_PREFIX) {
            LOG("ERROR: Input bytes seem be not UTF-8.");
            return result;
        }
    }
    result.isValid = true;
    return result;
}

unsigned char* U8String_fromUnicode(UnsignedByte* codePtr, UnsignedByte* destPtr, const unsigned char charSize) {
    unsigned char prefix = 0;
    switch (charSize) {
    case 2:
        prefix = DOUBLE_BYTES_PREFIX;
        break;
    case 3:
        prefix = TRIPLE_BYTES_PREFIX;
        break;
    case 4:
        prefix = QUADRUPLE_BYTES_PREFIX;
        break;
    default:
        break;
    }
    UnsignedByte secondaryPrefix = SECONDARY_BYTE_PREFIX;
    unsigned int prefixBitLen = charSize + 1;
    const unsigned int sourceSize = 4 * 8;
    // Fist byte
    unsigned int firstByteDataLen = 8 - prefixBitLen;
    unsigned int sourceIndex = sourceSize - firstByteDataLen - ((unsigned int)charSize - 1) * 6;
    *destPtr = prefix;
    QrmCopyBits(codePtr, 4, sourceIndex, qrmIsLittleEndian, destPtr, prefixBitLen, firstByteDataLen);
    destPtr += 1;
    sourceIndex += firstByteDataLen;
    // Next bytes
    for (int index = 0; index < charSize - 1; index += 1) {
        *destPtr = secondaryPrefix;
        QrmCopyBits(codePtr, 4, sourceIndex, qrmIsLittleEndian, destPtr, 2, 6);
        destPtr += 1;
        sourceIndex += 6;
    }
    return destPtr;
}

Utf8String U8CreateFromUnicodes(
    const UnicodePoint unicodes
) {
    const Unsigned4Bytes* codes = unicodes.raw;
    unsigned int length = unicodes.length;

    Utf8String result;
    result.charCount = 0;
    result.maxBytesPerChar = 0;
    result.isValid = false;

    if (length == 0 || codes == NULL) {
        result.byteCount = 0;
        result.charCount = 0;
        result.raw = NULL;
        result.charsMap = NULL;
        return result;
    }

    ALLOC_(UnsignedByte, result.charsMap, length);
    unsigned int bytesCount = 0;
    for (int index = 0; index < length; index += 1) {
        unsigned char charSize = 0;
        Unsigned4Bytes code = codes[index];
        if (code <= SINGLE_BYTE_LAST_CODEPOINT) {
            charSize = 1;
        } else if (code <= DOUBLE_BYTES_LAST_CODEPOINT) {
            charSize = 2;
        } else if (code <= TRIPLE_BYTES_LAST_CODEPOINT) {
            charSize = 3;
        } else {
            charSize = 4;
        }
        if (result.maxBytesPerChar < charSize) {
            result.maxBytesPerChar = charSize;
        }
        result.charsMap[result.charCount] = charSize;
        bytesCount += charSize;
        result.charCount += 1;
    }

    ALLOC_(UnsignedByte, result.raw, bytesCount + 1); // +1 for null terminator
    result.byteCount = bytesCount;
    UnsignedByte* ptr = result.raw;
    for (int index = 0; index < length; index += 1) {
        UnsignedByte charSize = result.charsMap[index];
        Unsigned4Bytes code = codes[index];
        UnsignedByte* codePtr = (UnsignedByte*)&code;
        if (charSize == 1) {
            *ptr = (UnsignedByte)code;
            ptr += 1;
        } else {
            ptr = U8String_fromUnicode(codePtr, ptr, charSize);
        }
    }

    result.isValid = true;
    return result;
}

Unsigned4Bytes U8String_toUnicode(UnsignedByte* source, UnsignedByte charSize) {
    unsigned int u4BSize = (unsigned int)sizeof(Unsigned4Bytes);
    Unsigned4Bytes result = 0;
    UnsignedByte* resultPtr = (UnsignedByte*)&result;
    unsigned int resultBitIndex = u4BSize * 8;
    if (charSize == 1) {
        result = (Unsigned4Bytes)*source;
    } else {
        UnsignedByte* lastByte = source + charSize - 1;
        for (int index = 0; index < charSize; index += 1) {
            UnsignedByte *curByte = lastByte - index;
            unsigned int prefixBitCount = 2;
            if (curByte == source) {
                prefixBitCount = charSize + 1;
            }
            unsigned int bitCount = 8 - prefixBitCount;
            resultBitIndex -= bitCount;
            QrmCopyBits(curByte, 1, prefixBitCount, qrmIsLittleEndian, resultPtr, resultBitIndex, bitCount);
        }
        if (qrmIsLittleEndian) { // Reverse bytes
            Unsigned4Bytes tmp = result;
            UnsignedByte* tmpPtr = (UnsignedByte*)&tmp;
            for (int index = 0; index < u4BSize; index += 1) {
                resultPtr[index] = tmpPtr[u4BSize - index - 1];
            }
        }
    }
    return result;
}

UnicodePoint U8ToUnicodes(Utf8String source) {
    if (!source.isValid || source.byteCount == 0 || source.raw == NULL) {
        LOG("ERROR: This object is invalid so can not perform the requested action.");
        return UPCreateEmpty(0);
    }
    UnicodePoint result = UPCreateEmpty(source.charCount);
    UnsignedByte* charPtr = source.raw;

    for (int index = 0; index < source.charCount; index += 1) {
        UnsignedByte charSize = source.charsMap[index];
        Unsigned4Bytes value = U8String_toUnicode(charPtr, charSize);
        result.raw[index] = value;
        charPtr += charSize;
    }
    return result;
}
