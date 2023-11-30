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

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <stdexcept>
#include <QtConcurrent/QtConcurrent>

extern "C" {
#include "../../QRMatrix/qrmatrixencoder.h"
#include "../../String/utf8string.h"
}

QPixmap* imageFromBoard(QrmBoard board, int scale, bool isMicro) {
    UnsignedByte quietZone = isMicro ? 2 : 4;
    int qrSize = (board.dimension + 2 * quietZone) * scale;
    QPixmap* result = new QPixmap(qrSize, qrSize);
    result->fill(QColorConstants::White);
    QPainter painter(result);
    for (unsigned int row = 0; row < board.dimension; row += 1) {
        for (unsigned int column = 0; column < board.dimension; column += 1) {
            UnsignedByte cell = board.buffer[row][column];
            UnsignedByte low = cell & CellLowMask;
            if (low == CellSet) {
                painter.fillRect((column + quietZone) * scale, (row + quietZone) * scale, scale, scale, QColorConstants::Black);
            }
        }
    }
    painter.end();
    return result;
}

QMap<QString, QPixmap*> MainWindow_encode(QString text, const UnsignedByte* raw, QrmEncodingMode mode, unsigned int eci, bool isMicro) {
    QMap<QString, QPixmap*> result;
    QrmSegment segment = QrmSegCreate(mode, raw, strlen((const char*)raw), eci != 0 ? eci : DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments[] = {segment};
    QrmErrorCorrectionLevel level = isMicro ? ELevelLow : ELevelHigh;
    QrmExtraEncodingInfo extra = isMicro ? QrmExtraCreate(XModeMicroQr) : QrmExtraCreateNone();
    QrmBoard board = QrmEncoderEncode(segments, 1, level, extra, 0, 0xFF);
    QrmSegDestroy(&segment);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        std::cout << "ERROR: " << text.toStdString() << std::endl;
        QrmBoardDestroy(&board);
        return result;
    }
    std::cout << "QR for: " << text.toStdString() << std::endl;
    QrmBoardPrintDescription(board, false);
    QPixmap* image = imageFromBoard(board, 10, isMicro);
    result[text] = image;
    QrmBoardDestroy(&board);
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeMixModes() {
    QMap<QString, QPixmap*> result;
    QrmSegment segment1 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"ABC123 ", 7, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment2 = QrmSegCreate(EModeByte, (const UnsignedByte*)"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21", 11, 30);
    QrmSegment segments[] = {segment1, segment2};
    QrmErrorCorrectionLevel level = ELevelHigh;
    QrmExtraEncodingInfo extra = QrmExtraCreateNone();
    QrmBoard board = QrmEncoderEncode(segments, 1, level, extra, 0, 0xFF);
    QrmSegDestroy(&segment1);
    QrmSegDestroy(&segment2);
    QrmExtraDestroy(&extra);

    if (board.dimension == 0) {
        std::cout << "ERROR: Mixed: ABC123 안녕하세요!" << std::endl;
        QrmBoardDestroy(&board);
        return result;
    }
    std::cout << "QR Mixed: ABC123 안녕하세요!" << std::endl;
    QrmBoardPrintDescription(board, false);
    QPixmap* image = imageFromBoard(board, 10, false);
    result["Mixed: ABC123 안녕하세요!"] = image;
    QrmBoardDestroy(&board);
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeAutoMixModes() {
    QMap<QString, QPixmap*> result;
    const char* raw = "123456789 こんにちは世界！A56B 안녕하세요!";
    Utf8String text = U8Create((const UnsignedByte*)raw, strlen(raw));
    UnicodePoint unicodes = U8ToUnicodes(text);
    unsigned int segmentCount = 0;
    QrmErrorCorrectionLevel ecLevel = ELevelHigh;
    QrmSegment* segments = UPMakeSegments(unicodes, ecLevel, &segmentCount, false);
    printf("Mixed Auto: %d\n", segmentCount);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        printf("%d: %02X", index, segments[index].mode);
    }
    printf("\n");
    QrmExtraEncodingInfo extra = QrmExtraCreateNone();
    QrmBoard board = QrmEncoderEncode(segments, segmentCount, ecLevel, extra, 0, DEFAULT_ECI_ASSIGMENT);

    U8Destroy(&text);
    UPDestroy(&unicodes);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    DEALLOC(segments);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        std::cout << "ERROR: Mixed Auto" << std::endl;
        QrmBoardDestroy(&board);
        return result;
    }
    QrmBoardPrintDescription(board, false);
    QPixmap* image = imageFromBoard(board, 10, false);
    result["Mixed Auto: 123456789 こんにちは世界！A56B 안녕하세요!"] = image;
    QrmBoardDestroy(&board);
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeMicroMixModes() {
    QMap<QString, QPixmap*> result;
    const char* raw = "1234こんにちは";
    Utf8String text = U8Create((const UnsignedByte*)raw, strlen(raw));
    UnicodePoint unicodes = U8ToUnicodes(text);
    unsigned int segmentCount = 0;
    QrmErrorCorrectionLevel ecLevel = ELevelLow;
    QrmSegment* segments = UPMakeSegments(unicodes, ecLevel, &segmentCount, true);
    printf("Micro Mixed Auto: %d\n", segmentCount);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        printf("%d: %02X", index, segments[index].mode);
    }
    printf("\n");
    QrmExtraEncodingInfo extra = QrmExtraCreate(XModeMicroQr);
    QrmBoard board = QrmEncoderEncode(segments, segmentCount, ecLevel, extra, 0, DEFAULT_ECI_ASSIGMENT);

    U8Destroy(&text);
    UPDestroy(&unicodes);
    for (unsigned int index = 0; index < segmentCount; index += 1) {
        QrmSegDestroy(&segments[index]);
    }
    DEALLOC(segments);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        std::cout << "ERROR: Micro Mixed Auto" << std::endl;
        QrmBoardDestroy(&board);
        return result;
    }
    QrmBoardPrintDescription(board, false);
    QPixmap* image = imageFromBoard(board, 10, true);
    result["Micro Mixed: 1234こんにちは"] = image;
    QrmBoardDestroy(&board);
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeFNC1First() {
    QMap<QString, QPixmap*> result;
    QrmSegment segment1 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"01049123451234591597033130128", 29, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment2 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"%10ABC123", 9, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments[] = {segment1, segment2};
    QrmExtraEncodingInfo extra = QrmExtraCreate(XModeFnc1First);
    QrmBoard board = QrmEncoderEncode(segments, 2, ELevelHigh, extra, 0, 0xFF);
    printf("FNC1 1st:\n");
    QrmSegDestroy(&segment1);
    QrmSegDestroy(&segment2);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        std::cout << "ERROR: FNC1 First" << std::endl;
        QrmBoardDestroy(&board);
        return result;
    }
    QrmBoardPrintDescription(board, false);
    QPixmap* image = imageFromBoard(board, 10, false);
    result["FNC1 1st"] = image;
    QrmBoardDestroy(&board);
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeFNC1Second() {
    QMap<QString, QPixmap*> result;
    QrmSegment segment1 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"AA1234BBB112", 12, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment2 = QrmSegCreate(EModeByte, (const UnsignedByte*)"text text text text\n", 20, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments[] = {segment1, segment2};
    QrmExtraEncodingInfo extra = QrmExtraCreateFnc1Second((UnsignedByte*)"37", 2);
    QrmBoard board = QrmEncoderEncode(segments, 2, ELevelHigh, extra, 0, 0xFF);
    printf("FNC1 2nd:\n");
    QrmSegDestroy(&segment1);
    QrmSegDestroy(&segment2);
    QrmExtraDestroy(&extra);
    if (board.dimension == 0) {
        std::cout << "ERROR: FNC1 Second" << std::endl;
        QrmBoardDestroy(&board);
        return result;
    }
    QrmBoardPrintDescription(board, false);
    QPixmap* image = imageFromBoard(board, 10, false);
    result["FNC1 2nd"] = image;
    QrmBoardDestroy(&board);
    return result;
}

QMap<QString, QPixmap*> MainWindow_encodeStructuredAppend() {
    QMap<QString, QPixmap*> result;
    QrmErrorCorrectionLevel ecLevel = ELevelLow;

    QrmSegment segment11 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"123", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment12 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"ABC", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments1[] = {segment11, segment12};
    QrmStructuredAppend part1 = QrmStrAppCreate(segments1, 2, ecLevel);

    QrmSegment segment21 = QrmSegCreate(EModeNumeric, (const UnsignedByte*)"345", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segment22 = QrmSegCreate(EModeAlphaNumeric, (const UnsignedByte*)"DEF", 3, DEFAULT_ECI_ASSIGMENT);
    QrmSegment segments2[] = {segment21, segment22};
    QrmStructuredAppend part2 = QrmStrAppCreate(segments2, 2, ecLevel);

    QrmStructuredAppend allParts[] = {part1, part2};
    QrmBoard* boards = QrmEncoderMakeStructuredAppend(allParts, 2);

    QrmSegDestroy(&segment11);
    QrmSegDestroy(&segment12);
    QrmSegDestroy(&segment21);
    QrmSegDestroy(&segment22);
    QrmStrAppDestroy(&part1);
    QrmStrAppDestroy(&part2);

    if (boards == NULL) {
        std::cout << "ERROR: Structured append" << std::endl;
        return result;
    }

    if (boards[0].dimension > 0) {
        QrmBoardPrintDescription(boards[0], false);
        QPixmap* image1 = imageFromBoard(boards[0], 10, false);
        result["Structured Append 1"] = image1;
    } else {
        std::cout << "ERROR: Structured append 1" << std::endl;
    }
    QrmBoardDestroy(&boards[0]);

    if (boards[1].dimension > 0) {
        QrmBoardPrintDescription(boards[1], false);
        QPixmap* image2 = imageFromBoard(boards[1], 10, false);
        result["Structured Append 2"] = image2;
    } else {
        std::cout << "ERROR: Structured append 2" << std::endl;
    }
    QrmBoardDestroy(&boards[1]);

    DEALLOC(boards);
    return result;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->previousButton, &QPushButton::clicked, this, &MainWindow::buttonPreviousOnClick);
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::buttonNextOnClick);
    scenes = new QMap<QString, QGraphicsScene*>();
    texts = new QList<QString>();
    currentIndex = 0;

    QFuture<QMap<QString, QPixmap*>> operation;
    // Numeric mode
    const UnsignedByte* raw = (const UnsignedByte* )"12345678901234567";
    operation = QtConcurrent::run(MainWindow_encode, "NUMERIC: 12345678901234567", raw, EModeNumeric, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // AlphaNumeric mode
    raw = (const UnsignedByte* )"ABC$ 67890";
    operation = QtConcurrent::run(MainWindow_encode, "ALPHA NUMERIC: ABC$ 67890", raw, EModeAlphaNumeric, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Byte mode with standard encoding (Latin1)
    raw = (const UnsignedByte* )"L1! \xA9\xC2\xE2";
    operation = QtConcurrent::run(MainWindow_encode, "Latin-1: L1! ©Ââ", raw, EModeByte, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Kanji mode (with ShiftJIS encoding)
    raw = (const UnsignedByte* )"\x82\x4f\x82\x60\x82\xa0\x83\x41"; // \x88\x9f 亜
    operation = QtConcurrent::run(MainWindow_encode, "ShiftJIS: ０Ａあア", raw, EModeKanji, 0, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Byte mode using UTF-8 encoding directly
    raw = (const UnsignedByte* )"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    operation = QtConcurrent::run(
        MainWindow_encode,
        "UTF-8 no ECI: Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！",
        raw, EModeByte, 0, false
    );
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // ECI mode with UTF-8 encoding
    raw = (const UnsignedByte* )"Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！";
    operation = QtConcurrent::run(
        MainWindow_encode,
        "UTF-8 with ECI: Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！",
        raw, EModeByte, 26, false
    );
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // ECI mode with custom encoding (EUC-KR)
    raw = (const UnsignedByte* )"\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4\x21";
    operation = QtConcurrent::run(MainWindow_encode, "Custom ECI (KS X 1001): 안녕하세요!", raw, EModeByte, 30, false);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Mixed modes
    operation = QtConcurrent::run(MainWindow_encodeMixModes);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Mixed modes auto
    operation = QtConcurrent::run(MainWindow_encodeAutoMixModes);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });

    // Micro QR
    // Numeric
    raw = (const UnsignedByte* )"12345";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO NUMERIC: 12345", raw, EModeNumeric, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // AlphaNumeric
    raw = (const UnsignedByte* )"A12345";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO ALPHA NUMERIC: A12345", raw, EModeAlphaNumeric, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Byte
    raw = (const UnsignedByte* )"안녕";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO BYTE: 안녕", raw, EModeByte, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Kanji
    raw = (const UnsignedByte* )"\x82\x60\x82\xa0\x83\x41\x82\x4f";
    operation = QtConcurrent::run(MainWindow_encode, "MICRO KANJI: Ａあア０", raw, EModeKanji, 0, true);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Mix
    operation = QtConcurrent::run(MainWindow_encodeMicroMixModes);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });

    // FNC1 Fist Position
    operation = QtConcurrent::run(MainWindow_encodeFNC1First);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // FNC1 Second Position
    operation = QtConcurrent::run(MainWindow_encodeFNC1Second);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
    // Structured append
    operation = QtConcurrent::run(MainWindow_encodeStructuredAppend);
    operation.then(this, [this](QMap<QString, QPixmap*> result) { finalizeOps(result); });
}

MainWindow::~MainWindow() {
    delete ui;
    qDeleteAll(*scenes);
    delete scenes;
    delete texts;
}

void MainWindow::buttonPreviousOnClick() {
    if (currentIndex == 0) {
        if (scenes->size() > 0) {
            currentIndex = scenes->size() - 1;
            configureCurrentIndex();
        }
    } else {
        currentIndex -= 1;
        configureCurrentIndex();
    }
}

void MainWindow::buttonNextOnClick() {
    if (currentIndex + 1 < scenes->size()) {
        currentIndex += 1;
        configureCurrentIndex();
    } else {
        currentIndex = 0;
        configureCurrentIndex();
    }
}

void MainWindow::finalizeOps(QMap<QString, QPixmap*> result) {
    unsigned int before = texts->size();
    if (result.size() > 0) {
        foreach (QString text, result.keys()) {
            QPixmap* image = result[text];
            QGraphicsScene* scene = new QGraphicsScene(this);
            scene->addPixmap(*image);
            scene->setSceneRect(image->rect());
            texts->append(text);
            scenes->insert(text, scene);
            delete image;
        }
    }
    if (before == 0) {
        configureCurrentIndex();
    }
}

void MainWindow::configureCurrentIndex() {
    if (texts->size() == 0) { return; }
    QString qtext = texts->at(currentIndex);
    QGraphicsScene* scene = scenes->value(qtext);
    ui->imageView->setScene(scene);
    ui->statusbar->showMessage(qtext);
}
