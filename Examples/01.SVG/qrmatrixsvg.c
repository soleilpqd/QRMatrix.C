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

#include "qrmatrixsvg.h"
#include <stdio.h>

int QrmSvgDraw(QrmBoard board, const char* path, unsigned int scale, bool isMicro) {
    if (scale < 1) {
        printf("ERROR: Scale > 0");
        return 1;
    }

    unsigned int quietZone = isMicro ? 2 : 4;
    FILE* file = fopen(path , "w");
    if (file == NULL) {
        return 1;
    }

    // <svg>
    unsigned int dimension = board.dimension * scale + 2 * quietZone * scale;
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%d\" height=\"%d\">\n", dimension, dimension);
    fprintf(file, "    <rect fill=\"white\" stroke=\"none\" width=\"%d\" height=\"%d\" x=\"0\" y=\"0\"/>\n", dimension, dimension);
    fprintf(file, "    <g fill=\"black\" stroke=\"none\">\n");
    // Content
    UnsignedByte** data = board.buffer;
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;

                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // </svg>
    fprintf(file, "</svg>\n");
    return fclose(file);
}

int QrmSvgDrawDetail(
    QrmBoard board,
    const char* path,
    unsigned int scale,
    bool isMicro,
    const char* backgroundColor,
    const char* dataColor,
    const char* finderColor,
    const char* timingColor,
    const char* darkColor,
    const char* aligmentColor,
    const char* versionColor,
    const char* formatColor,
    const char* ecColor,
    const char* remainderColor
) {
    if (scale < 1) {
        printf("ERROR: Scale > 0");
        return 1;
    }

    unsigned int quietZone = isMicro ? 2 : 4;
    FILE* file = fopen(path , "w");
    if (file == NULL) {
        return 1;
    }
    // <svg>
    unsigned int dimension = board.dimension * scale + 2 * quietZone * scale;
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%d\" height=\"%d\">\n", dimension, dimension);
    fprintf(file, "    <rect fill=\"%s\" stroke=\"none\" width=\"%d\" height=\"%d\" x=\"0\" y=\"0\"/>\n", backgroundColor, dimension, dimension);
    // Data cells
    fprintf(file, "    <!-- Data cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", dataColor);
    UnsignedByte** data = board.buffer;
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == 0) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Finder cells
    fprintf(file, "    <!-- Finder cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", finderColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellFinder) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Timing cells
    fprintf(file, "    <!-- Timing cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", timingColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellTiming) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Aligment cells
    fprintf(file, "    <!-- Aligment cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", aligmentColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellAlignment) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Version cells
    fprintf(file, "    <!-- Version info cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", versionColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellVersion) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Format cells
    fprintf(file, "    <!-- Format info cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", formatColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellFormat) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Dark cell
    fprintf(file, "    <!-- Dark cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", darkColor);
    UnsignedByte cell = data[board.dimension - 8][8];
    UnsignedByte high = cell & CellHighMask;
    UnsignedByte low = cell & CellLowMask;
    if (low == CellSet && high == CellDark) {
        unsigned int x = (quietZone + 8) * scale;
        unsigned int y = (quietZone + board.dimension - 8) * scale;
        fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
    }
    fprintf(file, "    </g>\n");
    // ErrorCorrection cells
    fprintf(file, "    <!-- Error correction cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", ecColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellErrorCorrection) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // Remainder cells
    fprintf(file, "    <!-- Remainder cells -->\n");
    fprintf(file, "    <g fill=\"%s\" stroke=\"none\">\n", remainderColor);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte high = cell & CellHighMask;
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet && high == CellRemainder) {
                unsigned int x = (quietZone + column) * scale;
                unsigned int y = (quietZone + row) * scale;
                fprintf(file, "        <rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\"/>\n", scale, scale, x, y);
            }
        }
    }
    fprintf(file, "    </g>\n");
    // </svg>
    fprintf(file, "</svg>\n");
    return fclose(file);
}
