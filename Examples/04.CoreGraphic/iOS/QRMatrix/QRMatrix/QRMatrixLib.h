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

#import <UIKit/UIKit.h>

#define DEFAULT_ECI 3

typedef NS_ENUM( NSUInteger, QRErrorCorrectionLevel ) {
    ecLow,
    ecMedium,
    ecQuarter,
    ecHigh
};

typedef NS_ENUM( NSUInteger, QRMode ) {
    mNumeric,
    mAlphaNumeric,
    mByte,
    mKanji
};

typedef NS_ENUM( NSUInteger, QRExtraMode ) {
    mMicroQR,
    mFnc1First,
    mFnc1Second
};

@interface QRSegment: NSObject

-( nonnull instancetype )init:( nonnull NSData* )data
                         mode:( QRMode )mode
                          eci:( NSUInteger )eci;

@end

@interface QRExtraData: NSObject

-( nonnull instancetype )init:( QRExtraMode )mode;
+( nonnull instancetype )fnc1Second:( nonnull NSString* )appId;

@end

@interface QRStructuredAppend: NSObject

-( nonnull instancetype )init:( nonnull NSArray<QRSegment*>* )segments
         errorCorrectionLevel:( QRErrorCorrectionLevel )level
                        extra:( nullable QRExtraData* )extra;

@end

@interface QRMatrixLib : NSObject

+( nullable UIImage* )makeMixedModesQR:( nonnull NSString* )text
                  errorCorrectionLevel:( QRErrorCorrectionLevel )level
                                 scale:( NSUInteger )scale
                                 extra:( nullable QRExtraData* )extra
                                 error:( NSError* _Nullable * _Nullable )error;

+( nullable UIImage* )makeQR:( nonnull NSArray<QRSegment*>* )segments
        errorCorrectionLevel:( QRErrorCorrectionLevel )level
                       scale:( NSUInteger )scale
                       extra:( nullable QRExtraData* )extra
                       error:( NSError* _Nullable * _Nullable )error;

+( nullable NSArray<UIImage*>* )makeStructuredAppendQR:( nonnull NSArray<QRStructuredAppend*>* )parts
                                                 scale:( NSUInteger )scale
                                                 error:( NSError* _Nullable * _Nullable )error;

@end
