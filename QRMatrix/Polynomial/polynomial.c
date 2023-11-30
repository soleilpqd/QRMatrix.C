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

#include "polynomial.h"
#include <stdlib.h>

UnsignedByte Polynomial_Exp[256];
UnsignedByte Polynomial_Log[256];

void QrmPolynomialInitialize() {
    static const unsigned int prim = 0x11D;
    unsigned int xVal = 1;
    for (unsigned int index = 0; index < 255; index += 1) {
        Polynomial_Exp[index] = (UnsignedByte)xVal;
        Polynomial_Log[xVal] = (UnsignedByte)index;
        xVal <<= 1;
        if (xVal >= 256) {
            xVal ^= prim;
        }
    }
}

UnsignedByte Polynomial_Multiple(UnsignedByte left, UnsignedByte right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    UnsignedByte result = Polynomial_Exp[(Polynomial_Log[left] + Polynomial_Log[right]) % 255];
    return result;
}

UnsignedByte Polynomial_Power(UnsignedByte value, UnsignedByte power) {
    return Polynomial_Exp[(Polynomial_Log[value] * power) % 255];
}

QrmPolynomial Polynomial_PolyMultiple(QrmPolynomial self, QrmPolynomial other) {
    QrmPolynomial result = QrmPolynomialCreate(self.length + other.length - 1);
    for (unsigned int jndex = 0; jndex < other.length; jndex += 1) {
        for (unsigned int index = 0; index < self.length; index += 1) {
            result.terms[index + jndex] ^= Polynomial_Multiple(self.terms[index], other.terms[jndex]);
        }
    }
    return result;
}

QrmPolynomial Polynomial_getGeneratorPoly(unsigned int count) {
    QrmPolynomial result = QrmPolynomialCreate(1);
    result.terms[0] = 1;
    for (unsigned int index = 0; index < count; index += 1) {
        QrmPolynomial arg = QrmPolynomialCreate(2);
        arg.terms[0] = 1;
        arg.terms[1] = Polynomial_Power(2, index);
        QrmPolynomial temp = Polynomial_PolyMultiple(result, arg);
        QrmPolynomialDestroy(&arg);
        QrmPolynomialDestroy(&result);
        result = temp;

    }
    return result;
}

QrmPolynomial QrmGetErrorCorrections(unsigned int count, QrmPolynomial data) {
    if (data.length == 0 || data.terms == NULL || count == 0 || data.length + count > 255) {
        LOG("ERROR: Internal error: invalid message length to calculate Error Corrections");
        return QrmPolynomialCreate(0);
    }
    QrmPolynomial gen = Polynomial_getGeneratorPoly(count);
    QrmPolynomial buffer = QrmPolynomialCreate(data.length + gen.length - 1);
    for (unsigned int index = 0; index < data.length; index += 1) {
        buffer.terms[index] = data.terms[index];
    }
    for (unsigned int index = 0; index < data.length; index += 1) {
        UnsignedByte coef = buffer.terms[index];
        if (coef != 0) {
            for (unsigned int jndex = 1; jndex < gen.length; jndex += 1) {
                buffer.terms[index + jndex] ^= Polynomial_Multiple(gen.terms[jndex], coef);
            }
        }
    }
    QrmPolynomial result = QrmPolynomialCreate(buffer.length - data.length);
    for (unsigned int index = data.length; index < buffer.length; index += 1) {
        result.terms[index - data.length] = buffer.terms[index];
    }
    QrmPolynomialDestroy(&gen);
    QrmPolynomialDestroy(&buffer);
    return result;
}

// --------------------------------------------------------------------------------------------

void QrmPolynomialDestroy(QrmPolynomial* data) {
    if (data->length > 0 && data->terms != NULL) {
        DEALLOC(data->terms);
    }
}

QrmPolynomial QrmPolynomialCreate(unsigned int count) {
    QrmPolynomial result;
    result.length = count;
    if (count > 0) {
        ALLOC_(UnsignedByte, result.terms, count);
    } else {
        result.terms = NULL;
    }
    return result;
}
