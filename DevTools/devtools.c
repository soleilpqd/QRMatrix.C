/*
    QRMatrix - QR pixels presentation.
    Copyright © 2023 duongpq/soleilpqd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "devtools.h"
#include <stdio.h>
#include <stdbool.h>

void DevPrintSingleHex(unsigned char value) {
    printf("%02X\n", value);
}

void DevPrintHex(unsigned char* value, int count) {
    for (int idx = 0; idx < count; idx++) {
        if (idx != count - 1) {
            printf("%02X ", value[idx]);
        } else {
            printf("%02X\n", value[idx]);
        }
    }
}

void DevPrintSingleBin_(unsigned char value, bool enableNewLine) {
    unsigned int pattern = 0x01 << 7;
    for (int idx = 0; idx < 8; idx++) {
        unsigned char mask = pattern >> idx;
        printf("%s", (value & mask) > 0 ? "1" : "0");
    };
    if (enableNewLine) {
        printf("\n");
    }
}

void DevPrintSingleBin(unsigned char value) {
    DevPrintSingleBin_(value, true);
}

void DevPrintBin(unsigned char* value, int count) {
    printf("BIN      │ 0x │ Dec │ Id\n─────────┼────┼─────┼─────\n");
    for (int idx = 0; idx < count; idx++) {
        DevPrintSingleBin_(value[idx], false);
        printf(" │ %02X │ ", value[idx]);
        if (value[idx] < 100) {
            printf(" ");
        }
        if (value[idx] < 10) {
            printf(" ");
        }
        printf("%d │ %d\n", value[idx], (idx + 1) * 8);
    }
    printf("─────────┴────┴─────┴─────\n");
}
