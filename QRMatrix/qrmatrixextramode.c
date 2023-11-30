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

#include "qrmatrixextramode.h"
#include <stdlib.h>

void QrmExtraDestroy(QrmExtraEncodingInfo* info) {
    if (info->appIndicatorLength > 0 && info->appIndicator != NULL) {
        DEALLOC(info->appIndicator);
        info->appIndicatorLength = 0;
    }
}

QrmExtraEncodingInfo QrmExtraDuplicate(QrmExtraEncodingInfo other) {
    QrmExtraEncodingInfo result;
    result.mode = other.mode;
    if (other.appIndicator != NULL) {
        result.appIndicatorLength = other.appIndicatorLength;
        ALLOC_(UnsignedByte, result.appIndicator, result.appIndicatorLength);
        for (unsigned int index = 0; index < other.appIndicatorLength; index += 1) {
            result.appIndicator[index] = other.appIndicator[index];
        }
    } else {
        result.appIndicator = NULL;
        result.appIndicatorLength = 0;
    }
    return result;
}

QrmExtraEncodingInfo QrmExtraCreateNone() {
    QrmExtraEncodingInfo result;
    result.mode = XModeNone;
    result.appIndicator = NULL;
    result.appIndicatorLength = 0;
    return result;
}

QrmExtraEncodingInfo QrmExtraCreate(QrmExtraMode mode) {
    QrmExtraEncodingInfo result;
    result.mode = mode;
    result.appIndicator = NULL;
    result.appIndicatorLength = 0;
    return result;
}

QrmExtraEncodingInfo QrmExtraCreateFnc1Second(const UnsignedByte* appId, UnsignedByte appIdLen) {
    QrmExtraEncodingInfo result;
    result.mode = XModeFnc1Second;
    result.appIndicatorLength = appIdLen;
    ALLOC_(UnsignedByte, result.appIndicator, appIdLen);
    for (unsigned int index = 0; index < appIdLen; index += 1) {
        result.appIndicator[index] = appId[index];
    }
    return result;
}

// STRUCTURED APPEND --------------------------------------------------------------------------------------------------------------------------------

void QrmStrAppDestroy(QrmStructuredAppend* data) {
    QrmExtraDestroy(&data->extraMode);
    if (data->segments != NULL && data->count > 0) {
        for (unsigned int index = 0; index < data->count; index += 1) {
            QrmSegDestroy(&data->segments[index]);
        }
        DEALLOC(data->segments);
    }
}

QrmStructuredAppend QrmStrAppCreate(QrmSegment* segs, unsigned int segCount, QrmErrorCorrectionLevel ecLevel) {
    QrmStructuredAppend result;
    result.count = segCount;
    result.level = ecLevel;
    result.minVersion = 0;
    result.maskId = 0xFF;
    result.extraMode = QrmExtraCreateNone();
    if (segCount > 0 && segs != NULL) {
        ALLOC_(QrmSegment, result.segments, segCount);
        for (unsigned int index = 0; index < segCount; index += 1) {
            QrmSegCopy(&result.segments[index], segs[index]);
        }
    } else {
        result.segments = NULL;
        result.count = 0;
    }
    return result;
}

QrmStructuredAppend QrmStrAppCreateEmpty() {
    QrmStructuredAppend result;
    result.count = 0;
    result.segments = NULL;
    result.level = ELevelLow;
    result.minVersion = 0;
    result.maskId = 0xFF;
    result.extraMode = QrmExtraCreateNone();
    return result;
}

QrmStructuredAppend QrmStrAppDuplicate(QrmStructuredAppend other) {
    QrmStructuredAppend result;
    result.count = other.count;
    result.level = other.level;
    result.minVersion = other.minVersion;
    result.maskId = other.maskId;
    result.extraMode = QrmExtraDuplicate(other.extraMode);
    if (other.segments != NULL) {
        ALLOC_(QrmSegment, result.segments, other.count);
        for (unsigned int index = 0; index < other.count; index += 1) {
            QrmSegCopy(&result.segments[index], other.segments[index]);
        }
    } else {
        result.count = 0;
        result.segments = NULL;
    }
    return result;
}

void QrmStrAppFillSegs(QrmStructuredAppend* target, QrmSegment* segs, unsigned int segCount) {
    if (target->segments != NULL && target->count > 0) {
        for (unsigned int index = 0; index < target->count; index += 1) {
            QrmSegDestroy(&target->segments[index]);
        }
        DEALLOC(target->segments);
        target->segments = NULL;
        target->count = 0;
    }
    if (segs != NULL && segCount > 0) {
        target->count = segCount;
        ALLOC_(QrmSegment, target->segments, segCount);
        for (unsigned int index = 0; index < segCount; index += 1) {
            QrmSegCopy(&target->segments[index], segs[index]);
        }
    }
}

void QrmStrAppFillExtraMode(QrmStructuredAppend* target, QrmExtraEncodingInfo segs) {
    QrmExtraDestroy(&target->extraMode);
    target->extraMode = QrmExtraDuplicate(segs);
}
