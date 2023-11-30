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

import 'dart:convert';
import 'dart:io';
import 'package:flutter_isolate/flutter_isolate.dart';

import 'package:flutter/material.dart';
import 'dart:async';
import 'dart:ui' as ui;

import 'package:qrmatrixexample/qrmatrixexample.dart' as qrmatrixexample;

void main() {
  qrmatrixexample.initQr();
  ui.DartPluginRegistrant.ensureInitialized();
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class MakeQrParameter {
  String name = "";
  List<int>? raw;
  String text = "";
  qrmatrixexample.QrMode mode = qrmatrixexample.QrMode.numeric;
  qrmatrixexample.QrEcLevel level = qrmatrixexample.QrEcLevel.high;
  int eci = qrmatrixexample.qrDefaultEci;
  bool isAuto = false;
  qrmatrixexample.QrExtra extra = qrmatrixexample.QrExtra();

  Map<String, String> toMap() {
    return {
      "raw": raw?.map((e) => e.toString()).join(",") ?? "",
      "name": name,
      "text": text,
      "mode": mode.index.toString(),
      "level": level.index.toString(),
      "eci": eci.toString(),
      "isAuto": isAuto.toString(),
      "extraMode": extra.mode.index.toString(),
      "extraAppId": extra.fnc1SecondAppId
    };
  }

  void import(Map<String, String> map) {
    if (map["raw"] != null && map["raw"]!.isNotEmpty) {
      raw = map["raw"]!.split(",").map((e) => int.parse(e)).toList();
    } else {
      raw = null;
    }
    name = map["name"]!;
    text = map["text"]!;
    mode = qrmatrixexample.QrMode.values[int.parse(map["mode"]!)];
    level = qrmatrixexample.QrEcLevel.values[int.parse(map["level"]!)];
    eci = int.parse(map["eci"]!);
    isAuto = map["isAuto"] == "true";
    extra.mode = qrmatrixexample.QrExtraMode.values[int.parse(map["extraMode"]!)];
    extra.fnc1SecondAppId = map["extraAppId"]!;
  }
}

class _MyAppState extends State<MyApp> {
  var images = <String, MemoryImage>{};
  var texts = <String>[];
  var currentIndex = 0;

  // Dart Isolate allows to send `Map<String, MemoryImage>` back to main isolate,
  // but not allow to access UI methods (PictureRecorder, Canvas) in detached isolate.
  // So I need to use Flutter Isolate here.
  // But I can not send `Map<String, MemoryImage or ByteArray>` via Flutter Isolate,
  // so I use `Map<String, String>` instead
  // (where Bytes of PNG image is converted to Base64 string).
  // Also, Flutter Isolate is available only on iOS and Android.
  @pragma('vm:entry-point')
  static Future<Map<String, String>> drawQRImage(Map<String, String> map) async {
    final param = MakeQrParameter();
    param.import(map);
    ui.Image image;
    if (param.isAuto) {
      image = qrmatrixexample.makeQrAutoSegments(param.text, param.level, param.extra, 10);
    } else {
      final segment = qrmatrixexample.QrSegment();
      segment.mode = param.mode;
      segment.eci = param.eci;
      segment.text = param.text;
      segment.raw = param.raw;
      image = qrmatrixexample.makeQr([segment], param.level, param.extra, 10);
    }
    final data = await image.toByteData(format: ui.ImageByteFormat.png);
    if (data != null) {
      return {param.name: base64Encode(data.buffer.asUint8List())};
    }
    return {};
  }

  @pragma('vm:entry-point')
  static Future<Map<String, String>> drawStructuredQRImage(String whatever) async {
    final seg11 = qrmatrixexample.QrSegment();
    seg11.text = "123";
    seg11.mode = qrmatrixexample.QrMode.numeric;
    final seg12 = qrmatrixexample.QrSegment();
    seg12.text = "ABC";
    seg12.mode = qrmatrixexample.QrMode.alphanumeric;
    final seg21 = qrmatrixexample.QrSegment();
    seg21.text = "456";
    seg21.mode = qrmatrixexample.QrMode.numeric;
    final seg22 = qrmatrixexample.QrSegment();
    seg22.text = "DEF";
    seg22.mode = qrmatrixexample.QrMode.alphanumeric;
    final images = qrmatrixexample.makeQrStructuredAppend([[seg11, seg12], [seg21, seg22]], qrmatrixexample.QrEcLevel.high, 10);
    final result = <String, String>{};
    for (int index = 0; index < images.length; index++) {
      final image = images[index];
      final data = await image.toByteData(format: ui.ImageByteFormat.png);
      if (data != null) {
        result["Structured Append ${index + 1}"] = base64Encode(data.buffer.asUint8List());
      }
    }
    return result;
  }

  void handleResult(Map<String, String> result) {
    if (result.isEmpty) return;
    for (final text in result.keys) {
      final hexValue = result[text];
      if (hexValue != null) {
        final data = base64Decode(hexValue);
        final image = MemoryImage(data);
        if (currentIndex == texts.length) {
          setState(() {
            texts.add(text);
            images[text] = image;
          });
        } else {
          texts.add(text);
          images[text] = image;
        }
      }
    }
  }

  void runExamples(MakeQrParameter param) {
    if (Platform.isIOS || Platform.isAndroid) {
      flutterCompute(drawQRImage, param.toMap()).then((value) {
        handleResult(value);
      });
    } else {
      // On Desktop, we can not use Isolate, so we delay each task a bit to unfreeze UI.
      Future.delayed(const Duration(milliseconds: 100), () {
        _MyAppState.drawQRImage(param.toMap()).then((value) {
          handleResult(value);
        });
      });
    }
  }

  void launchNumericExample() {
    final param = MakeQrParameter();
    param.text = "12345678901234567";
    param.name = "Numeric: 12345678901234567";
    param.mode = qrmatrixexample.QrMode.numeric;
    runExamples(param);
  }

  void launchAlphaNumericExample() {
    final param = MakeQrParameter();
    param.text = "ABC\$ 67890";
    param.name = "AlphaNumeric: ABC\$ 67890";
    param.mode = qrmatrixexample.QrMode.alphanumeric;
    runExamples(param);
  }

  void launchLatin1DirectExample() {
    final param = MakeQrParameter();
    param.raw = List<int>.filled(7, 0);
    param.raw![0] = 0x4c;
    param.raw![1] = 0x31;
    param.raw![2] = 0x21;
    param.raw![3] = 0x20;
    param.raw![4] = 0xa9;
    param.raw![5] = 0xc2;
    param.raw![6] = 0xe2;
    param.name = "Latin1 (direct): L1! ©Ââ";
    param.mode = qrmatrixexample.QrMode.byte;
    runExamples(param);
  }

  void launchLatin1Example() {
    final param = MakeQrParameter();
    param.raw = latin1.encode("L1! ©Ââ");
    param.name = "Latin1: L1! ©Ââ";
    param.mode = qrmatrixexample.QrMode.byte;
    runExamples(param);
  }

  void launchKanjiExample() {
    final param = MakeQrParameter();
    param.text = "０Ａあア";
    param.name = "Kanji: ０Ａあア";
    param.mode = qrmatrixexample.QrMode.kanji;
    runExamples(param);
  }

  void launchKanjiDirectExample() {
    final param = MakeQrParameter();
    param.raw = List<int>.filled(8, 0);
    param.raw![0] = 0x82;
    param.raw![1] = 0x4f;
    param.raw![2] = 0x82;
    param.raw![3] = 0x60;
    param.raw![4] = 0x82;
    param.raw![5] = 0xa0;
    param.raw![6] = 0x83;
    param.raw![7] = 0x41;
    param.name = "Kanji (direct): ０Ａあア";
    param.mode = qrmatrixexample.QrMode.kanji;
    runExamples(param);
  }

  void launchUtf8BytesExample() {
    final param = MakeQrParameter();
    param.text = "Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    param.name = "UTF-8 No ECI";
    param.mode = qrmatrixexample.QrMode.byte;
    runExamples(param);
  }

  void launchUtf8EciExample() {
    final param = MakeQrParameter();
    param.text = "Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    param.name = "UTF-8 with ECI";
    param.mode = qrmatrixexample.QrMode.byte;
    param.eci = 26;
    runExamples(param);
  }

  void launchCustomECIExample() {
    final param = MakeQrParameter();
    param.raw = List<int>.filled(11, 0);
    param.raw![0] = 0xbe;
    param.raw![1] = 0xc8;
    param.raw![2] = 0xb3;
    param.raw![3] = 0xe7;
    param.raw![4] = 0xc7;
    param.raw![5] = 0xcf;
    param.raw![6] = 0xbc;
    param.raw![7] = 0xbc;
    param.raw![8] = 0xbf;
    param.raw![9] = 0xe4;
    param.raw![10] = 0x21;
    param.name = "Custom ECI(KS X 1001): 안녕하세요!";
    param.mode = qrmatrixexample.QrMode.byte;
    param.eci = 30;
    runExamples(param);
  }

  void launchMicroNumericExample() {
    final param = MakeQrParameter();
    param.text = "12345";
    param.name = "Micro Numeric: 12345";
    param.mode = qrmatrixexample.QrMode.numeric;
    param.extra.mode = qrmatrixexample.QrExtraMode.micro;
    param.level = qrmatrixexample.QrEcLevel.low;
    runExamples(param);
  }

  void launchMicroAlphaNumericExample() {
    final param = MakeQrParameter();
    param.text = "A12345";
    param.name = "Micro AlphaNumeric: A12345";
    param.mode = qrmatrixexample.QrMode.alphanumeric;
    param.extra.mode = qrmatrixexample.QrExtraMode.micro;
    param.level = qrmatrixexample.QrEcLevel.low;
    runExamples(param);
  }

  void launchMicroBytesExample() {
    final param = MakeQrParameter();
    param.text = "안녕";
    param.name = "Micro byte: 안녕";
    param.mode = qrmatrixexample.QrMode.byte;
    param.extra.mode = qrmatrixexample.QrExtraMode.micro;
    param.level = qrmatrixexample.QrEcLevel.low;
    runExamples(param);
  }

  void launchMicroKanjiExample() {
    final param = MakeQrParameter();
    param.text = "０Ａあア";
    param.name = "Micro Kanji: ０Ａあア";
    param.mode = qrmatrixexample.QrMode.kanji;
    param.extra.mode = qrmatrixexample.QrExtraMode.micro;
    param.level = qrmatrixexample.QrEcLevel.low;
    runExamples(param);
  }

  void launchAutoSegments() {
    final param = MakeQrParameter();
    param.text = "123456789 こんにちは世界！A56B 안녕하세요!";
    param.name = "Auto mix modes";
    param.isAuto = true;
    runExamples(param);
  }

  void launchMicroAutoSegments() {
    final param = MakeQrParameter();
    param.text = "1234こんにちは";
    param.name = "Auto micro mix modes";
    param.isAuto = true;
    param.extra.mode = qrmatrixexample.QrExtraMode.micro;
    param.level = qrmatrixexample.QrEcLevel.low;
    runExamples(param);
  }

  void launchFnc1FirstExample() {
    final param = MakeQrParameter();
    param.text = "01049123451234591597033130128%10ABC123";
    param.name = "FNC1 First";
    param.isAuto = true;
    param.extra.mode = qrmatrixexample.QrExtraMode.fnc1First;
    runExamples(param);
  }

  void launchFnc1SecondExample() {
    final param = MakeQrParameter();
    param.text = "AA1234BBB112text text text text\n";
    param.name = "FNC1 Second";
    param.isAuto = true;
    param.extra.mode = qrmatrixexample.QrExtraMode.fnc1Second;
    param.extra.fnc1SecondAppId = "37";
    runExamples(param);
  }

  void launchStructuredAppendExample() {
    if (Platform.isIOS || Platform.isAndroid) {
      flutterCompute(drawStructuredQRImage, "").then((value) {
        handleResult(value);
      });
    } else {
      Future.delayed(const Duration(milliseconds: 100), () {
        _MyAppState.drawStructuredQRImage("").then((value) {
          handleResult(value);
        });
      });
    }
  }

  void launchExamples() {
    launchNumericExample();
    launchAlphaNumericExample();
    launchLatin1Example();
    launchLatin1DirectExample();
    launchKanjiExample();
    launchKanjiDirectExample();
    launchUtf8BytesExample();
    launchUtf8EciExample();
    launchCustomECIExample();
    launchAutoSegments();

    launchMicroNumericExample();
    launchMicroAlphaNumericExample();
    launchMicroBytesExample();
    launchMicroKanjiExample();
    launchMicroAutoSegments();

    launchFnc1FirstExample();
    launchFnc1SecondExample();
    launchStructuredAppendExample();
  }

  @override
  void initState() {
    super.initState();
    launchExamples();
  }

  void _preButtonOnClick() {
    if (currentIndex == 0) {
      if (texts.isNotEmpty) {
        setState(() {
          currentIndex = texts.length - 1;
        });
      }
    } else {
      setState(() {
        currentIndex -= 1;
      });
    }
  }

  void _nextButtonOnClic() {
    if (currentIndex + 1 < texts.length) {
      setState(() {
        currentIndex += 1;
      });
    } else {
      setState(() {
        currentIndex = 0;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('QRMatrixExample'),
        ),
        backgroundColor: Colors.grey,
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: Column(
              children: [
                SizedBox(
                  height: 32,
                  child: Text(texts.isEmpty ? "Loading ..." : texts.elementAt(currentIndex)),
                ),
                const SizedBox(height: 10),
                Row(
                  children: [
                    Expanded(child: TextButton(
                      style: ButtonStyle(backgroundColor: MaterialStateProperty.all<Color>(Colors.lightGreen)),
                      onPressed: _preButtonOnClick,
                      child: const Text("<<")
                    )),
                    const SizedBox(width: 10),
                    Expanded(child:  TextButton(
                      style: ButtonStyle(backgroundColor: MaterialStateProperty.all<Color>(Colors.lightGreen)),
                      onPressed: _nextButtonOnClic,
                      child: const Text(">>")
                    ))
                  ],
                ),
                const SizedBox(height: 10),
                texts.isEmpty ?
                const Text("Loading..") :
                Image(
                  image: images[texts.elementAt(currentIndex)]!,
                )
              ],
            ),
          ),
        ),
      ),
    );
  }
}
