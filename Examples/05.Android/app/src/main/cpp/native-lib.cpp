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

#include <jni.h>
#include <string>
#include <iostream>
#include <list>
#include <map>

extern "C" {
#include "../../../../../../QRMatrix/qrmatrixencoder.h"
#include "../../../../../../String/utf8string.h"
#include "../../../../../../String/shiftjisstring.h"
#include "../../../../../../String/latinstring.h"
}

using namespace std;

enum QRExampleMode {
    mNumeric = 1,
    mAlphaNumeric = 2,
    mByte = 3,
    mKanji = 4
};

enum QRExampleECLevel {
    lLow = 1,
    lMedium = 2,
    lQuarter = 3,
    lHigh = 4
};

enum QRExampleExtraMode {
    mNone = 1,
    mMicroQR = 2,
    mFNC1First = 3,
    mFNC1Second = 4
};

map<int, list<QrmStructuredAppend*>*> structuredAppendJobs;
map<int, list<QrmBoard*>*> structuredAppendResults;

jbyteArray errorResult(string mesg, JNIEnv* env) {
    int len = mesg.size();
    jbyteArray result = env->NewByteArray(len + 1);
    char value = 1;
    env->SetByteArrayRegion(result, 0, 1, (jbyte *)&value);
    env->SetByteArrayRegion(result, 1, len, (jbyte *)mesg.data());
    return result;
}

jbyteArray successResult(QrmBoard board, JNIEnv* env) {
    int len = board.dimension;
    len = len * len + 1;
    char value = 0;
    int offset = 0;
    jbyteArray result = env->NewByteArray(len + 1);
    env->SetByteArrayRegion(result, offset, 1, (jbyte *)&value);
    offset += 1;
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        env->SetByteArrayRegion(result, offset, board.dimension, (jbyte *)board.buffer[row]);
        offset += board.dimension;
    }
    return result;
}

QrmEncodingMode convertMode(jint mode) {
    switch (mode) {
        case QRExampleMode::mNumeric:
            return EModeNumeric;
        case QRExampleMode::mAlphaNumeric:
            return EModeAlphaNumeric;
        case QRExampleMode::mByte:
            return EModeByte;
        case QRExampleMode::mKanji:
            return EModeKanji;
    }
}

QrmErrorCorrectionLevel convertEcLevel(jint level) {
    QrmErrorCorrectionLevel ecLevel;
    switch (level) {
        case QRExampleECLevel::lLow:
            ecLevel = ELevelLow;
            break;
        case QRExampleECLevel::lMedium:
            ecLevel = ELevelMedium;
            break;
        case QRExampleECLevel::lQuarter:
            ecLevel = ELevelQuarter;
            break;
        case QRExampleECLevel::lHigh:
            ecLevel = ELevelHigh;
            break;
    }
    return ecLevel;
}

QrmExtraEncodingInfo convertExtraMode(jint extraMode, jstring fnc1SecondAppId, JNIEnv* env) {
    switch (extraMode) {
        case QRExampleExtraMode::mNone:
            return QrmExtraCreateNone();
        case QRExampleExtraMode::mMicroQR:
            return QrmExtraCreate(XModeMicroQr);
        case QRExampleExtraMode::mFNC1First:
            return QrmExtraCreate(XModeFnc1First);
        case QRExampleExtraMode::mFNC1Second: {
            jboolean isCopy = false;
            jint len = env->GetStringLength(fnc1SecondAppId);
            const char* appId = env->GetStringUTFChars(fnc1SecondAppId, &isCopy);
            return QrmExtraCreateFnc1Second((UnsignedByte*)appId, len);
        }
        default:
            return QrmExtraCreateNone();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_qrmatrixexample_MainActivity_initQR(
        JNIEnv* env,
        jobject thiz) {
    QRMatrixInit();
}

/* See description in MainActivity.kt  */
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_makeQR(
        JNIEnv* env,
        jobject thiz,
        jobjectArray datas,
        jintArray modes,
        jbooleanArray needConverts,
        jintArray ecis,
        jint level,
        jint extraMode,
        jstring fnc1SecondAppId
) {
    QrmErrorCorrectionLevel ecLevel = convertEcLevel(level);

    jsize numberOfSegments = env->GetArrayLength(datas);
    QrmSegment segments[numberOfSegments];
    jboolean isCopy = false;
    jint* modeArray = env->GetIntArrayElements(modes, &isCopy);
    jboolean * convertArray = env->GetBooleanArrayElements(needConverts, &isCopy);
    jint* eciArray = env->GetIntArrayElements(ecis, &isCopy);

    for (jsize index = 0; index < numberOfSegments; index += 1) {
        jbyteArray data = (jbyteArray)env->GetObjectArrayElement(datas, index);
        jsize dataLen = env->GetArrayLength(data);
        jint mode = modeArray[index];
        QrmEncodingMode eMode = convertMode(mode);
        jboolean needConvert = convertArray[index];
        jint eci = eciArray[index];

        UnsignedByte *raw = new UnsignedByte [dataLen];
        env->GetByteArrayRegion(data, 0, dataLen, (jbyte*)raw);

        if (needConvert) {
            Utf8String utf8 = U8Create(raw, dataLen);
            delete[] raw;
            UnicodePoint unicodes = U8ToUnicodes(utf8);
            U8Destroy(&utf8);
            switch (eMode) {
                case EModeNumeric:
                case EModeAlphaNumeric:
                case EModeByte: {
                    // Actually, this is not necessary
                    LatinString latin1 = LtCreateFromUnicodes(unicodes);
                    UPDestroy(&unicodes);
                    if (!latin1.isValid) {
                        LtDestroy(&latin1);
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Fail to convert text", env);
                    }
                    QrmSegment segment = QrmSegCreate(eMode, latin1.raw, latin1.length, eci);
                    LtDestroy(&latin1);
                    if (segment.length == 0) {
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Fail to create data segment", env);
                    }
                    segments[index] = segment;
                }
                    break;
                case EModeKanji: {
                    ShiftJisString shiftjis = SjCreateFromUnicodes(unicodes);
                    UPDestroy(&unicodes);
                    if (!shiftjis.isValid || shiftjis.maxBytesPerChar < 2 || shiftjis.minBytesPerChar < 2) {
                        SjDestroy(&shiftjis);
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Only support 2-bytes ShiftJIS Kanji", env);
                    }
                    QrmSegment segment = QrmSegCreate(eMode, shiftjis.raw, shiftjis.charCount * 2, eci);
                    SjDestroy(&shiftjis);
                    if (segment.length == 0) {
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Fail to create data segment", env);
                    }
                    segments[index] = segment;
                }
                    break;
            }
        } else {
            QrmSegment segment = QrmSegCreate(eMode, raw, dataLen, eci);
            delete[] raw;
            if (segment.length == 0) {
                for (jsize idx = 0; idx < index; idx += 1) {
                    QrmSegDestroy(&segments[idx]);
                }
                return errorResult("Fail to create data segment", env);
            }
            segments[index] = segment;
        }
    }
    QrmExtraEncodingInfo extra = convertExtraMode(extraMode, fnc1SecondAppId, env);
    QrmBoard board = QrmEncoderEncode(segments, numberOfSegments, ecLevel, extra, 0, 0xFF);
    QrmExtraDestroy(&extra);
    for (jsize idx = 0; idx < numberOfSegments; idx += 1) {
        QrmSegDestroy(&segments[idx]);
    }
    if (board.dimension > 0) {
        jbyteArray result = successResult(board, env);
        QrmBoardDestroy(&board);
        return  result;
    }
    return errorResult("Fail to make QR", env);
}

/* See description in MainActivity.kt  */
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_makeQRAuto(
        JNIEnv* env,
        jobject thiz,
        jstring text,
        jint level,
        jint extraMode,
        jstring fnc1SecondAppId
) {
    QrmErrorCorrectionLevel ecLevel = convertEcLevel(level);
    jsize utf8len = env->GetStringUTFLength(text);
    jboolean isCopy = false;
    const char* utf8bytes = env->GetStringUTFChars(text, &isCopy);
    Utf8String utf8String = U8Create((const UnsignedByte*)utf8bytes, utf8len);
    UnicodePoint unicodes = U8ToUnicodes(utf8String);
    unsigned int segCount = 0;
    QrmSegment* segments = UPMakeSegments(unicodes, ecLevel, &segCount, extraMode == QRExampleExtraMode::mMicroQR);
    UPDestroy(&unicodes);
    U8Destroy(&utf8String);
    if (segments == NULL) {
        return errorResult("Fail to create segments", env);
    }
    QrmExtraEncodingInfo extra = convertExtraMode(extraMode, fnc1SecondAppId, env);
    QrmBoard board = QrmEncoderEncode(segments, segCount, ecLevel, extra, 0, 0xFF);
    for (unsigned int index = 0; index < segCount; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    free(segments);
    if (board.dimension == 0) {
        return errorResult("Fail to create QR Code", env);
    }
    jbyteArray result = successResult(board, env);
    QrmBoardDestroy(&board);
    return  result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_qrmatrixexample_MainActivity_startMakeStructuredAppendQR(
        JNIEnv* env,
        jobject thiz
){
    for (jint index = 0; index < INT_MAX; index += 1) {
        try {
            list<QrmStructuredAppend*>* list = structuredAppendJobs.at(index);
        } catch (std::exception exception) {
            structuredAppendJobs[index] = new list<QrmStructuredAppend*>;
            return index;
        }
    }
    return -1;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_addStructuredAppendQR(
    JNIEnv* env,
    jobject thiz,
    jint id,
    jobjectArray datas,
    jintArray modes,
    jbooleanArray needConverts,
    jintArray ecis,
    jint level,
    jint extraMode,
    jstring  fnc1SecondAppId
) {
    QrmErrorCorrectionLevel ecLevel = convertEcLevel(level);

    jsize numberOfSegments = env->GetArrayLength(datas);
    QrmSegment segments[numberOfSegments];
    jboolean isCopy = false;
    jint* modeArray = env->GetIntArrayElements(modes, &isCopy);
    jboolean * convertArray = env->GetBooleanArrayElements(needConverts, &isCopy);
    jint* eciArray = env->GetIntArrayElements(ecis, &isCopy);

    for (jsize index = 0; index < numberOfSegments; index += 1) {
        jbyteArray data = (jbyteArray)env->GetObjectArrayElement(datas, index);
        jsize dataLen = env->GetArrayLength(data);
        jint mode = modeArray[index];
        QrmEncodingMode eMode = convertMode(mode);
        jboolean needConvert = convertArray[index];
        jint eci = eciArray[index];

        UnsignedByte *raw = new UnsignedByte [dataLen];
        env->GetByteArrayRegion(data, 0, dataLen, (jbyte*)raw);

        if (needConvert) {
            Utf8String utf8 = U8Create(raw, dataLen);
            delete[] raw;
            UnicodePoint unicodes = U8ToUnicodes(utf8);
            U8Destroy(&utf8);
            switch (eMode) {
                case EModeNumeric:
                case EModeAlphaNumeric:
                case EModeByte: {
                    // Actually, this is not necessary
                    LatinString latin1 = LtCreateFromUnicodes(unicodes);
                    UPDestroy(&unicodes);
                    if (!latin1.isValid) {
                        LtDestroy(&latin1);
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Fail to convert text", env);
                    }
                    QrmSegment segment = QrmSegCreate(eMode, latin1.raw, latin1.length, eci);
                    LtDestroy(&latin1);
                    if (segment.length == 0) {
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Fail to create data segment", env);
                    }
                    segments[index] = segment;
                }
                    break;
                case EModeKanji: {
                    ShiftJisString shiftjis = SjCreateFromUnicodes(unicodes);
                    UPDestroy(&unicodes);
                    if (!shiftjis.isValid || shiftjis.maxBytesPerChar < 2 || shiftjis.minBytesPerChar < 2) {
                        SjDestroy(&shiftjis);
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Only support 2-bytes ShiftJIS Kanji", env);
                    }
                    QrmSegment segment = QrmSegCreate(eMode, shiftjis.raw, shiftjis.charCount * 2, eci);
                    SjDestroy(&shiftjis);
                    if (segment.length == 0) {
                        for (jsize idx = 0; idx < index; idx += 1) {
                            QrmSegDestroy(&segments[idx]);
                        }
                        return errorResult("Fail to create data segment", env);
                    }
                    segments[index] = segment;
                }
                    break;
            }
        } else {
            QrmSegment segment = QrmSegCreate(eMode, raw, dataLen, eci);
            delete[] raw;
            if (segment.length == 0) {
                for (jsize idx = 0; idx < index; idx += 1) {
                    QrmSegDestroy(&segments[idx]);
                }
                return errorResult("Fail to create data segment", env);
            }
            segments[index] = segment;
        }
    }

    QrmExtraEncodingInfo extra = convertExtraMode(extraMode, fnc1SecondAppId, env);
    list<QrmStructuredAppend*>* inputList = nullptr;
    try {
        inputList = structuredAppendJobs.at(id);
    } catch (std::exception exception) {
        inputList = new list<QrmStructuredAppend*>;
        structuredAppendJobs[id] = inputList;
    }
    QrmStructuredAppend part = QrmStrAppCreate(segments, numberOfSegments, ecLevel);
    for (unsigned int index = 0; index < numberOfSegments; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    QrmExtraDestroy(&extra);
    if (part.count == 0) {
        return errorResult("Fail to create Structured Append part data", env);
    }
    ALLOC(QrmStructuredAppend, result, 1);
    *result = part;
    inputList->push_back(result);
    return errorResult("", env);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_commitStructuredAppendQR(
    JNIEnv* env,
    jobject thiz,
    jint id
) {
    list<QrmStructuredAppend*>* inputList = nullptr;
    try {
        inputList = structuredAppendJobs.at(id);
    } catch (std::exception exception) {
        return errorResult("No input", env);
    }
    QrmStructuredAppend parts[inputList->size()];
    if (inputList->size() == 0) {
        return errorResult("No input", env);
    }
    unsigned int index = 0;
    for (QrmStructuredAppend* part: *inputList) {
        parts[index] = *part;
        index += 1;
    }
    QrmBoard* results = QrmEncoderMakeStructuredAppend(parts, inputList->size());
    list<QrmBoard*>* resList = nullptr;
    try {
        resList = structuredAppendResults.at(id);
        for (QrmBoard* board: *resList) {
            delete board;
        }
        delete resList;
        structuredAppendResults.erase(id);
    } catch (std::exception exception) {}
    resList = new list<QrmBoard*>;
    structuredAppendResults[id] = resList;
    for (unsigned int idx = 0; idx < inputList->size(); idx += 1) {
        QrmBoard board = results[idx];
        if (board.dimension > 0) {
            ALLOC(QrmBoard, result, 1);
            *result = board;
            resList->push_back(result);
        }
    }
    return errorResult("", env);
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_qrmatrixexample_MainActivity_getStructuredAppedQRBoard(
    JNIEnv* env,
    jobject thiz,
    jint id,
    jint index
) {
    list<QrmBoard*>* resList = structuredAppendResults[id];
    unsigned int idx = 0;
    for (QrmBoard* board: *resList) {
        if (index == idx) {
            return successResult(*board, env);
        }
        idx += 1;
    }
    return errorResult("Not found", env);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_qrmatrixexample_MainActivity_clearStructuredAppendQR(
    JNIEnv* env,
    jobject thiz,
    jint id
) {
    try {
        list<QrmStructuredAppend*>* inputList = structuredAppendJobs.at(id);
        for (QrmStructuredAppend* part: *inputList) {
            QrmStrAppDestroy(part);
        }
        delete inputList;
        structuredAppendJobs.erase(id);
    } catch (std::exception exception) {}
    try {
        list<QrmBoard*>* resList = structuredAppendResults[id];
        for (QrmBoard* board: *resList) {
            QrmBoardDestroy(board);
        }
        delete resList;
        structuredAppendResults.erase(id);
    } catch (std::exception exception) {}
}
