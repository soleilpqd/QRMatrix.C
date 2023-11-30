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

#import "QRMatrixLib.h"
#import "qrmatrixencoder.h"
#import "utf8string.h"

@interface QRSegment()

@property ( nonatomic, strong ) NSData* data;
@property ( nonatomic, assign ) QRMode mode;
@property ( nonatomic, assign ) NSUInteger eci;

@end

@implementation QRSegment

-( instancetype )init:( NSData* )data mode:( QRMode )mode eci:( NSUInteger )eci {
    self = [ super init ];
    if ( self != nil ) {
        self.data = data;
        self.mode = mode;
        self.eci = eci;
    }
    return self;
}

@end

// MARK: -

@interface QRExtraData()

@property ( nonatomic, assign ) QRExtraMode mode;
@property ( nonatomic, nullable, strong ) NSString* fnc1SecondAppId;

@end

@implementation QRExtraData

-( instancetype )init:( QRExtraMode )mode {
    self = [ super init ];
    self.mode = mode;
    return self;
}

+( instancetype )fnc1Second:( NSString* )appId {
    QRExtraData* result = [[ QRExtraData alloc ] init:mFnc1Second ];
    result.fnc1SecondAppId = appId;
    return result;
}

@end

// MARK: -

@interface QRStructuredAppend()

@property ( nonatomic, strong ) NSArray<QRSegment*>* segments;
@property ( nonatomic, assign ) QRErrorCorrectionLevel ecLevel;
@property ( nonatomic, strong ) QRExtraData* extra;

@end

@implementation QRStructuredAppend

-( instancetype )init:( NSArray<QRSegment*>* )segments errorCorrectionLevel:( QRErrorCorrectionLevel )level extra:( QRExtraData* )extra {
    self = [ super init ];
    self.segments = segments;
    self.ecLevel = level;
    self.extra = extra;
    return self;
}

@end

// MARK: -

@interface QRMatrixLib()

+( nullable UIImage* )imageFromBoard:( QrmBoard )board scale:( NSUInteger )scale isMicro:( BOOL )isMicro;

+( QrmEncodingMode )mapMode:( QRMode )mode;
+( QrmErrorCorrectionLevel )convertECLevel:( QRErrorCorrectionLevel )level;

@end

@implementation QRMatrixLib

+( void )initialize {
    [ super initialize ];
    QRMatrixInit();
    if (!qrmIsEnvValid) {
        [ NSException raise:@"QRMatrix Environment" format:@"Fail to init environment for QRMatrix. Please check `constant.h` and rebuild." ];
    }
}

+( QrmEncodingMode )mapMode:( QRMode )mode {
    switch (mode) {
        case mNumeric:
            return EModeNumeric;
        case mAlphaNumeric:
            return EModeAlphaNumeric;
        case mByte:
            return EModeByte;
        case mKanji:
            return EModeKanji;
    }
}

+( QrmErrorCorrectionLevel )convertECLevel:( QRErrorCorrectionLevel )level {
    QrmErrorCorrectionLevel qrLevel;
    switch (level) {
        case ecLow:
            qrLevel = ELevelLow;
            break;
        case ecMedium:
            qrLevel = ELevelMedium;
            break;
        case ecQuarter:
            qrLevel = ELevelQuarter;
            break;
        case ecHigh:
            qrLevel = ELevelHigh;
            break;
    }
    return qrLevel;
}

+( QrmExtraEncodingInfo )mapExtraMode:( QRExtraData* )extra {
    if (extra != nil) {
        switch (extra.mode) {
            case mMicroQR:
                return QrmExtraCreate(XModeMicroQr);
            case mFnc1First:
                return QrmExtraCreate(XModeFnc1First);
            case mFnc1Second: {
                NSData *data = [ extra.fnc1SecondAppId dataUsingEncoding:NSASCIIStringEncoding ];
                return QrmExtraCreateFnc1Second((UnsignedByte*)data.bytes, data.length);
            }
                break;
        }
    }
    return QrmExtraCreateNone();
}

+( UIImage* )makeQR:( NSArray<QRSegment*>* )segments
errorCorrectionLevel:( QRErrorCorrectionLevel )level
              scale:( NSUInteger )scale
              extra:( nullable QRExtraData* )extra
              error:( NSError** )error; {
    if ( scale < 1 ) {
        *error = [ NSError errorWithDomain:@"QRMatrixLib" code:1 userInfo:@{ NSLocalizedDescriptionKey: @"Invalid scale" }];
        return nil;
    }
    ALLOC(QrmSegment, segs, segments.count);
    for (NSUInteger index = 0; index < segments.count; index += 1) {
        QRSegment* seg = [segments objectAtIndex:index ];
        QrmEncodingMode mode = [ self mapMode:seg.mode ];
        QrmSegFill(&segs[index], mode, (const UnsignedByte*)seg.data.bytes, (unsigned int)seg.data.length, (unsigned int)seg.eci);
    }

    QrmErrorCorrectionLevel qrLevel = [ self convertECLevel:level ];
    QrmExtraEncodingInfo extraMode = [ self mapExtraMode:extra ];

    QrmBoard board = QrmEncoderEncode(segs, (unsigned int)segments.count, qrLevel, extraMode, 0, 0xFF);
    for (NSUInteger index = 0; index < segments.count; index += 1) {
        QrmSegDestroy(&segs[index]);
    }
    QrmExtraDestroy(&extraMode);
    DEALLOC(segs);
    if (board.dimension > 0) {
        UIImage *result = [ self imageFromBoard:board scale:scale isMicro:extra.mode == mMicroQR ];
        QrmBoardDestroy(&board);
        return result;
    } else {
        *error = [ NSError errorWithDomain:@"QRMatrixLib"
                                      code:3
                                  userInfo:@{ NSLocalizedDescriptionKey: @"FAILED"}];
        QrmBoardDestroy(&board);
        return nil;
    }
}

+( UIImage* )imageFromBoard:( QrmBoard )board scale:( NSUInteger )scale isMicro:( BOOL )isMicro {
    CGFloat quietZone = isMicro ? 2.0f : 4.0f;
    CGFloat qrSize = board.dimension * scale + 2.0f * quietZone * scale;
    UIGraphicsBeginImageContextWithOptions( CGSizeMake( qrSize, qrSize ), true, 1.0f );
    CGContextRef context = UIGraphicsGetCurrentContext();
    // Background
    [ UIColor.whiteColor setFill ];
    CGContextFillRect( context, CGRectMake( 0, 0, qrSize, qrSize ));
    // Cells
    [ UIColor.blackColor setFill ];
    for ( NSUInteger row = 0; row < board.dimension; row += 1 ) {
        for ( NSUInteger column = 0; column < board.dimension; column += 1 ) {
            UnsignedByte cell = board.buffer[row][column];
            UnsignedByte low = cell & CellLowMask;
            if ( low == CellSet ) {
                CGContextFillRect( context, CGRectMake( (column + quietZone) * scale, (row + quietZone) * scale, scale, scale ));
            }
        }
    }
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return image;
}

+( UIImage* )makeMixedModesQR:( NSString* )text
         errorCorrectionLevel:( QRErrorCorrectionLevel )level
                        scale:( NSUInteger )scale
                        extra:( nullable QRExtraData* )extra
                        error:( NSError** )error {
    NSData* data = [ text dataUsingEncoding:NSUTF8StringEncoding ];
    Utf8String utf8 = U8Create((const UnsignedByte*)data.bytes, (unsigned int)data.length);
    QrmErrorCorrectionLevel qrLevel = [ self convertECLevel:level ];
    QrmExtraEncodingInfo extraMode = [ self mapExtraMode:extra ];
    bool isMicro = (extraMode.mode == XModeMicroQr);
    unsigned int segCount = 0;
    UnicodePoint unicodes = U8ToUnicodes(utf8);
    QrmSegment* segments = UPMakeSegments(unicodes, qrLevel, &segCount, isMicro);
    U8Destroy(&utf8);
    UPDestroy(&unicodes);
    QrmBoard board = QrmEncoderEncode(segments, segCount, qrLevel, extraMode, 0, 0xFF);
    for (NSUInteger index = 0; index < segCount; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    QrmExtraDestroy(&extraMode);
    DEALLOC(segments);
    if (board.dimension > 0) {
        UIImage *result = [ self imageFromBoard:board scale:scale isMicro:isMicro ];
        QrmBoardDestroy(&board);
        return result;
    } else {
        *error = [ NSError errorWithDomain:@"QRMatrixLib"
                                      code:3
                                  userInfo:@{ NSLocalizedDescriptionKey: @"FAILED"}];
        QrmBoardDestroy(&board);
        return nil;
    }
}

+( NSArray<UIImage*>* )makeStructuredAppendQR:( NSArray<QRStructuredAppend*>* )parts scale:( NSUInteger )scale error:( NSError** )error {
    ALLOC(QrmStructuredAppend, qrParts, parts.count);
    for (NSInteger index = 0; index < parts.count; index += 1) {
        QRStructuredAppend* part = parts[index];
        ALLOC(QrmSegment, segs, part.segments.count);
        for (NSInteger jndex = 0; jndex < part.segments.count; jndex += 1) {
            QRSegment* originSeg = part.segments[jndex];
            QrmSegFill(&segs[jndex], [ self mapMode:originSeg.mode ], (UnsignedByte*)originSeg.data.bytes, (unsigned int)originSeg.data.length, (unsigned int)originSeg.eci);
        }
        qrParts[index] = QrmStrAppCreate(segs, (unsigned int)part.segments.count, [ self convertECLevel:part.ecLevel ]);
        for (NSInteger jndex = 0; jndex < part.segments.count; jndex += 1) {
            QrmSegDestroy(&segs[index]);
        }
        DEALLOC(segs);
        QrmExtraEncodingInfo extra = [ self mapExtraMode:part.extra ];
        qrParts[index].extraMode = extra;
    }
    QrmBoard* boards = QrmEncoderMakeStructuredAppend(qrParts, (unsigned int)parts.count);
    NSMutableArray<UIImage*>* result = [ NSMutableArray new ];
    for (unsigned int index = 0; index < parts.count; index += 1) {
        if (boards[index].dimension > 0) {
            [ result addObject:[ self imageFromBoard:boards[index] scale:scale isMicro:NO ]];
        }
        QrmStrAppDestroy(&qrParts[index]);
        QrmBoardDestroy(&boards[index]);
    }
    DEALLOC(qrParts);
    DEALLOC(boards);
    return result;
}

@end
