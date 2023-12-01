# QRMatrix

> This is a 'downgrade' version of [QRMatrix.CPP](https://github.com/soleilpqd/QRMatrix.CPP) from C++ to C.

This is a library to encode binary string data into QR Code, following the ISO 18004. It supports all 4 encoding modes (Numeric, AlphaNumeric, Kanji, Byte); other QR features: ECI protocol, MicroQR, Structured Append, FNC1.

This lib is made in C, not required any 3rd library.

It's a bit low level. It does not include text processing or drawing features. It takes your binary string data as input, encodes them into a 2D bytes array representing the QR Code symbol. You may have to implement a small drawing process to turn it into an image (please check [the examples](examples.md)).

## Usage

### Source code

For a quick starting, you import the folder [QRMatrix](QRMatrix) into your project.

### Create QR code

You only need to include one header file:

```
#include "QRMatrix/qrmatrixencoder.h"
```

Let's assume we have an UTF-8 C string as input, we will create a QR code with mode Byte and Error Correction level High:

```
const char* inputText = "some text in UTF-8"; // length = 18 bytes
// Create a data segment
QrmSegment segment = QrmSegCreate(EModeByte, (const UnsignedByte*)inputText, 18, DEFAULT_ECI_ASSIGMENT);
QrmSegment segments[] = {segment};
QrmExtraEncodingInfo extra = QrmExtraCreateNone();
QrmBoard board = QrmEncoderEncode(segments, 1, ELevelHigh, extra, 0, 0xFF);
// Clean up (here or later)
QrmSegDestroy(&segment);
QrmExtraDestroy(&extra);
// Draw QR
if (board.dimension > 0) { // success
    UnsignedByte** data = board.buffer;
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = data[row][column];
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet) {
                // This is a QR black module
            }
        }
    }
}
// Clean up
QrmBoardDestroy(&board);
```

For more detail please check [Detail Manual](DOCS/index.md).

You can find some examples project inside folder [Examples](Examples). The description about those examples is [here](DOCS/examples.md). Available examples: SVG, PNG, QT, iOS, MacOS, Android, Flutter.

## Reference

These are documents I used to make this library:

 - [Thonky QR Code Tutorial](https://www.thonky.com/qr-code-tutorial/).
 - [Wikipedia](https://en.wikipedia.org/wiki/QR_code).
 - [Reed-Solomon Error Correction](https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders).
 - [ISO 18004 2000 (Version 1)](https://www.swisseduc.ch/informatik/theoretische_informatik/qr_codes/docs/qr_standard.pdf).
 - [ISO 18004 2006 (Version 2) ~ TCVN 7322:2009](https://luatminhkhue.vn/tieu-chuan-quoc-gia-tcvn-7322-2009-iso-iec-18004-2006-ve-cong-nghe-thong-tin-ky-thuat-phan-dinh-va-thu-nhan-du-lieu-tu-dong-yeu-cau-ky-thuat-doi-voi-ma-hinh-qr-code-2005.aspx).
 - [ISO 18004 2015 (Version 3)](https://github.com/yansikeim/QR-Code/blob/master/ISO%20IEC%2018004%202015%20Standard.pdf).

## License

MIT.

<code>QRMatrix - QR pixels presentation.
Copyright © 2023 duongpq/soleilpqd.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</code>

You can check in [LICENSE file](LICENSE) or on each source code header.
