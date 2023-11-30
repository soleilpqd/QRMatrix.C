# Examples

## [01.SVG](../Examples/01.SVG/):

This is the most basic example of QRMatrix. It shows how to encode some sample texts into QR Code and write out to SVG files.

> Please edit path at `OUTPUT_PREFIX` in [main.c](../Examples/01.SVG/main.cpp) to store the output SVG files.

- Language: C.
- Tools: CMake 3.27, QT Creator 8.0.1.
- Test platforms: Fedora 37; Mac OS X 12.7.

## [02.LibSPNG](../Examples/02.LibSPNG/):

This example shows how to make a QR Code PNG file.

This example uses `libspng` installed in my system:
- On Fedora, I installed `libspng` (including `devel` package) from Fedora repository.
- On MacOS I use `HomeBrew` to install it.
- Please check [CMakeLists.txt](../Examples/02.LibSPNG/QRMatrixExample/CMakeLists.txt) to configure on your case.

> Please edit path at `OUTPUT_PREFIX` in [main.cpp](../Examples/02.LibSPNG/QRMatrixExample/main.cpp) to store the output PNG files.

- Language: C.
- Tools: CMake 3.27, QT Creator 8.0.1.
- Test platforms: Fedora 37; Mac OS X 12.7.

## [03.QT](../Examples/03.QT/):

This example shows how to make a `QPixmap` QR Code and display it on a window.

And of course, this example requires QT SDK.

- Language: C/C++.
- Tools: CMake 3.27, QT Creator 8.0.1.
- Test platforms: Fedora 37; Mac OS X 12.7.

## [04.CoreGraphic](../Examples/04.CoreGraphic/):

This example shows how to make a `UIImage/NSImage` QR Code in **iOS/MacOS** XCode app project.

It's the same way to use C source code in iOS and MacOS project. We need create a XCode project, import all C source code into it. We create a wrapper class in ObjC to call C functions.

If the project is in Swift, we have to create `bridging.h` header file to allow Swift code using ObjC functions. More simple, we can make a separate framework project for C and wrapper ObjC class. Now the main project sees it as a module and we does not feel the difference of languages. (In this example, please use `QRMatrixExample` project to start, `QRMatrix` is the wrapper framework project).

In this example, I have 2 separated folders for iOS and MacOS. But you will find the codes to draw QR Code `UIImage/NSImage` almost the same (just a little different the step to create `UIImage/NSImage` object).

iOS/MacOS SDK (`ObjC NSData; NSString/Swift Data; String`) supports required String Encoding (Charset) for QR Code, so we don't need source code in [String folder](../String/) to convert text. But I still import them to do example about making QR Segments from [UnicodePoint class](../String/unicodepoint.h).

- Language: C++/ObjC/Swift).
- Tools: XCode 14.
- Test platforms: Mac OS X 12.7, iOS.

## [05.Android](../Examples/05.Android/):

This example shows how to make a `Bitmap` (via `Canvas`) QR Code in Android project.

You need to configure your project to support NDK. So Kotlin source code can call C++/C functions via JNI.

This process is a bit complex. The communication via JNI almost uses binary data. So we have to convert our input data (text) to data type supported by JNI (`jstring`, `jbytearray` ...). Next step is convert j-type datas to C data. And when we return the result, we have to do similar process.

In this example, because Kotlin String `toByteArray` does not support ShiftJIS encoding (and it never fails - it replaces the unable-to-convert characters to `?`), I use code in [String folder](../String/) to convert UTF-8 input string to other encoding string to use in QR Code.

- Language: C/C++/Kotlin.
- Tools: Android Studio Giraffe.

## [06.Flutter](../Examples/06.Flutter/):

This example shows how to make QR Code `dart:ui Image` (using `Paint`, `Canvas` & `PictureRecorder`). Flutter project calls C functions via `dart:ffi`.

This example Flutter project is based on Flutter FFI template:
```
flutter create --template=plugin_ffi --platforms=android,ios,linux,macos qrmatrixexample
```

On files directory tree, the outside Flutter project is plugin project where we make wrapper for C source.

The main app (example app) is located in [example folder](../Examples/06.Flutter/example/).

- To make the C wrapper plugin, the first step is to copy the C source into project. In this example, I just use path to C source in root folder.
- Next, we have to edit [ffigen.yaml](../Examples/06.Flutter/ffigen.yaml) to configure path to (required) headers of C source. Then we execute: `dart run ffigen --config ffigen.yaml`. This command generates file [qrmatrixexample_bindings_generated.dart](../Examples/06.Flutter/lib/qrmatrixexample_bindings_generated.dart) (we may still have to edit this file to add `final` keyword to `class`).
- For each platform, we have to configure C source:
  - For MacOS and iOS, we have to `#import` all C files into [MacOS qrmatrixexample.c](../Examples/06.Flutter/macos/Classes/qrmatrixexample.c) and [iOS qrmatrixexample.c](../Examples/06.Flutter/ios/Classes/qrmatrixexample.c).
  - For Linux and Android, we have to create/edit [Linux CMakeLists.txt](../Examples/06.Flutter/linux/CMakeLists.txt) and [Android CMakeLists.txt](../Examples/06.Flutter/android/CMakeLists.txt). For Android we also need to set path to `CMakeLists.txt` file in [build.gradle](../Examples/06.Flutter/android/build.gradle).
- Now we edit [qrmatrixexample.dart](../Examples/06.Flutter/lib/qrmatrixexample.dart) to make a bridge from Dart code to C code via `dart:ffi`. Here we need to create Dart `enum` and `class` to store data from Dart code, then convert into C `enum` and `struct` via FFI `Pointer`.
- Finally, in [main app](../Examples/06.Flutter/example/lib/main.dart), we can import plugin package to create QR Code Image.

.

- Language: C/Dart.
- Tools: VSCode (XCode, Android Studio, CMake).
- Test platform:  Mac OS X 12.7, iOS, Android 33, Fedora 37.
