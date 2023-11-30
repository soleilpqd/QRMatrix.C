# Detail manual

## Vocabulary

[I explain some names, words used in QR Standards and these documents - in my way.](vocabulary.md) (so for exact meaning, please check the ISO documents).

## Source code

To use, you have to import the source code into your project. There are 2 folders of source code:
 - `QRMatrix`: core source.
 - `String`: optional source. This source code handles some string encoding used in QR Code (`UTF-8`, `ShiftJIS`); creates simple data segments for QR encoding process. You can ignore this folder if you handles the text yourself.

## Usage


You only need to include one header file:
```
#include "QRMatrix/qrmatrixencoder.h"
```

### Common:

I use `struct` as value variable and pointer wrapper. The common way to use, by example `QrmSegment`, is:

- Create a `struct XXX` instance by `xxxCreate` function.
```
QrmSegment segment = QrmSegCreate(EModeByte, (const UnsignedByte*)inputText, 18, DEFAULT_ECI_ASSIGMENT);
```

- The `Create` function will create a new memory block for struct instance and copy your input data into it (so remember to free your input data later if required).
- Use `struct` instance property (almost `length`) to check if it is created successfully:
```
if (segment.length == 0) {
    // Fail to create segment
}
```

- Now we can assign this `struct` instance to other variable:
```
QrmSegment other = segment;
```

- When we finish with this `struct` instance, we delete it by call `xxxDestroy` function with a pointer to this `struct` instance:
```
QrmSegDestroy(&segment); // or `QrmSegDestroy(&other)` but not both;
// From this point, do not use `segment` also `other`
```
- It does not require to `destroy` a failed created `struct` instance, but we can still pass a failed `struct` intance into `destroy` function. So a `Create` function must make a pair with `Destroy` function.
- And of course, if you want to manage memory yourself, you still can use `struct` your way.

### Step 1: create data segments

To start encoding a QR Code, you create a single data segment to store all your data or split your data into multiple segments.

```
QrmSegment segment = QrmSegCreate(
    QrmEncodingMode mode,
    const UnsignedByte* data,
    unsigned int length,
    unsigned int eciIndicator
);
```

- `mode`: Mode to encode this data segment (see below).
- `data`: your binary data. Depending on `mode`, this `data` may have to follow some rules.
- `length`: length (in byte) of `data`.
- `eciIndicator`: ECI Indicator value (ECI Assigment Value). Pleas use `DEFAULT_ECI_ASSIGMENT` for default value.

1 QR Code may contain 1 or multiple data segments. Each segment is encoded with a mode.

There are 4 encoding modes:
- `numeric`: this mode encodes only number characters (digits). So `data` must contain only digit ASCII bytes (from `'0'` (`0x30`) to `'9'` (`0x39`)).
- `alphaNumeric`: this mode encodes these ASCII characters (case sensitive): `0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:`.
- `kanji`: this mode encodes Kanji characters which is encoded in ShiftJIS encoding and 2 bytes length per characters. So each 2 bytes of `data` must make a value in range `0x8140 <= x <= 0x9FFC` or `0xE040 <= x <= 0xEBBF`
- `byte`: this mode does not modify `data` (it uses directly input `data`). So there's no direct rule to limit `data` but `data` still have to follow rule of `ECI Protocol`.

`ECI protocol`: this is the base data protocol of QR Code. It defines the Charset/String Encoding of data in the segment (by value of ECI Assigment or ECI Indicator). But 3 modes `numeric`, `alphaNumeric` & `kanji` have their own data limitation, this protocol is almost applied to `byte` mode.
> The default ECI Assigment Value is 3, meaning `ISO/IEC 8859-1` charset (equivalent to Latin1 in Unicodes). So if you use default ECI Assigment Value, `data` must only contain bytes from `ISO/IEC 8859-1` charset.
>
> But `ISO/IEC 8859-1` charset is a subgroup of `UTF-8` (string encoded in `ISO/IEC 8859-1` makes the same bytes as `UTF-8`; so we can detect non-`ISO/IEC 8859-1` characters if we use `UTF-8`). Almost all QR Decoders accept `UTF-8` in this case (mode `byte` with default ECI Assigment value).

But why do we have different modes? That's about the cost.
>  - `B`: number of bits of encoded segment.
>  - `E`: number of bits for ECI prefix. `0` if you use default value, about `4` + `8 ~ 32` for other values.
>  - `M`: number of bits of mode indicator (`4`, MicroQR are variable).
>  - `C`: number of bits of data length indicator (variable, about `3 ~ 16`)
>  - `D`: number of text character of `data`
- `numeric` encodes 3 digits (3 bytes) into 10 bits. Formula: `B = E + M + C + 10 * (D / 3) + R`. `R` is the rest, `= 0, 4, 7` bits depending on the rest of characters modulo 3 (`R=4` if `D % 3 = 1`, `R=7` if `D % 3 = 2`).
- `alphaNumeric` encodes 2 characters (2 bytes) into 11 bits. Formula: `B = E + M + C + 11 * (D / 2) + 6 * (D % 2)`
- `kanji` encodes each character (2 bytes) into 13 bits. Formular: `B = E + M + C + 13 * D`
- `byte` does not modify input data, so formula: `B = E + M + C + d` where `d` is number of bits of `data`.

So if you want to optimize the QR Symbols (to make it containing the biggest data in smallest version), you should split your data into segments with suitable mode.

> You can use this function to make segments from your `UTF-8` string (this is a simple function to split data, no guarantee to make the best):
> ```
> #include "String/utf8string.h""
>
> const char* raw = "Your UTF-8 text!";
> // Make UTF-8 instance
> Utf8String text = U8Create((const UnsignedByte*)raw, strlen(raw));
> // to convert it to Unicodes
> UnicodePoint unicodes = U8ToUnicodes(text);
> // UTF-8 instance is no more needed
> U8Destroy(&text);
> // Create segments
> unsigned int segmentCount = 0;
> QrmErrorCorrectionLevel ecLevel = ELevelHigh;
> QrmSegment* segments = UPMakeSegments(unicodes, ecLevel, &segmentCount, false);
> // Unicodes instance is no more needed
> UPDestroy(&unicodes);
> ...
> // Release memory
> for (unsigned int index = 0; index < segmentCount; index += 1) {
>   QrmSegDestroy(&segments[index]);
> }
> free(segments);
> ```

## Step 2: pass data segments to QR Encoder

```
QrmBoard board = QrmEncoderEncode(
    QrmSegment* segments,
    unsigned int count,
    QrmErrorCorrectionLevel level,
    QrmExtraEncodingInfo extraMode,
    UnsignedByte minVersion,
    UnsignedByte maskId
);
```

- `segments`: your data segments array (pointer).
- `count`: number of item in data segments.
- `level`: Error Correction Level. Error Correction data is some kind of checksum of (a part of) your data, included in QR code as meta-data, which helps QR decoder find and fix errors when it reads the QR Code. Higher EC level covers more input data, but also requires more EC data, so make the QR Code bigger.
- `extraMode`: configure this parameter to create other kinds of QR Code (see below). Pass `QrmExtraCreateNone()` as default value.
- `minVerson`: set the minimum QR version to create. Optional, default value `0`.
- `maskId`: set the QR Mask to use (so not evaluate QR masking). Pass a value > 7 (for simple, use `0xFF`) to enable QR masking evaluation. This is optional parameter, almost for testing.

### Step 2.1: create MicroQR code

To create a MicroQR code, pass to `extraMode` parameter value: `QrmExtraCreate(XModeMicroQr)`

Some notes about MicroQR Code:

- MicroQR has only 4 versions: 1 (11x11) to 4 (17x17).
- Error Correction Level High is not available (this fails with MicroQR version >= 2).
- MicroQR version 1 detects error only (meaning Error Correction Level parameter is ignored - you can pass any value).
- Mode AlphaNumeric is available from MicroQR version 2. Modes Byte and Kanji are available from MicroQR version 3. (eg. if you segment has mode AlphaNumeric, MicroQR code result is version 2 minimum).

### Step 2.2: create FNC1 First position QR Code:

To create FNC1 First position QR code, pass to `extraMode` parameter value: `QrmExtraCreate(XModeFnc1First)`

Note: you should format your data for QRCode instead for GS1:

Example from QR Standard document:

Input data:
 - `0104912345123459`: Application ID `01`; `04912345123459` is Article Number, fixed length.
 - `15970331`: Application ID `15`; `970331` is Date (YYMMDD 97/03/31), fixed length.
 - `30128`: Application ID `30`; data `128`; variable length (requires separator with other data segments).
 - `10ABC%123`: Application ID `10`; data `ABC%123`; variable length.

GS1 data stream: `]Q301049123451234591597033130128<0x1D>10ABC%123` (`0x1D` is the separator character in GS1, `]Q3` is prefix).

Data for QR Code: `01049123451234591597033130128%10ABC%%123` (no prefix, replace GS1 separator character `0x1D` by character `%` - if input data contains `%`, duplicate it (`%%`)). Use this data to create 1 or more segments for QR code; example: 1 segment with `01049123451234591597033130128` and mode Numeric + 1 segment with `%10ABC%%123` and mode Alpha Numeric.

### Step 2.3: create FNC1 Second position QR Code:

To create FNC1 Second position QR code, pass to `extraMode` parameter value: `QrmExtraCreateFnc1Second(UnsignedByte* appId, UnsignedByte appIdLen)` (this requires `QrmExtraDestroy(...)` later), where:
- `appId` is Application Identifier.
- `appIdLen` is the length in bytes of `Aplicaton Identifier`.
- Application Identifier must be a single ASCII character in range `[a-z][A-Z]` (eg. `a`) or a 2-digits number (eg `01`).

### Step 2.4: create Structured Append QR Codes

To create Structured Append QR Codes, you follow the similar work flow (with a more small step) as to create single QR Code.

- Split your data into parts. Each part is data for a single QR Code.
- For each data part, split it into segments as above.
- But instead of passing segments to QR Encoder, create a `QRMatrixStructuredAppend` for each part. `QRMatrixStructuredAppend`'s parameters are the same ones of QR Encoder encoding single QR Code function. (note that `extraMode` `microQR` of `QRMatrixStructuredAppend` will be ignored).
- Pass all `QRMatrixStructuredAppend` to QR Encoder:
```
QrmBoard* boards = QrmEncoderMakeStructuredAppend(
    QrmStructuredAppend* parts,
    unsigned int count
);
```
This function returns an array of `QRMatrixBoard`.


## Step 3: Draw QR Code

QR Encoder returns a `QRMAtrixBoard` representing the QR symbol:

```
// Check
if (board.dimension == 0) {
    // Fail to create QR symbol
}
// Draw
for (unsigned int row = 0; row < board.dimension; row += 1) {
    for (unsigned int column = 0; column < board.dimension; column += 1) {
        UnsignedByte cell = board.buffer[row][column];
        ...
    }
}
// Destroy
QrmSegDestroy(&board);
```

A QR Module (cell) is represented by a byte:
- 4 lower bits are module *color* type: `CellSet` for **black** module, `CellUnset` for **white** module.
- 4 higher bits are module function type: please seee `QrmBoardCell` for more detail.

## Examples

[I describe about examples here.](examples.md)
