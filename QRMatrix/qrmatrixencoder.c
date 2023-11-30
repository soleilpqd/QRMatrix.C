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

#include "qrmatrixencoder.h"
#include "common.h"
#include <stdlib.h>
#include "Encoder/numericencoder.h"
#include "Encoder/alphanumericencoder.h"
#include "Encoder/kanjiencoder.h"

#include "Polynomial/polynomial.h"

bool qrmIsEnvInited = false;

void QRMatrixInit() {
    QrmCheckEnv();
    QrmPolynomialInitialize();
    qrmIsEnvInited = true;
}

// PREPARE DATA --------------------------------------------------------------------------------------------------------------------------------------

/// Calculate encoded data bits count,
/// include Mode Indicator and ECI header bits,
/// exclude Characters Count bits
unsigned int QRMatrixEncoder_calculateEncodedDataBitsCount(
    QrmSegment* segments,
    unsigned int count
) {
    unsigned int totalDataBitsCount = 0;
    for (unsigned int index = 0; index < count; index += 1) {
        QrmSegment segment = segments[index];
        if (segment.length == 0) {
            continue;
        }
        // Mode indicator
        totalDataBitsCount += 4;
        // ECI
        if (segment.eci != DEFAULT_ECI_ASSIGMENT) {
            totalDataBitsCount += 4; // ECI Header
            if (segment.eci <= 128) {
                totalDataBitsCount += 8; // 1 byte ECI Indicator
            } else if (segment.eci <= 16383) {
                totalDataBitsCount += 16; // 2 bytes ECI Indicator
            } else {
                totalDataBitsCount += 24; // 3 bytes ECI Indicator
            }
        }
        // Data
        switch (segment.mode) {
        case EModeNumeric: {
            // 3 characters encoded in 10 bits (each character is 1 byte)
            unsigned int numberOfGroups = (segment.length / 3);
            totalDataBitsCount += numberOfGroups * NUM_TRIPLE_DIGITS_BITS_LEN;
            // Remaining chars
            UnsignedByte remainChars = segment.length % 3;
            switch (remainChars) {
            case 1:
                totalDataBitsCount += NUM_SINGLE_DIGIT_BITS_LEN;
                break;
            case 2:
                totalDataBitsCount += NUM_DOUBLE_DIGITS_BITS_LEN;
            default:
                break;
            }
        }
        break;
        case EModeAlphaNumeric: {
            // 2 characters encoded in 11 bits (each character is 1 byte)
            // Remaining character encoded in 6 bits.
            unsigned int numberOfGroups = segment.length / 2;
            unsigned int remaining = segment.length % 2;
            totalDataBitsCount += ALPHA_NUM_PAIR_CHARS_BITS_LEN * numberOfGroups +
                                  ALPHA_NUM_SINGLE_CHAR_BITS_LEN * remaining;
        }
        break;
        case EModeKanji:
            // 2 bytes per Kanji character.
            // Each character is encoded in 13 bits.
            totalDataBitsCount += (segment.length / 2) * 13;
            break;
        case EModeByte:
            totalDataBitsCount += segment.length * 8;
            break;
        }
    }
    return totalDataBitsCount;
}

/// Find QR Version & its properties
QrmSymbolInfo QRMatrixEncoder_findVersion(
    QrmSegment* segments,
    unsigned int count,
    QrmErrorCorrectionLevel level,
    UnsignedByte minVersion,
    QrmExtraEncodingInfo extraMode,
    bool isStructuredAppend
) {
    // This is total estimated bits of data, excluding bits for Characters Count,
    // because each version requires difference number of Characters Count bits.
    unsigned int totalDataBitsCount = QRMatrixEncoder_calculateEncodedDataBitsCount(segments, count);
    if (isStructuredAppend) {
        totalDataBitsCount += 20; // 4 bits header, 4 bits position, 4 bits total number, 1 byte parity
    }
    if (extraMode.mode == XModeFnc1First) {
        totalDataBitsCount += 4; // 4 bits FNC1 indicator
    } else  if (extraMode.mode == XModeFnc1Second) {
        totalDataBitsCount += 12; // 4 bits FNC1 indicator, 8 bits Application Indicator
    }
    // Go for each version, get total bits and check
    bool isMicro = (extraMode.mode == XModeMicroQr && !isStructuredAppend);
    UnsignedByte maxVer = isMicro ? MICROQR_MAX_VERSION : QR_MAX_VERSION;
    for (UnsignedByte version = (minVersion > 0 && minVersion <= maxVer) ? minVersion : 1; version <= maxVer; version += 1) {
        if (version > 1 && isMicro && level == ELevelHigh) {
            LOG("ERROR: Error Correction Level High not available in MicroQR.");
            break;
        }
        QrmSymbolInfo info = QrmGetSymbolInfo(version, level, isMicro);
        unsigned int capacity = info.codewords * 8;
        if (capacity == 0) {
            break;
        }
        if (totalDataBitsCount >= capacity) {
            // Not need to count total bits, just go to next version
            continue;
        }
        // Calculate total number of bits for Characters Count for this version
        unsigned int totalBits = totalDataBitsCount;
        bool hasAlpha = false;
        bool hasKanji = false;
        bool hasByte = false;
        for (unsigned int index = 0; index < count; index += 1) {
            QrmSegment segment = segments[index];
            if (segment.length == 0) {
                continue;
            }
            switch (segment.mode) {
            case EModeAlphaNumeric:
                hasAlpha = true;
                break;
            case EModeByte:
                hasByte = true;
                break;
            case EModeKanji:
                hasKanji = true;
                break;
            default:
                break;
            }
            totalBits += QrmGetCharactersCountIndicatorLength(version, segment.mode, isMicro);
        }
        // Check if this QR version bits capacity is enough for required total bits
        if (totalBits <= capacity) {
            UnsignedByte finalVer = version;
            if (isMicro) {
                if (finalVer < 2 && hasAlpha) {
                    // AlphaNumeric Mode is not available with M1
                    finalVer = 2;
                }
                if (finalVer < 3 && (hasByte || hasKanji)) {
                    // Byte/Kanji Mode is not available with <= M2
                    finalVer = 3;
                }
            }
            if (finalVer == version) {
                return info;
            }
            return QrmGetSymbolInfo(finalVer, level, isMicro);
        }
    }
    QrmSymbolInfo result; QrmSymbolInfoInit(&result);
    return result;
}

// ENCODE DATA---------------------------------------------------------------------------------------------------------------------------------------

/// Encode ECI Indicator
UnsignedByte* QRMatrixEncoder_encodeEciIndicator(Unsigned4Bytes indicator, UnsignedByte* resultLength) {
    UnsignedByte* indicatorPtr = (UnsignedByte*)&indicator;
    UnsignedByte* result = NULL;
    if (indicator <= 127) {
        *resultLength = 1;
        ALLOC_(UnsignedByte, result, 1);
        if (qrmIsLittleEndian) {
            result[0] = indicatorPtr[0] & 0x7F;
        } else {
            result[0] = indicatorPtr[3] & 0x7F;
        }
    } else if (indicator <= 16383) {
        *resultLength = 2;
        ALLOC_(UnsignedByte, result, 2);
        if (qrmIsLittleEndian) {
            result[0] = (indicatorPtr[1] & 0x3F) | 0x80;
            result[1] = indicatorPtr[0];
        } else {
            result[0] = (indicatorPtr[2] & 0x3F) | 0x80;
            result[1] = indicatorPtr[3];
        }
    } else if (indicator <= 999999) {
        *resultLength = 3;
        ALLOC_(UnsignedByte, result, 3);
        if (qrmIsLittleEndian) {
            result[0] = (indicatorPtr[2] & 0x1F) | 0xC0;
            result[1] = indicatorPtr[1];
            result[2] = indicatorPtr[0];
        } else {
            result[0] = (indicatorPtr[1] & 0x1F) | 0xC0;
            result[1] = indicatorPtr[2];
            result[2] = indicatorPtr[3];
        }
    } else {
        LOG("ERROR: Invalid ECI Indicator");
        *resultLength = 0;
    }
    return result;
}

/// Encode segments into buffer
void QRMatrixEncoder_encodeSegment(
    UnsignedByte* buffer,
    QrmSegment segment,
    unsigned int segmentIndex,
    QrmErrorCorrectionLevel level,
    QrmSymbolInfo ecInfo,
    unsigned int* bitIndex,
    QrmExtraEncodingInfo extraMode
) {
    if (segment.length == 0) {
        return;
    }
    bool isMicro = extraMode.mode == XModeMicroQr;
    unsigned int uintSize = sizeof(unsigned int);
    // ECI Header if enable
    if (!isMicro && segment.eci != DEFAULT_ECI_ASSIGMENT) {
        UnsignedByte eciLen = 0;
        UnsignedByte* eciHeader = QRMatrixEncoder_encodeEciIndicator(segment.eci, &eciLen);
        if (eciLen > 0) {
            // 4 bits of ECI mode indicator
            UnsignedByte eciModeHeader = 0b0111;
            QrmCopyBits(&eciModeHeader, 1, 4, false, buffer, *bitIndex, 4);
            *bitIndex += 4;
            // ECI indicator
            QrmCopyBits(
                eciHeader,
                eciLen, // size of eciUtf8Indicator
                0, // bit of eciUtf8Indicator to start copy
                false, // `eciHeader` is always normal order (big endian)
                buffer,
                *bitIndex,
                8 * eciLen // number of bits to be copied
                );
            *bitIndex += 8 * eciLen;
            DEALLOC(eciHeader);
        }
    }
    if (segmentIndex == 0) {
        if (extraMode.mode == XModeFnc1First) {
            UnsignedByte fnc1Header = 0b0101;
            QrmCopyBits(&fnc1Header, 1, 4, false, buffer, *bitIndex, 4);
            *bitIndex += 4;
        } else if (extraMode.mode == XModeFnc1Second) {
            UnsignedByte fnc1Header = 0b1001;
            QrmCopyBits(&fnc1Header, 1, 4, false, buffer, *bitIndex, 4);
            *bitIndex += 4;
            fnc1Header = 0;
            switch (extraMode.appIndicatorLength) {
            case 1:
                fnc1Header = extraMode.appIndicator[0] + 100;
                break;
            case 2: {
                ALLOC(UnsignedByte, numStr, 3);
                numStr[0] = extraMode.appIndicator[0];
                numStr[1] = extraMode.appIndicator[1];
                fnc1Header = atoi((const char*)numStr);
                DEALLOC(numStr);
            }
                break;
            default:
                break;
            }
            QrmCopyBits(&fnc1Header, 1, 0, false, buffer, *bitIndex, 8);
            *bitIndex += 8;
        }
    }
    // Bits of segment mode indicator
    UnsignedByte numberOfModeBits = isMicro ? QrmGetMicroModeIndicatorLength(ecInfo.version, segment.mode) : 4;
    if (numberOfModeBits > 0) {
        UnsignedByte mode = isMicro ? QrmGetMicroQREncodingModeValue(segment.mode) : segment.mode;
        QrmCopyBits(&mode, 1, 8 - numberOfModeBits, false, buffer, *bitIndex, numberOfModeBits);
        *bitIndex += numberOfModeBits;
    }
    // Character counts bits
    unsigned int charCountIndicatorLen = QrmGetCharactersCountIndicatorLength(ecInfo.version, segment.mode, isMicro);
    unsigned int charCount = 0;
    switch (segment.mode) {
    case EModeNumeric:
    case EModeAlphaNumeric:
    case EModeByte:
        charCount = segment.length;
        break;
    case EModeKanji:
        charCount = segment.length / 2;
        break;
    }
    QrmCopyBits(
        (UnsignedByte*)&charCount,
        uintSize,
        uintSize * 8 - charCountIndicatorLen,
        qrmIsLittleEndian,
        buffer,
        *bitIndex,
        charCountIndicatorLen
    );
    *bitIndex += charCountIndicatorLen;

    LOG("SEGMENT:\n%s version: %d\nMode: %d\nEC Level: %d\nChar count: %d\nPayload length: %d\nEC Length: %d",
        isMicro ? "MicroQR" : "QR", ecInfo.version,
        segment.mode,
        level,
        charCount,
        ecInfo.codewords,
        QrmInfoECCodewordsTotalCount(ecInfo)
    );

    switch (segment.mode) {
    case EModeNumeric:
        *bitIndex += QrmNumericEncode(segment.data, segment.length, buffer, *bitIndex);
        break;
    case EModeAlphaNumeric:
        *bitIndex += QrmAlphaNumericEncode(segment.data, segment.length, buffer, *bitIndex);
        break;
    case EModeByte: {
        UnsignedByte* bytes = segment.data;
        for (unsigned int idx = 0; idx < segment.length; idx += 1) {
            QrmCopyBits(bytes, 1, 0, false, buffer, *bitIndex, 8);
            *bitIndex += 8;
            bytes += 1;
        }
    }
        break;
    case EModeKanji:
        *bitIndex += QrmKanjiEncode(segment.data, segment.length, buffer, *bitIndex);
        break;
    }
}

// ERROR CORRECTION ---------------------------------------------------------------------------------------------------------------------------------

/// Generate Error correction bytes
/// @return Binary data (should be deleted on unused)
UnsignedByte* QRMatrixEncoder_generateErrorCorrection(
    /// Bytes from previous (encode data) step
    UnsignedByte* encodedData,
    /// EC Info from previous step
    QrmSymbolInfo ecInfo,
    /// Group number: 0, 1
    UnsignedByte group,
    /// Block number: 0, ...
    UnsignedByte block
) {
    UnsignedByte maxGroup = QrmInfoGroupCount(ecInfo);
    if (group >= maxGroup) {
        LOG("ERROR: Invalid group number");
        return NULL;
    }
    UnsignedByte maxBlock = 0;
    unsigned int offset = 0;
    unsigned int blockSize = 0;
    switch (group) {
    case 0:
        maxBlock = ecInfo.group1Blocks;
        blockSize = ecInfo.group1BlockCodewords;
        offset = block * ecInfo.group1BlockCodewords;
        break;
    case 1:
        maxBlock = ecInfo.group2Blocks;
        blockSize = ecInfo.group2BlockCodewords;
        offset = ecInfo.group1Blocks * ecInfo.group1BlockCodewords + ecInfo.group2BlockCodewords * block;
        break;
    default:
        break;
    }
    if (block >= maxBlock) {
        LOG("ERROR: Invalid block number");
        return NULL;
    }

    QrmPolynomial mesg = QrmPolynomialCreate(blockSize);
    for (unsigned int index = 0; index < blockSize; index += 1) {
        mesg.terms[index] = encodedData[offset + index];
    }
    QrmPolynomial ecc = QrmGetErrorCorrections(ecInfo.ecCodewordsPerBlock, mesg);
    if (ecc.length == 0) {
        QrmPolynomialDestroy(&mesg);
        QrmPolynomialDestroy(&ecc);
        return NULL;
    }
    ALLOC(UnsignedByte, result, ecc.length);
    for (unsigned int index = 0; index < ecc.length; index += 1) {
        result[index] = ecc.terms[index];
    }

    LOG("DATA to ECC: group=%d; block=%d", group, block);
    LOG_BIN(mesg.terms, mesg.length);
    LOG("ECC: group=%d; block=%d", group, block);
    LOG_BIN(result, ecc.length);

    QrmPolynomialDestroy(&mesg);
    QrmPolynomialDestroy(&ecc);
    return result;
}


/// Generate Error correction bytes
/// @return Binary data (should be deleted on unused). 2D array contains EC bytes for all blocks.
UnsignedByte** QRMatrixEncoder_generateErrorCorrections(
    /// Bytes from previous (encode data) step
    UnsignedByte* encodedData,
    /// EC Info from previous step
    QrmSymbolInfo ecInfo
) {
    UnsignedByte maxGroup = QrmInfoGroupCount(ecInfo);
    ALLOC(UnsignedByte*, result, QrmInfoECBlockTotalCount(ecInfo));
    unsigned int blockIndex = 0;
    for (UnsignedByte group = 0; group < maxGroup; group += 1) {
        UnsignedByte maxBlock = 0;
        switch (group) {
        case 0:
            maxBlock = ecInfo.group1Blocks;
            break;
        case 1:
            maxBlock = ecInfo.group2Blocks;
            break;
        default:
            break;
        }
        for (UnsignedByte block = 0; block < maxBlock; block += 1) {
            result[blockIndex] = QRMatrixEncoder_generateErrorCorrection(encodedData, ecInfo, group, block);
            blockIndex += 1;
        }
    }
    return result;
}

// INTERLEAVE ---------------------------------------------------------------------------------------------------------------------------------------

/// Interleave data codeworks
/// Throw error if QR has only 1 block in total (check ecInfo before call this function)
/// @return Binary data (should be deleted on unused)
UnsignedByte* QRMatrixEncoder_interleave(
    /// Encoded data
    UnsignedByte* encodedData,
    /// EC info
    QrmSymbolInfo ecInfo
) {
    unsigned int blockCount = QrmInfoECBlockTotalCount(ecInfo);
    if (blockCount == 1) {
        LOG("ERROR: Interleave not required");
        return NULL;
    }
    ALLOC(UnsignedByte, result, ecInfo.codewords);
    UnsignedByte* blockPtr[blockCount];
    UnsignedByte* blockEndPtr[blockCount];
    UnsignedByte groupCount = QrmInfoGroupCount(ecInfo);
    unsigned int blockIndex = 0;

    for (UnsignedByte group = 0; group < groupCount; group += 1) {
        UnsignedByte blockCount = ecInfo.group1Blocks;
        unsigned int blockSize = ecInfo.group1BlockCodewords;
        if (group == 1) {
            blockCount = ecInfo.group2Blocks;
            blockSize = ecInfo.group2BlockCodewords;
        }
        for (UnsignedByte block = 0; block < blockCount; block += 1) {
            unsigned int offset = 0;
            if (group == 1) {
                offset += ecInfo.group1Blocks * ecInfo.group1BlockCodewords;
            }
            offset += block * blockSize;
            blockPtr[blockIndex] = &encodedData[offset];
            blockEndPtr[blockIndex] = &encodedData[offset + blockSize - 1];
            blockIndex += 1;
        }
    }

    blockIndex = 0;
    unsigned int resIndex = 0;
    bool found = true;
    while (found) {
        result[resIndex] = *blockPtr[blockIndex];
        resIndex += 1;
        blockPtr[blockIndex] += 1;

        unsigned int loopCount = 0;
        found = false;
        while (loopCount < blockCount) {
            blockIndex += 1;
            if (blockIndex >= blockCount) {
                blockIndex = 0;
            }
            if (blockPtr[blockIndex] <= blockEndPtr[blockIndex]) {
                found = true;
                break;
            }
            loopCount += 1;
        }
    }
    return result;
}

/// Interleave error correction codeworks
/// Throw error if QR has only 1 block in total (check ecInfo before call this function)
/// @return Binary data (should be deleted on unused)
UnsignedByte* QRMatrixEncoder_interleaveEC(
    /// Error correction data
    UnsignedByte** data,
    /// EC info
    QrmSymbolInfo ecInfo
) {
    unsigned int blockCount = QrmInfoECBlockTotalCount(ecInfo);
    if (blockCount == 1) {
        LOG("ERROR: Interleave not required");
        return NULL;
    }
    ALLOC(UnsignedByte, result, QrmInfoECCodewordsTotalCount(ecInfo));
    unsigned int resIndex = 0;
    for (unsigned int index = 0; index < ecInfo.ecCodewordsPerBlock; index += 1) {
        for (unsigned int jndex = 0; jndex < blockCount; jndex += 1) {
            result[resIndex] = data[jndex][index];
            resIndex += 1;
        }
    }
    return result;
}

// FINALIZE -----------------------------------------------------------------------------------------------------------------------------------------

void QRMatrixEncoder_clean(UnsignedByte* buffer, UnsignedByte** ecBuffer, QrmSymbolInfo ecInfo) {
    for (unsigned int index = 0; index < (ecInfo.group1Blocks + ecInfo.group2Blocks); index += 1) {
        DEALLOC(ecBuffer[index]);
    }
    DEALLOC(ecBuffer);
    DEALLOC(buffer);
}

QrmBoard QRMatrixEncoder_finishEncodingData(
    UnsignedByte* buffer,
    QrmSymbolInfo ecInfo,
    unsigned int* bitIndex,
    UnsignedByte maskId,
    QrmExtraEncodingInfo extraMode
) {
    bool isMicro = (extraMode.mode == XModeMicroQr);
    bool isMicroV13 = isMicro && ((ecInfo.version == 1) || ecInfo.version == 3);
    unsigned int bufferLen = ecInfo.codewords;
    unsigned int bufferBitsLen = bufferLen * 8;
    if (isMicroV13) {
        bufferBitsLen -= 4;
    }
    /// Terminator
    unsigned int terminatorLength = isMicro ? QrmGetMicroTerminatorLength(ecInfo.version) : 4;
    for (unsigned int index = 0; *bitIndex < bufferBitsLen && index < terminatorLength; index += 1) {
        *bitIndex += 1;
    }

    /// Make data multiple by 8
    if (isMicroV13) {
        while ((*bitIndex < bufferBitsLen - 4) && (*bitIndex % 8 != 0)) {
            *bitIndex += 1;
        }
    } else {
        while (*bitIndex < bufferBitsLen && *bitIndex % 8 != 0) {
            *bitIndex += 1;
        }
    }

    /// Fill up
    UnsignedByte byteFilling1 = 0b11101100; // 0xEC
    UnsignedByte byteFilling2 = 0b00010001; // 0x11
    UnsignedByte curByteFilling = byteFilling1;
    while (*bitIndex < bufferBitsLen - (isMicroV13 ? 4 : 0)) {
        QrmCopyBits(
            (UnsignedByte*)&curByteFilling,
            1, // size of curByteFilling
            0, // Bit index of curByteFilling
            false, // copy 1 byte source, so we don't need to care byte order here
            buffer, // destination
            *bitIndex, // destination bit index
            8
        );
        *bitIndex += 8;
        if (curByteFilling == byteFilling1) {
            curByteFilling = byteFilling2;
        } else {
            curByteFilling = byteFilling1;
        }
    }
    *bitIndex = bufferBitsLen;

    // Error corrections
    UnsignedByte** ecBuffer = QRMatrixEncoder_generateErrorCorrections(buffer, ecInfo);

    LOG("Input Data:")
    LOG_BIN(buffer, ecInfo.codewords);

    // Interleave ...
    if (QrmInfoECBlockTotalCount(ecInfo) > 1) {
        UnsignedByte *interleave = QRMatrixEncoder_interleave(buffer, ecInfo);
        UnsignedByte *ecInterleave = QRMatrixEncoder_interleaveEC(ecBuffer, ecInfo);

        LOG("Interleave data:");
        LOG_BIN(interleave, ecInfo.codewords);
        LOG("Interleave EC:");
        LOG_BIN(ecInterleave, QrmInfoECCodewordsTotalCount(ecInfo));

        QrmBoard board = QrmBoardCreate(interleave, ecInterleave, ecInfo, maskId, isMicro);

        DEALLOC(interleave);
        DEALLOC(ecInterleave);
        QRMatrixEncoder_clean(buffer, ecBuffer, ecInfo);
        return board;
    }
    // ... or not
    LOG("EC:");
    LOG_BIN(ecBuffer[0], ecInfo.ecCodewordsPerBlock);

    QrmBoard board = QrmBoardCreate(buffer, ecBuffer[0], ecInfo, maskId, isMicro);
    QRMatrixEncoder_clean(buffer, ecBuffer, ecInfo);
    return board;
}

QrmBoard QRMatrixEncoder_encodeSingle(
    QrmSegment* segments,
    unsigned int count,
    QrmErrorCorrectionLevel level,
    QrmExtraEncodingInfo extraMode,
    UnsignedByte minVersion,
    UnsignedByte maskId,
    UnsignedByte sequenceIndex,
    UnsignedByte sequenceTotal,
    UnsignedByte parity
) {
    unsigned int segCount = 0;
    for (unsigned int index = 0; index < count; index += 1) {
        if (segments[index].length > 0) {
            segCount += 1;
        }
    }
    if (segCount == 0) {
        LOG("ERROR: No input.");
        return QrmBoardCreateEmpty();
    }
    if (extraMode.mode == XModeFnc1Second) {
        bool isValid = false;
        switch (extraMode.appIndicatorLength) {
        case 1: {
            UnsignedByte value = extraMode.appIndicator[0];
            isValid = (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
        }
            break;
        case 2:
        {
            UnsignedByte value1 = extraMode.appIndicator[0];
            UnsignedByte value2 = extraMode.appIndicator[1];
            isValid = (value1 >= '0' && value1 <= '9') && (value2 >= '0' && value2 <= '9');
        }
            break;
        default:
            break;
        }
        if (!isValid) {
            LOG("ERROR: Invalid Application Indicator for FNC1 Second Position mode");
            return QrmBoardCreateEmpty();
        }
    }
    bool isStructuredAppend = sequenceTotal > 0 && sequenceTotal <= 16;
    QrmSymbolInfo ecInfo = QRMatrixEncoder_findVersion(segments, count, level, minVersion, extraMode, isStructuredAppend);
    if (ecInfo.version == 0) {
        LOG("ERROR: Unable to find suitable QR version.");
        return QrmBoardCreateEmpty();
    }
    if (isStructuredAppend && extraMode.mode == XModeMicroQr) {
        extraMode = QrmExtraCreateNone();
    }
    // Allocate
    ALLOC(UnsignedByte, buffer, ecInfo.codewords);
    unsigned int bitIndex = 0;
    // Structured append
    if (isStructuredAppend) {
        UnsignedByte bufByte = 0b0011;
        QrmCopyBits(&bufByte, 1, 4, false, buffer, bitIndex, 4);
        bitIndex += 4;
        QrmCopyBits(&sequenceIndex, 1, 4, false, buffer, bitIndex, 4);
        bitIndex += 4;
        bufByte = sequenceTotal - 1;
        QrmCopyBits(&bufByte, 1, 4, false, buffer, bitIndex, 4);
        bitIndex += 4;
        QrmCopyBits(&parity, 1, 0, false, buffer, bitIndex, 8);
        bitIndex += 8;
    }
    // Encode data
    for (unsigned int index = 0; index < count; index += 1) {
        QRMatrixEncoder_encodeSegment(buffer, segments[index], index, level, ecInfo, &bitIndex, extraMode);
    }
    // Finish
    return QRMatrixEncoder_finishEncodingData(buffer, ecInfo, &bitIndex, maskId, extraMode);
}

// PUBLIC METHODS -----------------------------------------------------------------------------------------------------------------------------------

QrmBoard QrmEncoderEncode(
    QrmSegment* segments,
    unsigned int count,
    QrmErrorCorrectionLevel level,
    QrmExtraEncodingInfo extraMode,
    UnsignedByte minVersion,
    UnsignedByte maskId
) {
    if (!qrmIsEnvInited || !qrmIsEnvValid) {
        LOG("ERROR: Environment is not initialized or invalid");
        return QrmBoardCreateEmpty();
    }
    return QRMatrixEncoder_encodeSingle(
        segments, count, level, extraMode, minVersion, maskId, 0, 0, 0
    );
}

UnsignedByte QrmEncoderGetVersion(
    QrmSegment* segments,
    unsigned int count,
    QrmErrorCorrectionLevel level,
    QrmExtraEncodingInfo extraMode,
    bool isStructuredAppend
) {
    unsigned int segCount = 0;
    for (unsigned int index = 0; index < count; index += 1) {
        if (segments[index].length > 0) {
            segCount += 1;
        }
    }
    if (segCount == 0) {
        return 0;
    }
    QrmSymbolInfo ecInfo = QRMatrixEncoder_findVersion(segments, count, level, 0, extraMode, isStructuredAppend);
    return ecInfo.version;
}

QrmBoard* QrmEncoderMakeStructuredAppend(
    /// Array of data parts to be encoded
    QrmStructuredAppend* parts,
    /// Number of parts
    unsigned int count
) {
    if (!qrmIsEnvInited || !qrmIsEnvValid) {
        LOG("ERROR: Environment is not initialized or invalid");
        return NULL;
    }
    if (count > 16) {
        LOG("ERROR: Structured Append only accepts 16 parts maximum");
        return NULL;
    }
    if (count == 0) {
        LOG("ERROR: No input.");
        return NULL;
    }
//    if (count == 1) {
        // Should change to encode single QR symbol or throw error?
        // But no rule prevents to make a Structured Append QR symbol single part
//    }
    UnsignedByte parity = 0;
    for (UnsignedByte index = 0; index < count; index += 1) {
        QrmStructuredAppend part = parts[index];
        for (unsigned int segIndex = 0; segIndex < part.count; segIndex += 1) {
            QrmSegment segment = part.segments[segIndex];
            for (unsigned int idx = 0; idx < segment.length; idx += 1) {
                if (index == 0 && segIndex == 0 && idx == 0) {
                    parity = segment.data[idx];
                } else {
                    parity = parity ^ segment.data[idx];
                }
            }
        }
    }
    ALLOC(QrmBoard, result, count);
    for (UnsignedByte index = 0; index < count; index += 1) {
        QrmStructuredAppend part = parts[index];
        QrmBoard board = QRMatrixEncoder_encodeSingle(
            part.segments, part.count,
            part.level, part.extraMode,
            part.minVersion, part.maskId,
            index, count, parity
        );
        QrmBoardCopy(&result[index], board);
        QrmBoardDestroy(&board);
    }
    return result;
}
