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

#include "qrmatrixsegment.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Data Validation ----------------------------------------------------------------------------------------------------------------------------------

bool QrmSeg_validateNumeric(const UnsignedByte* data, unsigned int length) {
    static const char* pattern = "0123456789";
    for (unsigned int index = 0; index < length; index += 1) {
        UnsignedByte byte = data[index];
        if (strchr(pattern, (char)byte) == NULL) {
            return false;
        }
    }
    return true;
}

bool QrmSeg_validateAlphaNumeric(const UnsignedByte* data, unsigned int length) {
    static const char* pattern = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
    for (unsigned int index = 0; index < length; index += 1) {
        UnsignedByte byte = data[index];
        if (strchr(pattern, (char)byte) == NULL) {
            return false;
        }
    }
    return true;
}

bool QrmSeg_validateKanji(const UnsignedByte* data, unsigned int length) {
    // Only accept 2 bytes ShiftJIS characters
    if ((length % 2) > 0) {
        return false;
    }
    for (unsigned int index = 0; index < length; index += 2) {
        Unsigned2Bytes curChar = 0;
        UnsignedByte* curCharPtr = (UnsignedByte*)&curChar;
        if (qrmIsLittleEndian) {
            *curCharPtr = data[index + 1];
            *(curCharPtr + 1) = data[index];
        } else {
            *curCharPtr = data[index];
            *(curCharPtr + 1) = data[index + 1];
        }
        bool isValid = (
            (curChar >= 0x8140) && (curChar <= 0x9FFC)) ||
            ((curChar >= 0xE040) && (curChar <= 0xEBBF)
        );
        if (!isValid) {
            return false;
        }
    }
    return true;
}

bool QrmSeg_validateInputBytes(QrmEncodingMode mode, const UnsignedByte* data, unsigned int length) {
    switch (mode) {
    case EModeNumeric:
        return QrmSeg_validateNumeric(data, length);
        break;
    case EModeAlphaNumeric:
        return QrmSeg_validateAlphaNumeric(data, length);
        break;
    case EModeKanji:
        return QrmSeg_validateKanji(data, length);
        break;
    default:
        break;
    }
    return true;
}

// PUBLIC -------------------------------------------------------------------------------------------------------------------------------------------

void QrmSegDestroy(QrmSegment* segment) {
    if (segment->data != NULL && segment->length > 0) {
        DEALLOC(segment->data);
        segment->length = 0;
    }
}

QrmSegment QrmSegCreate(QrmEncodingMode mode, const UnsignedByte *data, unsigned int length, unsigned int eciIndicator) {
    if (!QrmSeg_validateInputBytes(mode, data, length)) {
        return QrmSegCreateEmpty();
    }
    QrmSegment result;
    result.length = length;
    result.mode = mode;
    result.eci = eciIndicator;
    if (result.length > 0 && data != NULL) {
        ALLOC_(UnsignedByte, result.data, result.length);
        for (unsigned int index = 0; index < result.length; index += 1) {
            result.data[index] = data[index];
        }
    } else {
        result.length = 0;
        result.data = NULL;
    }
    return result;
}

QrmSegment QrmSegCreateEmpty() {
    QrmSegment result;
    result.mode = 0;
    result.length = 0;
    result.data = NULL;
    result.eci = DEFAULT_ECI_ASSIGMENT;
    return result;
}

void QrmSegFill(QrmSegment* segment, QrmEncodingMode mode, const UnsignedByte* data, unsigned int length, unsigned int eciIndicator) {
    if (!QrmSeg_validateInputBytes(mode, data, length)) {
        return;
    }
    QrmSegDestroy(segment);
    segment->length = length;
    segment->mode = mode;
    segment->eci = eciIndicator;
    if (length > 0 && data != NULL) {
        ALLOC_(UnsignedByte, segment->data, segment->length);
        for (unsigned int index = 0; index < segment->length; index += 1) {
            segment->data[index] = data[index];
        }
    } else {
        segment->data = NULL;
        segment->length = 0;
    }
}

void QrmSegCopy(QrmSegment* segment, QrmSegment other) {
    QrmSegFill(segment, other.mode, other.data, other.length, other.eci);
}

QrmSegment QrmSegDuplicate(QrmSegment other) {
    QrmSegment result;
    result.length = other.length;
    result.mode = other.mode;
    result.eci = other.eci;
    if (result.length > 0 && other.data != NULL) {
        ALLOC_(UnsignedByte, result.data, result.length);
        for (unsigned int index = 0; index < result.length; index += 1) {
            result.data[index] = other.data[index];
        }
    } else {
        result.data = NULL;
        result.length = 0;
    }
    return result;
}
