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

import 'dart:ffi';
import 'dart:io';
import 'dart:ui';
import 'dart:convert';
import 'package:flutter/material.dart' hide Image;

import 'package:ffi/ffi.dart';

import 'qrmatrixexample_bindings_generated.dart';

const String _libName = 'qrmatrixexample';

enum QrMode {
  numeric,
  alphanumeric,
  byte,
  kanji
}

enum QrEcLevel {
  low,
  medium,
  quartile,
  high
}

enum QrExtraMode {
  none,
  micro,
  fnc1First,
  fnc1Second
}

class QrExtra {
  var mode = QrExtraMode.none;
  var fnc1SecondAppId = "";
}

const qrDefaultEci = DEFAULT_ECI_ASSIGMENT;

class QrSegment {
  var text = "";
  QrMode mode = QrMode.byte;
  List<int>? raw;
  var eci = 0;
}

/// The dynamic library in which the symbols for [QrmatrixexampleBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final QrmatrixexampleBindings _bindings = QrmatrixexampleBindings(_dylib);

void initQr() {
  _bindings.QRMatrixInit();
}

int _mapQrMode(QrMode mode) {
  switch (mode) {
    case QrMode.numeric:
      return QrmEncodingMode.EModeNumeric;
    case QrMode.alphanumeric:
      return QrmEncodingMode.EModeAlphaNumeric;
    case QrMode.byte:
      return QrmEncodingMode.EModeByte;
    case QrMode.kanji:
      return QrmEncodingMode.EModeKanji;
  }
}

int _mapQrEcLevel(QrEcLevel level) {
  switch (level) {
    case QrEcLevel.low:
      return QrmErrorCorrectionLevel.ELevelLow;
    case QrEcLevel.medium:
      return QrmErrorCorrectionLevel.ELevelMedium;
    case QrEcLevel.quartile:
      return QrmErrorCorrectionLevel.ELevelQuarter;
    case QrEcLevel.high:
      return QrmErrorCorrectionLevel.ELevelHigh;
  }
}

Image _drawQR(Pointer<QrmBoard> boardPtr, int scale, bool isMicro) {
  final recorder = PictureRecorder();
  final canvas = Canvas(recorder);
  final paint = Paint();
  paint.color = Colors.white;
  paint.style = PaintingStyle.fill;
  final quietZone = isMicro ? 2 : 4;
  final qrSize = ((quietZone * 2) + boardPtr.ref.dimension - 1) * scale;
  canvas.drawRect(Rect.fromLTWH(0, 0, qrSize.toDouble(), qrSize.toDouble()), paint);
  paint.color = Colors.black;
  for (int row = 0; row < boardPtr.ref.dimension; row += 1) {
    for (int column = 0; column < boardPtr.ref.dimension; column += 1) {
      final cell = boardPtr.ref.buffer[row][column];
      final low = cell & QrmBoardCell.CellLowMask;
      if (low == QrmBoardCell.CellSet) {
        final x = (row + quietZone) * scale;
        final y = (column + quietZone) * scale;
        canvas.drawRect(Rect.fromCenter(
          center: Offset(x.toDouble(), y.toDouble()),
          width: scale.toDouble(),
          height: scale.toDouble()
        ), paint);
      }
    }
  }
  final pic = recorder.endRecording();
  final image = pic.toImageSync(qrSize, qrSize);
  return image;
}

Pointer<QrmSegment> _convertSegment(QrSegment segment) {
  int mode = _mapQrMode(segment.mode);
  List<int> raw;
  if (segment.raw != null) {
    raw = segment.raw!;
  } else {
    raw = utf8.encode(segment.text);
  }
  final rawPtr = calloc<UnsignedByte>(raw.length);
  for (var index = 0; index < raw.length; index += 1) {
    rawPtr[index] = raw[index];
  }

  if (segment.raw == null && segment.mode == QrMode.kanji) {
    // Input is text & mode Kanji => require to encode text in Shift-JIS
    final utf8Ptr = malloc<Utf8String>(sizeOf<Utf8String>())..ref = _bindings.U8Create(rawPtr, raw.length);
    final unicodesPtr = malloc<UnicodePoint>(sizeOf<UnicodePoint>())..ref = _bindings.U8ToUnicodes(utf8Ptr.ref);
    _bindings.U8Destroy(utf8Ptr);
    malloc.free(utf8Ptr);
    final shiftjisPtr = malloc<ShiftJisString>(sizeOf<ShiftJisString>())..ref = _bindings.SjCreateFromUnicodes(unicodesPtr.ref);
    _bindings.UPDestroy(unicodesPtr);
    malloc.free(unicodesPtr);
    Pointer<QrmSegment> segmentPtr = malloc<QrmSegment>(sizeOf<QrmSegment>())..ref = _bindings.QrmSegCreate(mode, shiftjisPtr.ref.raw, shiftjisPtr.ref.byteCount, segment.eci);
    _bindings.SjDestroy(shiftjisPtr);
    malloc.free(shiftjisPtr);
    calloc.free(rawPtr);
    return segmentPtr;
  }

  Pointer<QrmSegment> segmentPtr = malloc<QrmSegment>(sizeOf<QrmSegment>())..ref = _bindings.QrmSegCreate(mode, rawPtr, raw.length, segment.eci);
  calloc.free(rawPtr);
  return segmentPtr;
}

Pointer<QrmExtraEncodingInfo> _convertExtra(QrExtra extra) {
  switch (extra.mode) {
    case QrExtraMode.none:
      return malloc<QrmExtraEncodingInfo>(sizeOf<QrmExtraEncodingInfo>())..ref = _bindings.QrmExtraCreate(QrmExtraMode.XModeNone);
    case QrExtraMode.micro:
      return malloc<QrmExtraEncodingInfo>(sizeOf<QrmExtraEncodingInfo>())..ref = _bindings.QrmExtraCreate(QrmExtraMode.XModeMicroQr);
    case QrExtraMode.fnc1First:
      return malloc<QrmExtraEncodingInfo>(sizeOf<QrmExtraEncodingInfo>())..ref = _bindings.QrmExtraCreate(QrmExtraMode.XModeFnc1First);
    case QrExtraMode.fnc1Second: {
      final raw = utf8.encode(extra.fnc1SecondAppId);
      final rawPtr = calloc<UnsignedByte>(raw.length);
      for (var index = 0; index < raw.length; index += 1) {
        rawPtr[index] = raw[index];
      }
      final result = malloc<QrmExtraEncodingInfo>(sizeOf<QrmExtraEncodingInfo>())..ref = _bindings.QrmExtraCreateFnc1Second(rawPtr, raw.length);
      calloc.free(rawPtr);
      return result;
    }
  }
}

Image makeQr(List<QrSegment> segments, QrEcLevel level, QrExtra extra, int scale) {
  final segmentsArray = calloc<QrmSegment>(segments.length * sizeOf<QrmSegment>());
  final segmentPtrs = List<Pointer<QrmSegment>>.filled(segments.length, nullptr);
  for (int index = 0; index < segments.length; index += 1) {
    segmentPtrs[index] = _convertSegment(segments[index]);
    segmentsArray[index] = segmentPtrs[index].ref;
  }
  int llevel = _mapQrEcLevel(level);
  final extraPtr = _convertExtra(extra);
  final boardPtr = malloc<QrmBoard>(sizeOf<QrmBoard>())..ref = _bindings.QrmEncoderEncode(segmentsArray, segments.length, llevel, extraPtr.ref, 0, 0xFF);

  _bindings.QrmExtraDestroy(extraPtr);
  malloc.free(extraPtr);

  for (int index = 0; index < segments.length; index += 1) {
    _bindings.QrmSegDestroy(segmentPtrs[index]);
    malloc.free(segmentPtrs[index]);
  }
  calloc.free(segmentsArray);

  if (boardPtr.ref.dimension == 0) {
    _bindings.QrmBoardDestroy(boardPtr);
    malloc.free(boardPtr);
    throw("Fail to create QR code");
  }

  final image = _drawQR(boardPtr, scale, extra.mode == QrExtraMode.micro);
  _bindings.QrmBoardDestroy(boardPtr);
  malloc.free(boardPtr);

  return image;
}

Image makeQrAutoSegments(String text, QrEcLevel level, QrExtra extra, int scale) {
  int llevel = _mapQrEcLevel(level);
  final raw = utf8.encode(text);
  final rawPtr = calloc<UnsignedByte>(raw.length);
  for (var index = 0; index < raw.length; index += 1) {
    rawPtr[index] = raw[index];
  }
  final utf8Ptr = malloc<Utf8String>(sizeOf<Utf8String>())..ref = _bindings.U8Create(rawPtr, raw.length);
  final unicodesPtr = malloc<UnicodePoint>(sizeOf<UnicodePoint>())..ref = _bindings.U8ToUnicodes(utf8Ptr.ref);
  _bindings.U8Destroy(utf8Ptr);
  malloc.free(utf8Ptr);
  final segCountPtr = calloc<UnsignedInt>(sizeOf<UnsignedInt>());
  final segments = _bindings.UPMakeSegments(unicodesPtr.ref, llevel, segCountPtr, extra.mode == QrExtraMode.micro);
  _bindings.UPDestroy(unicodesPtr);
  malloc.free(unicodesPtr);
  if (segCountPtr.value == 0) {
    calloc.free(segCountPtr);
    throw("Fail to create segments");
  }
  final extraPtr = _convertExtra(extra);
  final boardPtr = malloc<QrmBoard>(sizeOf<QrmBoard>())..ref = _bindings.QrmEncoderEncode(segments, segCountPtr.value, llevel, extraPtr.ref, 0, 0xFF);

  _bindings.QrmExtraDestroy(extraPtr);
  malloc.free(extraPtr);
  for (int index = 0; index < segCountPtr.value; index += 1) {
    _bindings.QrmSegDestroy(segments.elementAt(index));
  }
  calloc.free(segments);
  calloc.free(segCountPtr);

  if (boardPtr.ref.dimension == 0) {
    _bindings.QrmBoardDestroy(boardPtr);
    malloc.free(boardPtr);
    throw("Fail to create QR code");
  }

  final image = _drawQR(boardPtr, scale, extra.mode == QrExtraMode.micro);
  _bindings.QrmBoardDestroy(boardPtr);
  malloc.free(boardPtr);

  return image;
}

List<Image> makeQrStructuredAppend(List<List<QrSegment>> parts, QrEcLevel level, int scale) {
  int llevel = _mapQrEcLevel(level);
  final partPtrs = List<Pointer<QrmStructuredAppend>>.filled(parts.length, nullptr);
  final partArray = calloc<QrmStructuredAppend>(parts.length * sizeOf<QrmStructuredAppend>());
  for (int idx = 0; idx < parts.length; idx += 1) {
    final part = parts[idx];
    final segmentPtrs = List<Pointer<QrmSegment>>.empty(growable: true);
    final segmentsArray = calloc<QrmSegment>(part.length * sizeOf<QrmSegment>());
    for (int jdx = 0; jdx < part.length; jdx += 1) {
      final segmentPtr = _convertSegment(part[jdx]);
      segmentPtrs.add(segmentPtr);
      segmentsArray[jdx] = segmentPtr.ref;
    }
    final partPtr = malloc<QrmStructuredAppend>(sizeOf<QrmStructuredAppend>())..ref = _bindings.QrmStrAppCreate(segmentsArray, part.length, llevel);
    partPtrs[idx] = partPtr;
    partArray[idx] = partPtr.ref;
    for (final segmentPtr in segmentPtrs) {
      _bindings.QrmSegDestroy(segmentPtr);
      malloc.free(segmentPtr);
    }
    calloc.free(segmentsArray);
  }
  final boards = _bindings.QrmEncoderMakeStructuredAppend(partArray, parts.length);
  for (final part in partPtrs) {
    _bindings.QrmStrAppDestroy(part);
    malloc.free(part);
  }
  calloc.free(partArray);
  final result = List<Image>.empty(growable: true);
  for (int idx = 0; idx < parts.length; idx += 1) {
    final boardPtr = boards.elementAt(idx);
    if (boardPtr.ref.dimension > 0) {
      final image = _drawQR(boardPtr, scale, false);
      result.add(image);
    }
    _bindings.QrmBoardDestroy(boardPtr);
  }
  calloc.free(boards);
  return result;
}
