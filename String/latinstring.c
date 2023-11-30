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
#include "latinstring.h"

void LtDestroy(LatinString* string) {
    if (string->length > 0 && string->raw != NULL) {
        DEALLOC(string->raw);
        string->length = 0;
    }
}

LatinString LtDuplicate(LatinString other) {
    LatinString result;
    result.isValid = other.isValid;
    result.length = other.length;
    if (result.length > 0) {
        ALLOC_(UnsignedByte, result.raw, result.length);
        for (unsigned int index = 0; index < result.length; index += 1) {
            result.raw[index] = other.raw[index];
        }
    } else {
        result.raw = NULL;
    }
    return result;
}

LatinString LtCreate(
    const UnsignedByte* raw,
    const unsigned int length
) {
    LatinString result;
    result.length = length;
    if (result.length > 0) {
        ALLOC_(UnsignedByte, result.raw, length);
        result.isValid = true;
        for (unsigned int index = 0; index < length; index += 1) {
            UnsignedByte value = raw[index];
            result.raw[index] = value;
            if ((value < 0x20 || (0x7E < value && value < 0xA0)) && value != '\n' && value != '\r' && value != '\t') {
                result.isValid = false;
                LOG("ERROR: Given bytes contains byte out of range of ISO/IEC 8859-1 charset.");
                break;
            }
        }
    } else{
        result.isValid = false;
        result.raw = NULL;
    }
    return result;
}

LatinString LtCreateFromUnicodes(const UnicodePoint unicodes) {
    LatinString result;
    result.length = unicodes.length;
    if (result.length > 0) {
        ALLOC_(UnsignedByte, result.raw, result.length);
        result.isValid = true;
        const Unsigned4Bytes* points = unicodes.raw;
        for (unsigned int index = 0; index < result.length; index += 1) {
            Unsigned4Bytes value = points[index];
            result.raw[index] = (UnsignedByte)value;
            if ((value < 0x20 || (0x7E < value && value < 0xA0) || value > 0xFF) && value != '\n' && value != '\r' && value != '\t') {
                result.isValid = false;
                LOG("ERROR: Given unicodes contain character out of range of ISO/IEC 8859-1 charset.");
                break;
            }
        }
    } else {
        result.isValid = false;
        result.raw = NULL;
    }
    return result;
}

UnicodePoint LtToUnicodes(LatinString source) {
    if (!source.isValid || source.length == 0) {
        LOG("This object is invalid so can not perform the requested action.");
        return UPCreateEmpty(0);
    }
    UnicodePoint result = UPCreateEmpty(source.length);
    for (unsigned int index = 0; index < source.length; index += 1) {
        Unsigned4Bytes value = (Unsigned4Bytes)source.raw[index];
        result.raw[index] = value;
    }
    return result;
}
