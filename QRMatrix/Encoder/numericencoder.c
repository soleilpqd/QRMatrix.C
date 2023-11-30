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

#include "numericencoder.h"
#include "../common.h"
#include <string.h>
#include <stdlib.h>

unsigned int QrmNumericEncode(const UnsignedByte* text, unsigned int length, UnsignedByte* buffer, unsigned int startIndex) {
    int index = 0;
    unsigned int bitIndex = startIndex;
    while (index < length) {
        unsigned int groupLen;
        int offset  = length - index;
        if (offset > 2) {
            groupLen = 3;
        } else if (offset > 1) {
            groupLen = 2;
        } else {
            groupLen = 1;
        }
        ALLOC(UnsignedByte, group, groupLen + 1);
        for (unsigned int idx = 0; idx < groupLen; idx += 1) {
            group[idx] = text[index + idx];
        }
        index += groupLen;
        unsigned int value = atoi((const char*)group);
        DEALLOC(group);
        unsigned int bitLen;
        switch (groupLen) {
        case 3:
            bitLen = NUM_TRIPLE_DIGITS_BITS_LEN;
            break;
        case 2:
            bitLen = NUM_DOUBLE_DIGITS_BITS_LEN;
            break;
        case 1:
            bitLen = NUM_SINGLE_DIGIT_BITS_LEN;
            break;
        default:
            bitLen = 0;
            break;
        }
        unsigned int encodedSize = sizeof(unsigned int);
        unsigned int encodedOffset = encodedSize * 8 - bitLen;
        QrmCopyBits(
            (UnsignedByte*)&value,
            encodedSize,
            encodedOffset,
            qrmIsLittleEndian,
            buffer,
            bitIndex,
            bitLen
        );
        bitIndex += bitLen;
    }
    return bitIndex - startIndex;
}
