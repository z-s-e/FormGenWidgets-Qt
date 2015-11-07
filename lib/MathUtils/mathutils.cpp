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

#include "mathutils.h"

#include <QRegularExpression>

#include <fenv.h>
#include <limits>
#include <math.h>

static QRegularExpression s_decimalLiteral("\\A-?(0|[1-9][0-9]*)(\\.[0-9]+)?([eE][\\+-]?[0-9]+)?\\z");

bool MathUtils::isIntegerType(const QVariant &value)
{
    switch( static_cast<QMetaType::Type>(value.type()) ) {
    case QMetaType::SChar:
    case QMetaType::UChar:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Long:
    case QMetaType::ULong:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return true;
    default:
        return false;
    }
}

MathUtils::ConversionResult MathUtils::decimalToFloatB64(const QString & src,
                                                         RoundingMode round,
                                                         double * dst)
{
    if( ! s_decimalLiteral.match(src).hasMatch() )
        return StringFormatError;

    int tmp = fegetround();
    switch(round) {
    case RoundNearestEven:
        fesetround(FE_TONEAREST);
        break;
    case RoundToInf:
        fesetround(FE_UPWARD);
        break;
    case RoundToMinusInf:
        fesetround(FE_DOWNWARD);
        break;
    default:
        Q_ASSERT(false);
    }

    bool ok;
    *dst = src.toDouble(&ok);
    fesetround(tmp);

    if( ! ok )
        return RangeError;
    else
        return NoError;
}

MathUtils::ConversionResult MathUtils::decimalToFloatB32(const QString & src,
                                                         RoundingMode round,
                                                         float * dst)
{
    if( ! s_decimalLiteral.match(src).hasMatch() )
        return StringFormatError;

    int tmp = fegetround();
    switch(round) {
    case RoundNearestEven:
        fesetround(FE_TONEAREST);
        break;
    case RoundToInf:
        fesetround(FE_UPWARD);
        break;
    case RoundToMinusInf:
        fesetround(FE_DOWNWARD);
        break;
    default:
        Q_ASSERT(false);
    }

    bool ok;
    *dst = src.toFloat(&ok);
    fesetround(tmp);

    if( ! ok )
        return RangeError;
    else
        return NoError;
}

MathUtils::ConversionResult MathUtils::floatB64ToFloatB32(double src,
                                                          RoundingMode round,
                                                          float * dst)
{
    static double floatMax = (1.0 - pow(2, -24)) * pow(2, 128);
    if( src > floatMax || src < -floatMax )
        return RangeError;

    int tmp = fegetround();
    switch(round) {
    case RoundNearestEven:
        fesetround(FE_TONEAREST);
        break;
    case RoundToInf:
        fesetround(FE_UPWARD);
        break;
    case RoundToMinusInf:
        fesetround(FE_DOWNWARD);
        break;
    default:
        Q_ASSERT(false);
    }
    *dst = src;
    fesetround(tmp);

    return NoError;
}

static char getFormatChar(MathUtils::NotationFormat format)
{
    switch( format ) {
    case MathUtils::SimpleNotation: return 'f';
    case MathUtils::ScientificNotation: return 'e';
    case MathUtils::AutoNotation: return 'g';
    default:
        return ' ';
    }
}

template <typename T>
static QString getSpecialValue(T value)
{
    static const QString nan("NaN");
    static const QString inf("Inf");
    static const QString neginf("-Inf");

    using namespace std;
    switch( fpclassify(value) ) {
    case FP_NAN:
        return nan;
    case FP_INFINITE:
        return signbit(value) ? neginf : inf;
    default:
        return QString();
    }
}

QString MathUtils::floatB32ToString_RoundTripPrecision(float f, MathUtils::NotationFormat format)
{
    QString s = getSpecialValue(f);
    if( ! s.isEmpty() )
        return s;

    return QString::number(f, getFormatChar(format), 8);
}

QString MathUtils::floatB64ToString_RoundTripPrecision(double d, MathUtils::NotationFormat format)
{
    QString s = getSpecialValue(d);
    if( ! s.isEmpty() )
        return s;

    return QString::number(d, getFormatChar(format), 16);
}

QVariant MathUtils::intDecimalToQVariantInteger(const QString & data)
{
    bool ok = false;
    QVariant v;
    v = data.toLongLong(&ok);
    if( ! ok ) {
        v = data.toULongLong(&ok);
        if( ! ok )
            return QVariant();
    }
    return v;
}

bool MathUtils::isSmallerZero(const QVariant & integerVariant)
{
    switch( static_cast<QMetaType::Type>(integerVariant.type()) ) {
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
        return integerVariant.toLongLong() < 0;
    default:
        return false;
    }
}

bool MathUtils::minLeqValLeqMax(QVariant min, QVariant val, QVariant max)
{
    if(min.isNull())
        min = QVariant(std::numeric_limits<qint64>::min());
    if(max.isNull())
        max = QVariant(std::numeric_limits<quint64>::max());

    if( ! isSmallerZero(min) ) {
        if( ! isSmallerZero(val) ) {
            if( ! isSmallerZero(max) )
                return ((min.toULongLong() <= val.toULongLong()) &&
                        (val.toULongLong() <= max.toULongLong()));
            else
                return false;
        }
        else {
            return false;
        }
    }
    else {
        if( ! isSmallerZero(val) ) {
            if( ! isSmallerZero(max) )
                return (val.toULongLong() <= max.toULongLong());
            else
                return false;
        }
        else {
            if( min.toLongLong() <= val.toLongLong() ) {
                if( ! isSmallerZero(max) )
                    return true;
                else
                    return (val.toLongLong() <= max.toLongLong());
            }
            else
                return false;
        }
    }
}
