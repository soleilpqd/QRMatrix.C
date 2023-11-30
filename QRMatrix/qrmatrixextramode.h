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

#ifndef QRMATRIXEXTRAMODE_H
#define QRMATRIXEXTRAMODE_H

#include "constants.h"
#include "qrmatrixsegment.h"

typedef enum {
    /// none
    XModeNone,
    /// MicroQR
    XModeMicroQr,
    /// FNC1 First Position
    XModeFnc1First,
    /// FNC2 Second Position
    XModeFnc1Second
} QrmExtraMode;

typedef struct {
    /// Extra mode
    QrmExtraMode mode;
    /// A C-String represents Application Indicator for FNC1 Second Position.
    /// 2 valid forms:
    ///  - Single ASCII character [a-z][A-Z] eg. `a`.
    ///  - Two ditis number eg. `01`
    UnsignedByte* appIndicator;
    UnsignedByte appIndicatorLength;
} QrmExtraEncodingInfo;

/// Destructor
void QrmExtraDestroy(QrmExtraEncodingInfo* info);
/// Copy constructor
QrmExtraEncodingInfo QrmExtraDuplicate(QrmExtraEncodingInfo other);
/// Create none
QrmExtraEncodingInfo QrmExtraCreateNone(void);
/// Init for MicroQR or FCN1 First Position mode
QrmExtraEncodingInfo QrmExtraCreate(QrmExtraMode mode);
/// Init for FCN1 Second Positon mode
QrmExtraEncodingInfo QrmExtraCreateFnc1Second(const UnsignedByte* appId, UnsignedByte appIdLen);

typedef struct {
    /// Data segments
    QrmSegment* segments;
    /// Count of segments
    unsigned int count;
    /// Error correction info
    QrmErrorCorrectionLevel level;
    /// Optional. Limit minimum version
    /// (result version = max(minimum version, required version to fit data).
    UnsignedByte minVersion;
    /// Optional. Force to use given mask (0-7).
    /// Almost for test, you can ignore this.
    UnsignedByte maskId;
    /// Extra mode (MicroQR will be ignored. Not sure about FNC1.)
    QrmExtraEncodingInfo extraMode;
} QrmStructuredAppend;

/// Constructor
QrmStructuredAppend QrmStrAppCreate(QrmSegment* segs, unsigned int segCount, QrmErrorCorrectionLevel ecLevel);
/// Constructor
QrmStructuredAppend QrmStrAppCreateEmpty(void);
/// Copy constructor
QrmStructuredAppend QrmStrAppDuplicate(QrmStructuredAppend other);

void QrmStrAppFillSegs(QrmStructuredAppend* target, QrmSegment* segs, unsigned int segCount);
void QrmStrAppFillExtraMode(QrmStructuredAppend* target, QrmExtraEncodingInfo segs);

/// Destructor
void QrmStrAppDestroy(QrmStructuredAppend* data);

#endif // QRMATRIXEXTRAMODE_H
