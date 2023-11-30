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

#ifndef UNICODEPOINT_H
#define UNICODEPOINT_H

#include "../QRMatrix/constants.h"
#include "../QRMatrix/qrmatrixsegment.h"

typedef struct {
    Unsigned4Bytes* raw;
    unsigned int length;
} UnicodePoint;

void UPDestroy(UnicodePoint* points);
UnicodePoint UPDuplicate(UnicodePoint points);
UnicodePoint UPCreate(const Unsigned4Bytes* codes, const unsigned int length);
UnicodePoint UPCreateEmpty(const unsigned int length);
bool UPIsEqual(UnicodePoint points1, UnicodePoint points2);
UnicodePoint UPSubstring(UnicodePoint source, unsigned int startIndex, unsigned int length);

/// Simple auto make segments.
/// Result must be delete when done.
/// Return NULL if 0 length.
/// (this is just experimental function:
/// Base on Byte mode; switch to other mode (create new segment)
/// if found a segment of text which makes better encoded data
/// )
QrmSegment* UPMakeSegments(UnicodePoint points, QrmErrorCorrectionLevel level, unsigned int* length, bool isMicro);

#endif // UNICODEPOINT_H
