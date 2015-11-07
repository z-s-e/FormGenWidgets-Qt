/* Copyright 2014, 2015 Zeno Sebastian Endemann <zeno.endemann@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <QVariant>

namespace MathUtils {

    enum RoundingMode {
        RoundNearestEven,
        RoundToInf,
        RoundToMinusInf
    };
    enum ConversionResult {
        NoError,
        RangeError,
        StringFormatError
    };
    enum NotationFormat {
        SimpleNotation,
        ScientificNotation,
        AutoNotation
    };

    bool isIntegerType(const QVariant &value);

    ConversionResult decimalToFloatB64(const QString & src,
                                       RoundingMode round,
                                       double * dst);
    ConversionResult decimalToFloatB32(const QString & src,
                                       RoundingMode round,
                                       float * dst);
    ConversionResult floatB64ToFloatB32(double src,
                                        RoundingMode round,
                                        float * dst);

    QString floatB32ToString_RoundTripPrecision(float f, NotationFormat format = AutoNotation);
    QString floatB64ToString_RoundTripPrecision(double d, NotationFormat format = AutoNotation);

    QVariant intDecimalToQVariantInteger(const QString & src);

    bool isSmallerZero(const QVariant & integerVariant);
    bool minLeqValLeqMax(QVariant min, QVariant val, QVariant max);
}

#endif // MATHUTILS_H
