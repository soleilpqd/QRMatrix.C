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

#ifndef QRMATRIXSEGMENT_H
#define QRMATRIXSEGMENT_H

#include "constants.h"

#define DEFAULT_ECI_ASSIGMENT 3

/// Hold information of input data for 1 QR segment.
typedef struct {
    QrmEncodingMode mode;
    unsigned int length;
    UnsignedByte* data;
    unsigned int eci;
} QrmSegment;

/// Destructor
void QrmSegDestroy(QrmSegment* segment);
/// Create QR segment
QrmSegment QrmSegCreate(
    /// Encoding mode
    QrmEncodingMode mode,
    /// Bytes sequence to encode.
    /// If mode is `Numeric`, `data` must contain only ASCII bytes of characters `[0...9]`
    /// (or other text encoding if compatiple eg. UTF-8).
    /// If mode is `AlphaNumeric, `data` must contain only ASCII bytes of characters `[0...9][A...Z] $%*+-./:`
    /// (or other text encoding if compatiple eg. UTF -8).
    /// If mode is `Kanji`, `data` must contain only 2-bytes ShiftJIS characters
    /// (each 2-bytes must be in range [0x8140...0x9FFC] & [0xE040...0xEBBF]).
    const UnsignedByte* data,
    /// Number of `data` bytes
    unsigned int length,
    /// Enable ECI mode with given ECI Indicator (ECI Assigment value)
    unsigned int eciIndicator
);
/// Create empty and set later
QrmSegment QrmSegCreateEmpty(void);
/// Fill segment with given data
void QrmSegFill(QrmSegment* segment, QrmEncodingMode mode, const UnsignedByte* data, unsigned int length, unsigned int eciIndicator);
void QrmSegCopy(QrmSegment* segment, QrmSegment other);
/// Copy constructor
QrmSegment QrmSegDuplicate(QrmSegment other);

#endif // QRMATRIXSEGMENT_H
