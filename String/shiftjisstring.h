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

#ifndef SHIFTJISSTRING_H
#define SHIFTJISSTRING_H

#include "../QRMatrix/constants.h"
#include "unicodepoint.h"

/// Shift-JIS 2 bytes
typedef struct {
    bool isValid;
    /// Not NULL terminated.
    UnsignedByte* raw;
    unsigned int charCount;
    unsigned int byteCount;
    unsigned int minBytesPerChar;
    unsigned int maxBytesPerChar;
    UnsignedByte* charsMap;
} ShiftJisString;

void SjDestroy(ShiftJisString* string);
ShiftJisString SjDuplicate(ShiftJisString other);
/// Init with raw ShiftJIS bytes
ShiftJisString SjCreate(
    /// Raw bytes
    const UnsignedByte* raw,
    /// Number of bytes
    unsigned int length
);
/// Init from Unicode points
ShiftJisString SjCreateFromUnicodes(const UnicodePoint unicodes);
/// Get the byte index in `rawString` of character at given `index`
/// @return Byte index in rawString
unsigned int SJGetCharacterByte(
    ShiftJisString source,
    /// Character index
    unsigned int index,
    /// To store character size in byte
    UnsignedByte* charSize
);
/// Get Unicode characters code points (decoded data).
UnicodePoint SjToUnicodes(ShiftJisString source);

#endif // SHIFTJISSTRING_H
