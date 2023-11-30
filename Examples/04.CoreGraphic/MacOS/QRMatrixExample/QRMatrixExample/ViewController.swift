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

import Cocoa
import QRMatrix

class ViewController: NSViewController {

    private let queue = OperationQueue()
    private var images = [String: NSImage]()
    private var textIndices = [String]()
    private var currentIndex = 0;

    @IBOutlet private weak var scrollView: NSScrollView!
    @IBOutlet private weak var imageView: NSImageView!
    @IBOutlet private weak var imageHeightLayout: NSLayoutConstraint!
    @IBOutlet private weak var imageWidthLayout: NSLayoutConstraint!
    @IBOutlet private weak var contentLabel: NSTextField!

    func genQR(text: String, mode: QRMode, isLatin1: Bool, eci: UInt, isMicro: Bool) {
        let encoding: String.Encoding
        switch mode {
        case .mAlphaNumeric, .mNumeric:
            encoding = .ascii
        case .mKanji:
            encoding = .shiftJIS
        case .mByte:
            if isLatin1 {
                encoding = .isoLatin1
            } else {
                encoding = .utf8
            }
        @unknown default:
            encoding = .utf8
        }
        guard let data = text.data(using: encoding) else { return }
        let segment = QRSegment(data, mode: mode, eci: eci)
        do {
            let image = try QRMatrixLib.makeQR([segment], errorCorrectionLevel: isMicro ? .ecLow : .ecHigh, scale: 10, extra: isMicro ? QRExtraData(.mMicroQR) : nil);
            finalizeOp(text: text, image: image)
        } catch let err {
            print("ERROR:", text, err.localizedDescription)
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        // Numeric mode
        queue.addOperation {[weak self] in
            self?.genQR(text: "12345678901234567", mode: .mNumeric, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: false)
        }
        // AlphaNumeric mode
        queue.addOperation {[weak self] in
            self?.genQR(text: "ABC$ 67890", mode: .mAlphaNumeric, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: false)
        }
        // Byte mode using standard encoding (Latin1)
        queue.addOperation {[weak self] in
            self?.genQR(text: "L1! ©Ââ", mode: .mByte, isLatin1: true, eci: UInt(DEFAULT_ECI), isMicro: false)
        }
        // Kanji mode (using ShiftJIS)
        queue.addOperation {[weak self] in
            self?.genQR(text: "０Ａあア", mode: .mKanji, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: false)
        }
        // Byte mode using UTF-8 directly
        queue.addOperation {[weak self] in
            self?.genQR(text: "Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！", mode: .mByte, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: false)
        }
        // ECI mode with UTF-8 encoding
        queue.addOperation {[weak self] in
            self?.genQR(text: "ECI Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！", mode: .mByte, isLatin1: false, eci: 26, isMicro: false)
        }
        // ECI mode with non UTF-8 encoding
        queue.addOperation {[weak self] in
            let data = Data([0xBE, 0xC8, 0xB3, 0xE7, 0xC7, 0xCF, 0xBC, 0xBC, 0xBF, 0xE4, 0x21]); // "안녕하세요!" EUC-KR sequence
            let segment = QRSegment(data, mode: .mByte, eci: 30)
            do {
                let image = try QRMatrixLib.makeQR([segment], errorCorrectionLevel: .ecHigh, scale: 10, extra: nil);
                self?.finalizeOp(text: "안녕하세요!", image: image)
            } catch let err {
                print("ERROR:", "안녕하세요!", err.localizedDescription)
            }
        }
        // Mixed
        queue.addOperation {[weak self] in
            guard let data1 = "ABC123 ".data(using: .ascii) else { return }
            let segment1 = QRSegment(data1, mode: .mAlphaNumeric, eci: UInt(DEFAULT_ECI))
            let data2 = Data([0xBE, 0xC8, 0xB3, 0xE7, 0xC7, 0xCF, 0xBC, 0xBC, 0xBF, 0xE4, 0x21]); // "안녕하세요!" EUC-KR sequence
            let segment2 = QRSegment(data2, mode: .mByte, eci: 30)
            do {
                let image = try QRMatrixLib.makeQR([segment1, segment2], errorCorrectionLevel: .ecHigh, scale: 10, extra: nil);
                self?.finalizeOp(text: "Mixed: \"ABC123 안녕하세요!\"", image: image)
            } catch let err {
                print("ERROR:", "Mixed", err.localizedDescription)
            }
        }
        // Auto Mixed
        queue.addOperation {[weak self] in
            do {
                let image = try QRMatrixLib.makeMixedModesQR("123456789 こんにちは世界！A56B 안녕하세요!", errorCorrectionLevel: .ecHigh, scale: 10, extra: nil)
                self?.finalizeOp(text: "Mixed auto: \"123456789 こんにちは世界！A56B 안녕하세요!\"", image: image)
            } catch let err {
                print("ERROR:", "AutoMixed", err.localizedDescription)
            }
        }
        // MICRO QR
        // Numeric mode
        queue.addOperation {[weak self] in
            self?.genQR(text: "12345", mode: .mNumeric, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: true)
        }
        // AlphaNumeric mode
        queue.addOperation {[weak self] in
            self?.genQR(text: "A12345", mode: .mAlphaNumeric, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: true)
        }
        // Byte mode
        queue.addOperation {[weak self] in
            self?.genQR(text: "안녕", mode: .mNumeric, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: true)
        }
        // Kanji mode (with ShiftJIS encoding)
        queue.addOperation {[weak self] in
            self?.genQR(text: "Ａあア０", mode: .mKanji, isLatin1: false, eci: UInt(DEFAULT_ECI), isMicro: true)
        }
        // Auto Mixed
        queue.addOperation {[weak self] in
            do {
                let image = try QRMatrixLib.makeMixedModesQR("1234こんにちは", errorCorrectionLevel: .ecLow, scale: 10, extra: QRExtraData(.mMicroQR))
                self?.finalizeOp(text: "Micro Mixed: \"1234こんにちは\"", image: image)
            } catch let err {
                print("ERROR:", "MicroMixed", err.localizedDescription)
            }
        }
        // FNC1 First position
        queue.addOperation {[weak self] in
            guard let data1 = "01049123451234591597033130128".data(using: .ascii),
                  let data2 = "%10ABC123".data(using: .ascii)
            else { return }
            let segment1 = QRSegment(data1, mode: .mNumeric, eci: UInt(DEFAULT_ECI))
            let segment2 = QRSegment(data2, mode: .mAlphaNumeric, eci: UInt(DEFAULT_ECI))
            let extra = QRExtraData(.mFnc1First)
            do {
                let image = try QRMatrixLib.makeQR([segment1, segment2], errorCorrectionLevel: .ecHigh, scale: 10, extra: extra);
                self?.finalizeOp(text: "FNC1 1st", image: image)
            } catch let err {
                print("ERROR:", "FNC1 1st", err.localizedDescription)
            }
        }
        // FNC1 Second position
        queue.addOperation {[weak self] in
            guard let data1 = "AA1234BBB112".data(using: .ascii),
                  let data2 = "text text text text\n".data(using: .ascii)
            else { return }
            let segment1 = QRSegment(data1, mode: .mAlphaNumeric, eci: UInt(DEFAULT_ECI))
            let segment2 = QRSegment(data2, mode: .mByte, eci: UInt(DEFAULT_ECI))
            let extra = QRExtraData.fnc1Second("37")
            do {
                let image = try QRMatrixLib.makeQR([segment1, segment2], errorCorrectionLevel: .ecHigh, scale: 10, extra: extra);
                self?.finalizeOp(text: "FNC1 2nd", image: image)
            } catch let err {
                print("ERROR:", "FNC1 2nd", err.localizedDescription)
            }
        }
        /// Structred Append
        queue.addOperation {[weak self] in
            guard let data11 = "123".data(using: .ascii),
                  let data12 = "ABC".data(using: .ascii),
                  let data21 = "345".data(using: .ascii),
                  let data22 = "DEF".data(using: .ascii)
            else { return }
            let segment11 = QRSegment(data11, mode: .mNumeric, eci: UInt(DEFAULT_ECI))
            let segment12 = QRSegment(data12, mode: .mAlphaNumeric, eci: UInt(DEFAULT_ECI))
            let segment21 = QRSegment(data21, mode: .mNumeric, eci: UInt(DEFAULT_ECI))
            let segment22 = QRSegment(data22, mode: .mAlphaNumeric, eci: UInt(DEFAULT_ECI))
            let part1 = QRStructuredAppend([segment11, segment12], errorCorrectionLevel: .ecHigh, extra: nil)
            let part2 = QRStructuredAppend([segment21, segment22], errorCorrectionLevel: .ecHigh, extra: nil)
            do {
                let images = try QRMatrixLib.makeStructuredAppendQR([part1, part2], scale: 10);
                for (index, image) in images.enumerated() {
                    self?.finalizeOp(text: "Structured Append \(index)", image: image)
                }
            } catch let err {
                print("ERROR:", "Structured append", err.localizedDescription)
            }
        }
    }

    private func finalizeOp(text: String, image: NSImage?) {
        DispatchQueue.main.async {[weak self] in
            guard let mSelf = self else { return }
            mSelf.textIndices.append(text)
            if let img = image {
                mSelf.images[text] = img
            }
            if mSelf.currentIndex == mSelf.textIndices.count - 1 {
                mSelf.completeOps()
            }
        }
    }

    private func completeOps() {
        configureCurrentIndex()
    }

    private func configureCurrentIndex() {
        guard textIndices.count > 0 else { return }
        let text = textIndices[currentIndex];
        contentLabel.stringValue = text
        let image = images[text]
        imageView.image = image
        imageWidthLayout.constant = image?.size.width ?? 0
        imageHeightLayout.constant = image?.size.height ?? 0
        scrollView.magnification = 1.0
        scrollView.needsLayout = true
    }

    @IBAction private func previousButtonOnTap(_ sender: Any?) {
        if currentIndex == 0 {
            if (images.count > 0) {
                currentIndex = images.count - 1
                configureCurrentIndex()
            }
        } else {
            currentIndex -= 1
            configureCurrentIndex()
        }
    }

    @IBAction private func nextButtonOnTap(_ sender: Any?) {
        if currentIndex + 1 < images.count {
            currentIndex += 1
            configureCurrentIndex()
        } else {
            currentIndex = 0
            configureCurrentIndex()
        }
    }

}
