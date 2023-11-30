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

#include "kanjiencoder.h"
#include "../common.h"

unsigned int QrmKanjiEncode(
    const UnsignedByte* text,
    unsigned int length,
    UnsignedByte* buffer,
    unsigned int startIndex
) {
    unsigned int bitIndex = startIndex;
    for (unsigned int index = 0; index < length; index += 2) {
        UnsignedByte* ptr = (UnsignedByte*)text + index;
        Unsigned2Bytes charWord = 0;
        UnsignedByte* charWordPtr = (UnsignedByte*)&charWord;
        if (qrmIsLittleEndian) {
            *charWordPtr = *(ptr + 1);
            *(charWordPtr + 1) = *ptr;
        } else {
            *charWordPtr = *ptr;
            *(charWordPtr + 1) = *(ptr + 1);
        }
        Unsigned2Bytes offset = 0;
        if (charWord >= 0x8140 && charWord <= 0x9FFC) {
            offset = 0x8140;
        } else if (charWord >= 0xE040 && charWord <= 0xEBBF) {
            offset = 0xC140;
        } else {
            LOG("ERROR: Unknown or unsupported kanji character.");
            break;
        }
        charWord = charWord - offset;
        Unsigned2Bytes lsByte = 0;
        Unsigned2Bytes msByte = 0;
        if (qrmIsLittleEndian) {
            lsByte = (Unsigned2Bytes)*charWordPtr;
            msByte = (Unsigned2Bytes)*(charWordPtr + 1);
        } else {
            lsByte = (Unsigned2Bytes)*(charWordPtr + 1);
            msByte = (Unsigned2Bytes)*charWordPtr;
        }
        charWord = (msByte * 0xC0) + lsByte;
        QrmCopyBits(charWordPtr, 2, 3 /* 16 - 13 */, qrmIsLittleEndian, buffer, bitIndex, 13);
        bitIndex += 13;
    }
    return bitIndex - startIndex;
}
