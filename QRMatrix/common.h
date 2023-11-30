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

#ifndef COMMON_H
#define COMMON_H

#include "constants.h"

/// Maximum QR version
#define QR_MAX_VERSION          40
/// Different dimension amount between 2 sequential versions
#define QR_VERSION_OFFSET       4
/// QR minimum dimension
#define QR_MIN_DIMENSION        21

/// Maximum MicroQR version
#define MICROQR_MAX_VERSION     4
/// Different dimension amount between 2 sequential micro versions
#define MICROQR_VERSION_OFFSET  2
/// Micro QR minimum dimension
#define MICROQR_MIN_DIMENSION   11

/// Info to make QR Code
typedef struct {
    /// QR Version
    UnsignedByte version;
    /// EC level
    QrmErrorCorrectionLevel level;
    /// Number of data bytes
    Unsigned2Bytes codewords;
    /// Number of EC bytes per block
    Unsigned2Bytes ecCodewordsPerBlock;
    /// Number of group1 blocks
    Unsigned2Bytes group1Blocks;
    /// Number of data bytes for 1 group1 block
    Unsigned2Bytes group1BlockCodewords;
    /// Number of group2 blocks
    Unsigned2Bytes group2Blocks;
    /// Number of data bytes for 1 group2 block
    Unsigned2Bytes group2BlockCodewords;
} QrmSymbolInfo;

void QrmSymbolInfoInit(QrmSymbolInfo* result);
/// How many blocks in QR symbol
UnsignedByte QrmInfoGroupCount(QrmSymbolInfo info);
/// How many ErrorCorrection blocks in total
UnsignedByte QrmInfoECBlockTotalCount(QrmSymbolInfo info);
/// How many ErrorCorrection Codewords in total
unsigned int QrmInfoECCodewordsTotalCount(QrmSymbolInfo info);
/// @ref: https://www.thonky.com/qr-code-tutorial/error-correction-table.
QrmSymbolInfo QrmGetSymbolInfo(UnsignedByte version, QrmErrorCorrectionLevel level, bool isMicro);

/// Current environment
extern bool qrmIsEnvValid;
extern bool qrmIsLittleEndian;
void QrmCheckEnv(void);

/// Get QR dimension by its version (version in 1...40 ~ dimension 21...177, Micro version in 1...4 ~ dimension 11...17).
UnsignedByte QrmGetDimensionByVersion(UnsignedByte version, bool isMicro);

/// Number of bits for character counts indicator.
unsigned int QrmGetCharactersCountIndicatorLength(
    UnsignedByte version,
    QrmEncodingMode mode,
    bool isMicro
);

/// Number of bits of mode indicator for MicroQR.
unsigned int QrmGetMicroModeIndicatorLength(
    UnsignedByte version,
    QrmEncodingMode mode
);
/// Number of bits of terminator for MicroQR.
unsigned int QrmGetMicroTerminatorLength(
    UnsignedByte version
);
/// Map Encoding mode value to MicroQR
UnsignedByte QrmGetMicroQREncodingModeValue(
    QrmEncodingMode mode
);
/// Map ErrorCorrectionLevel value to MicroQR
UnsignedByte QrmGetMicroQRErrorCorrectionLevelValue(
    QrmErrorCorrectionLevel level,
    UnsignedByte version
);

/// Copy `count` bits of source BYTE (from bit `sourceStartIndex`) into `destination` BYTE stating from bit at `destStartIndex`.
/// Bits index is from left to right.
/// Bit index must be 0...7 (because this copies 1 byte only).
/// `count` must be 1...8 (bits - size of `source`), also `count` must be < 8 - destStartIndex && 8 - sourceStartIndex.
bool QrmCopySingleByte(
    /// Source Byte.
    UnsignedByte sourceByte,
    /// Source starting bit.
    unsigned int sourceStartIndex,
    /// Destination memory allocation.
    UnsignedByte* destination,
    /// Destination starting bit.
    unsigned int destStartIndex,
    /// Number of bits to be copied.
    unsigned int count
);

/// Copy `count` bits of source (from bit 0th) into `destination` stating from bit at `startIndex`.
bool QrmCopyBits(
    /// Source.
    UnsignedByte* source,
    /// Source length in byte.
    unsigned int sourceLength,
    /// Source starting bit index
    unsigned int sourceStartIndex,
    /// Source order (little/big endian).
    bool isSourceOrderReversed,
    /// Destination (bytes array - not care about order here).
    UnsignedByte* destination,
    /// Destintation starting bit index.
    unsigned int destStartIndex,
    /// Number of bits to be copied.
    unsigned int count
);

/// Return array of 6 items contains QR aligment locations for version 2...40
const UnsignedByte* QrmGetAlignmentLocations(UnsignedByte version);

#endif // COMMON_H
