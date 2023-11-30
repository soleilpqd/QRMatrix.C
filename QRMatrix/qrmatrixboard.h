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

#ifndef QRMATRIXBOARD_H
#define QRMATRIXBOARD_H

#include "constants.h"
#include "common.h"

/// Value of QR board cell
typedef enum {
    /// Cell is not filled yet
    CellNeutral         = 0x00,
    /// Cell is white (lower 4 bits)
    CellUnset           = 0x05,
    /// Cell is black (lower 4 bits)
    CellSet             = 0x0A,
    /// Cell is finder (higher 4 bits)
    CellFinder          = 0x10,
    /// Cell is separator (higher 4 bits)
    CellSeparator       = 0x20,
    /// Cell is alignment (higher 4 bits)
    CellAlignment       = 0x30,
    /// Cell is timing (higher 4 bits)
    CellTiming          = 0x40,
    /// Cell is dark module (higher 4 bits)
    CellDark            = 0x50,
    /// Cell is format (higher 4 bits)
    CellFormat          = 0x60,
    /// Cell is version (higher 4 bits) (version ≥ 7)
    CellVersion         = 0x70,
    /// Cell is ErrorCorrection
    CellErrorCorrection = 0x80,
    /// Cell is Remainder
    CellRemainder       = 0x90,

    /// cell & lowMask = lower 4 bits value
    CellLowMask         = 0x0F,
    /// cell & highMask = higher 4 bits value
    CellHighMask        = 0xF0,
    /// cell & funcMask > 0 => cell is function module
    CellFuncMask        = 0x70
} QrmBoardCell;

/// QR cells (modules) (not inclues quiet zone)
typedef struct {
    UnsignedByte dimension;
    UnsignedByte** buffer;
} QrmBoard;

void QrmBoardDestroy(QrmBoard* board);
QrmBoard QrmBoardDuplicate(QrmBoard other);
void QrmBoardCopy(QrmBoard* board, QrmBoard other);
/// Place holder. Internal purpose. Do not use.
QrmBoard QrmBoardCreateEmpty(void);
/// To create QR board, refer `QRMatrixEncoder`.
/// This constructor is for internal purpose.
QrmBoard QrmBoardCreate(UnsignedByte* data, UnsignedByte* errorCorrection, QrmSymbolInfo ecInfo, UnsignedByte maskId, bool isMicro);
void QrmBoardPrintDescription(QrmBoard board, bool isTypeVisible);

#endif // QRMATRIXBOARD_H
