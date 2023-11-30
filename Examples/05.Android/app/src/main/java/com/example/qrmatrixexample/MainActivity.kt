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

package com.example.qrmatrixexample

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.example.qrmatrixexample.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlin.math.sqrt

enum class QRExampleMode(val raw: Int) {
    mNumeric(1),
    mAlphaNumeric(2),
    mByte(3),
    mKanji(4)
}

enum class QRExampleECLevel(val raw: Int) {
    lLow(1),
    lMedium(2),
    lQuarter(3),
    lHigh(4)
}

enum class BoardCell(val raw: UByte) {
    cUnset(0x05u),
    cSet(0x0Au),
    cLowMask(0x0Fu)
}

enum class QRExampleExtraMode(val raw: Int) {
    mNone(1),
    mMicroQR(2),
    mFNC1First(3),
    mFNC1Second(4)
}

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val bitmaps = emptyMap<String, Bitmap>().toMutableMap()
    private val texts = emptyList<String>().toMutableList()
    private var currentIndex = 0
    private val defaultEci = 3
    private var structuredAppendId = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        initQR();
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.buttonPrevious.setOnClickListener {
            this.buttonPreviousOnClick()
        }
        binding.buttonNext.setOnClickListener {
            this.buttonNextOnClick()
        }
        launchExamples()
    }

    private fun launchExamples() {
        GlobalScope.launch {
            // Numeric mode
            doExample("12345678901234567", QRExampleMode.mNumeric, defaultEci, false)
            // AlphaNumeric mode
            doExample("ABC\$ 67890", QRExampleMode.mAlphaNumeric, defaultEci, false)
            // Byte (Latin1) mode
            doExample("L1! ©Ââ", QRExampleMode.mByte, defaultEci, false)
            // Kanji mode (String.toByteArray() does not support ShiftJIS, so we have to pass UTF-8 bytes and convert to ShiftJIS in CPP module)
            doExample("０Ａあア", QRExampleMode.mKanji, defaultEci, false)
            // Byte mode with UTF-8
            doExample("Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！", QRExampleMode.mByte, defaultEci, false)
            // ECI mode with sub-mode Byte & UTF8 ECI Indicator (26)
            doExample("ECI Hello world!\nXin chào thế giới!\nこんにちは世界！\n안녕하세요!\n你好世界！", QRExampleMode.mByte, 26, false)
            // ECI mode with non UTF8 encoding (EUC-KR/KS X 1001 30)
            launch {
                // "안녕하세요!" EUC-KR sequence
                val bytes = byteArrayOf(
                    0xBE.toByte(), 0xC8.toByte(), 0xB3.toByte(), 0xE7.toByte(), 0xC7.toByte(),
                    0xCF.toByte(), 0xBC.toByte(), 0xBC.toByte(), 0xBF.toByte(), 0xE4.toByte(), 0x21.toByte()
                )
                val datas = arrayOf(bytes)
                val modes = IntArray(1)
                modes[0] = QRExampleMode.mByte.raw
                val converts = BooleanArray(1)
                converts[0] = false
                val ecis = IntArray(1)
                ecis[0] = 30 // 30 is ECI Indicator for KS X 1001 (which includes EUC-KR)
                val result = makeQR(datas, modes, converts, ecis, QRExampleECLevel.lHigh.raw, QRExampleExtraMode.mNone.raw, "")
                if (result[0] != 0.toByte()) {
                    val mesg = String(result.drop(1).toByteArray())
                    Log.e("QR RESULT", "Custom ECI")
                    Log.e("QR RESULT", mesg)
                } else {
                    drawQRBoard("Custom ECI: 안녕하세요!", result, false)
                }
            }
            // Mixed modes
            launch {
                val bytes1 = "ABC123 ".toByteArray(Charsets.US_ASCII)
                // "안녕하세요!" EUC-KR sequence
                val bytes2 = byteArrayOf(
                    0xBE.toByte(), 0xC8.toByte(), 0xB3.toByte(), 0xE7.toByte(), 0xC7.toByte(),
                    0xCF.toByte(), 0xBC.toByte(), 0xBC.toByte(), 0xBF.toByte(), 0xE4.toByte(), 0x21.toByte()
                )
                val datas = arrayOf(bytes1, bytes2)
                val modes = IntArray(2)
                modes[0] = QRExampleMode.mAlphaNumeric.raw
                modes[1] = QRExampleMode.mByte.raw
                val converts = BooleanArray(2)
                converts[0] = false
                converts[1] = false
                val ecis = IntArray(2)
                ecis[0] = defaultEci
                ecis[1] = 30 // 30 is ECI Indicator for KS X 1001 (which includes EUC-KR)
                val result = makeQR(datas, modes, converts, ecis, QRExampleECLevel.lHigh.raw, QRExampleExtraMode.mNone.raw, "")
                if (result[0] != 0.toByte()) {
                    val mesg = String(result.drop(1).toByteArray())
                    Log.e("QR RESULT", "Mixed")
                    Log.e("QR RESULT", mesg)
                } else {
                    drawQRBoard("Mixed: ABC123 안녕하세요!", result, false)
                }
            }
            // Mixed mode auto
            launch {
                val text = "123456789 こんにちは世界！A56B 안녕하세요!"
                val result = makeQRAuto(text, QRExampleECLevel.lHigh.raw,  QRExampleExtraMode.mNone.raw, "")
                if (result[0] != 0.toByte()) {
                    val mesg = String(result.drop(1).toByteArray())
                    Log.e("QR RESULT", "Mixed")
                    Log.e("QR RESULT", mesg)
                } else {
                    drawQRBoard("Mixed auto: 123456789 こんにちは世界！A56B 안녕하세요!", result, false)
                }
            }

            // Micro
            // Numeric mode
            doExample("12345", QRExampleMode.mNumeric, defaultEci, true)
            // AlphaNumeric mode
            doExample("A12345", QRExampleMode.mAlphaNumeric, defaultEci, true)
            // Byte
            doExample("안녕", QRExampleMode.mByte, defaultEci, true)
            // Kanji mode (String.toByteArray() does not support ShiftJIS, so we have to pass UTF-8 bytes and convert to ShiftJIS in CPP module)
            doExample("Ａあア０", QRExampleMode.mKanji, defaultEci, true)
            // Micro Mixed mode auto
            launch {
                val text = "1234こんにちは"
                val result = makeQRAuto(text, QRExampleECLevel.lLow.raw,  QRExampleExtraMode.mMicroQR.raw, "")
                if (result[0] != 0.toByte()) {
                    val mesg = String(result.drop(1).toByteArray())
                    Log.e("QR RESULT", "Mixed")
                    Log.e("QR RESULT", mesg)
                } else {
                    drawQRBoard("Micro mixed: 1234こんにちは", result, true)
                }
            }
            // FNC1 First
            launch {
                val bytes1 = "01049123451234591597033130128".toByteArray(Charsets.US_ASCII)
                val bytes2 = "%10ABC123".toByteArray(Charsets.US_ASCII)
                val datas = arrayOf(bytes1, bytes2)
                val modes = IntArray(2)
                modes[0] = QRExampleMode.mNumeric.raw
                modes[1] = QRExampleMode.mAlphaNumeric.raw
                val converts = BooleanArray(2)
                converts[0] = false
                converts[1] = false
                val ecis = IntArray(2)
                ecis[0] = defaultEci
                ecis[1] = defaultEci
                val result = makeQR(datas, modes, converts, ecis, QRExampleECLevel.lHigh.raw, QRExampleExtraMode.mFNC1First.raw, "")
                if (result[0] != 0.toByte()) {
                    val mesg = String(result.drop(1).toByteArray())
                    Log.e("QR RESULT", "FNC1 1st")
                    Log.e("QR RESULT", mesg)
                } else {
                    drawQRBoard("FNC1 First Position", result, false)
                }
            }
            // FNC1 Second
            launch {
                val bytes1 = "AA1234BBB112".toByteArray(Charsets.US_ASCII)
                val bytes2 = "text text text text\n".toByteArray(Charsets.US_ASCII)
                val datas = arrayOf(bytes1, bytes2)
                val modes = IntArray(2)
                modes[0] = QRExampleMode.mAlphaNumeric.raw
                modes[1] = QRExampleMode.mByte.raw
                val converts = BooleanArray(2)
                converts[0] = false
                converts[1] = false
                val ecis = IntArray(2)
                ecis[0] = defaultEci
                ecis[1] = defaultEci
                val result = makeQR(datas, modes, converts, ecis, QRExampleECLevel.lHigh.raw, QRExampleExtraMode.mFNC1Second.raw, "37")
                if (result[0] != 0.toByte()) {
                    val mesg = String(result.drop(1).toByteArray())
                    Log.e("QR RESULT", "FNC1 2nd")
                    Log.e("QR RESULT", mesg)
                } else {
                    drawQRBoard("FNC1 Second Position", result, false)
                }
            }
            // Structured append
            launch {
                val job = startMakeStructuredAppendQR();  // This func should not used parallely in asynch environment like this because it accesses a static memory location.
                                                          // In this example it's fine because I use once.
                Log.i("STRUCTURED APPEND", "START $job")
                val bytes11 = "123".toByteArray(Charsets.US_ASCII);
                val bytes12 = "ABC".toByteArray(Charsets.US_ASCII);
                val datas1 = arrayOf(bytes11, bytes12);
                val bytes21 = "456".toByteArray(Charsets.US_ASCII);
                val bytes22 = "DEF".toByteArray(Charsets.US_ASCII);
                val datas2 = arrayOf(bytes21, bytes22);
                val modes = IntArray(2)
                modes[0] = QRExampleMode.mNumeric.raw
                modes[1] = QRExampleMode.mAlphaNumeric.raw
                val converts = BooleanArray(2)
                converts[0] = false
                converts[1] = false
                val ecis = IntArray(2)
                ecis[0] = defaultEci
                ecis[1] = defaultEci
                var res = addStructuredAppendQR(job, datas1, modes, converts, ecis, QRExampleECLevel.lHigh.raw, QRExampleExtraMode.mNone.raw, "")
                if (res.size > 1) {
                    val mesg = String(res.drop(1).toByteArray())
                    Log.e("QR RESULT", "Structured Append part 1")
                    Log.e("QR RESULT", mesg)
                }
                res = addStructuredAppendQR(job, datas2, modes, converts, ecis, QRExampleECLevel.lHigh.raw, QRExampleExtraMode.mNone.raw, "")
                if (res.size > 1) {
                    val mesg = String(res.drop(1).toByteArray())
                    Log.e("QR RESULT", "Structured Append part 2")
                    Log.e("QR RESULT", mesg)
                }
                res = commitStructuredAppendQR(job)
                if (res.size > 1) {
                    val mesg = String(res.drop(1).toByteArray())
                    Log.e("QR RESULT", "Structured Append commit")
                    Log.e("QR RESULT", mesg)
                } else {
                    for (index in 0..1) {
                        val result = getStructuredAppedQRBoard(job, index);
                        if (result[0] != 0.toByte()) {
                            val mesg = String(result.drop(1).toByteArray())
                            Log.e("QR RESULT", "Structured Append res $index")
                            Log.e("QR RESULT", mesg)
                        } else {
                            drawQRBoard("Structured append $index", result, false)
                        }
                    }
                }
                clearStructuredAppendQR(job);
            }
        }
    }

    private suspend fun doExample(text: String, mode: QRExampleMode, eci: Int, isMicro: Boolean) = coroutineScope {
        launch {
            val bytes = text.toByteArray(Charsets.UTF_8)
            val datas = arrayOf(bytes)
            val modes = IntArray(1)
            modes[0] = mode.raw
            val converts = BooleanArray(1)
            converts[0] = mode == QRExampleMode.mKanji
            val ecis = IntArray(1)
            ecis[0] = eci
            val result = makeQR(datas, modes, converts, ecis,
                if (isMicro) QRExampleECLevel.lLow.raw else QRExampleECLevel.lHigh.raw,
                if (isMicro) QRExampleExtraMode.mMicroQR.raw else QRExampleExtraMode.mNone.raw,
                ""
            )
            if (result[0] != 0.toByte()) {
                val mesg = String(result.drop(1).toByteArray())
                Log.e("QR RESULT", text)
                Log.e("QR RESULT", mesg)
            } else {
                drawQRBoard(text, result, isMicro)
            }
        }
    }

    private fun drawQRBoard(text: String, board: ByteArray, isMicro: Boolean) {
        val scale = 10
        val quietZone = if (isMicro) 2 else 4
        val cellSide = sqrt((board.size - 1).toFloat()).toInt()
        val dimension = (cellSide.toFloat() + 2.0f * quietZone) * scale.toFloat()
        val dimen = dimension.toInt()
        val image = Bitmap.createBitmap(dimen, dimen, Bitmap.Config.ARGB_8888, false)
        val canvas = Canvas(image)
        // Background
        val whitePaint = Paint()
        whitePaint.style = Paint.Style.FILL
        whitePaint.color = Color.WHITE
        canvas.drawRect(Rect(0, 0, dimen, dimen), whitePaint)
        // Cells
        val blackPaint = Paint()
        whitePaint.style = Paint.Style.FILL
        whitePaint.color = Color.BLACK

        for (row in 0..cellSide - 1) {
            for (column in 0..cellSide - 1) {
                val offet = 1 + row * cellSide + column // +1 because the first byte in Board array is the result status
                val cell = board[offet].toUByte()
                val low = cell.and(BoardCell.cLowMask.raw)
                if (low == BoardCell.cSet.raw) {
                    val x = (quietZone + column) * scale
                    val y = (quietZone + row) * scale
                    val rect = Rect(x, y, x + scale, y + scale)
                    canvas.drawRect(rect, blackPaint)
                }
            }
        }
        Dispatchers.Main.run {
            texts.add(text)
            bitmaps.put(text, image)
            if (currentIndex == texts.size - 1) {
                configCurrentIndex()
            }
        }
    }

    private fun configCurrentIndex() {
        if (texts.size == 0) { return }
        val text = texts[currentIndex]
        val bitmap = bitmaps[text]
        this.title = text
        binding.imageView.setImageBitmap(bitmap)
    }

    private fun buttonPreviousOnClick() {
        if (currentIndex == 0) {
            if (texts.size > 0) {
                currentIndex = texts.size - 1
                configCurrentIndex()
            }
        } else {
            currentIndex -= 1
            configCurrentIndex()
        }
    }

    private fun buttonNextOnClick() {
        if (currentIndex + 1 < texts.size) {
            currentIndex += 1
            configCurrentIndex()
        } else {
            currentIndex = 0
            configCurrentIndex()
        }
    }

    external fun initQR();

    /**
     * A native method that is implemented by the 'qrmatrixexample' native library,
     * which is packaged with this application.
     *
     * All input arrays must have same size
     *
     * @return A byte array:
     * - 1st item (array[0]) is result status. `0` means success.
     * - Success: following bytes are QR cells (modules).
     * - Failure: following bytes are error message (C String).
     */
    external fun makeQR(
        /// Array of datas to be encoded
        datas: Array<ByteArray>,
        /// Mode for each item in data array
        modes: IntArray,
        /// Need to convert input data into suitable bytes (so data in array must be UTF8)
        /// (this example only converts UTF8 to ShiftJIS in Kanji mode)
        needConverts: BooleanArray,
        /// Custom ECI Indicator (0 to not use ECI)
        ecis: IntArray,
        /// Error correction level
        level: Int,
        /// Extra mode
        extraMode: Int,
        /// FNC1 Second App ID
        fnc1SecondAppId: String
    ): ByteArray

    /**
     * A native method that is implemented by the 'qrmatrixexample' native library,
     * which is packaged with this application.
     *
     * @return A byte array:
     * - 1st item (array[0]) is result status. `0` means success.
     * - Success: following bytes are QR cells (modules).
     * - Failure: following bytes are error message (C String).
     */
    external fun makeQRAuto(
        /// Array of datas to be encoded
        text: String,
        /// Error correction level
        level: Int,
        /// Extra mode
        extraMode: Int,
        /// FNC1 Second App ID
        fnc1SecondAppId: String
    ): ByteArray

    external fun startMakeStructuredAppendQR(): Int
    external fun addStructuredAppendQR(
        id: Int,
        /// Array of datas to be encoded
        datas: Array<ByteArray>,
        /// Mode for each item in data array
        modes: IntArray,
        /// Need to convert input data into suitable bytes (so data in array must be UTF8)
        /// (this example only converts UTF8 to ShiftJIS in Kanji mode)
        needConverts: BooleanArray,
        /// Custom ECI Indicator (0 to not use ECI)
        ecis: IntArray,
        /// Error correction level
        level: Int,
        /// Extra mode
        extraMode: Int,
        /// FNC1 Second App ID
        fnc1SecondAppId: String
    ): ByteArray
    external fun commitStructuredAppendQR(id: Int): ByteArray
    external fun getStructuredAppedQRBoard(id: Int, index: Int): ByteArray
    external fun clearStructuredAppendQR(id: Int)

    companion object {
        // Used to load the 'qrmatrixexample' library on application startup.
        init {
            System.loadLibrary("qrmatrixexample")
        }
    }
}