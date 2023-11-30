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

#ifndef UTF8STRING_H
#define UTF8STRING_H

#include "../QRMatrix/constants.h"
#include "unicodepoint.h"

/// Handle unicode (UTF-8) from std::string
typedef struct {
    bool isValid;
    UnsignedByte* raw;
    unsigned int charCount;
    unsigned int byteCount;
    unsigned int maxBytesPerChar;
    UnsignedByte* charsMap;
} Utf8String;

void U8Destroy(Utf8String* string);
Utf8String U8Copy(Utf8String other);
/// Init from bytes of UTF-8 encoded string.
Utf8String U8Create(
    /// Bytes.
    const UnsignedByte* raw,
    /// Number of bytes (leave 0 if `raw` is C-String (null-terminated).
    const unsigned int length
);
/// Init from Unicode codes (Encode given Unicode characters into UTF-8)
Utf8String U8CreateFromUnicodes(
    /// Array of codes
    const UnicodePoint unicodes
);
/// Get Unicode characters code points (decoded data).
UnicodePoint U8ToUnicodes(Utf8String source);

#endif // UTF8STRING_H
