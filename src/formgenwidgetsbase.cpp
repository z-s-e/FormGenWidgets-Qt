/* Copyright 2014, 2015 Zeno Sebastian Endemann <zeno.endemann@googlemail.com>
 *
 * This file is part of FormGenWidgets-Qt.
 * 
 * FormGenWidgets-Qt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FormGenWidgets-Qt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FormGenWidgets-Qt.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "formgenwidgetsbase.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QRegularExpression>


FormGenAcceptResult FormGenAcceptResult::accept(QVariant value, const QString &valueString)
{
    return FormGenAcceptResult(true, {}, value, valueString);
}

FormGenAcceptResult FormGenAcceptResult::reject(QString path, QVariant value)
{
    return FormGenAcceptResult(false, path, value, {});
}


FormGenElement::FormGenElement(FormGenElement::ElementType type, QWidget *parent)
    : QWidget(parent)
    , mType(type)
    , mValueSet(type == Required)
{
    connect(this, &FormGenElement::valueSetChanged, this, &FormGenElement::valueChanged);
}

FormGenElement::ElementType FormGenElement::elementType() const
{
    return mType;
}

QVariant FormGenElement::value() const
{
    if( ! isValueSet() )
        return {};

    return valueImpl();
}

QString FormGenElement::valueString() const
{
    if( ! isValueSet() )
        return stringUnset();

    return valueStringImpl();
}

FormGenAcceptResult FormGenElement::acceptsValue(const QVariant &val) const
{
    if (elementType() == Optional && ! val.isValid())
        return FormGenAcceptResult::accept(val, stringUnset());

    return acceptsValueImpl(val);
}

void FormGenElement::setValue(const QVariant &val)
{
    if( ! acceptsValue(val).acceptable )
        return;

    setValidatedValue(val);
}

void FormGenElement::setValidatedValue(const QVariant &val)
{
    if( ! val.isValid() ) {
        setValueSet(false);
    } else {
        setVaidatedValueImpl(val);
        setValueSet(true);
    }
}

QGroupBox *FormGenElement::frameWidget() const
{
    return nullptr;
}

bool FormGenElement::isValueSet() const
{
    return mValueSet;
}

void FormGenElement::setValueSet(bool valueSet)
{
    if (mValueSet == valueSet)
        return;

    mValueSet = valueSet;
    emit valueSetChanged(mValueSet);
}


QString FormGenElement::quotedString(const QString &s)
{
    QString result;
    for(int i=0; i < s.size(); ++i) {
        int tmp = s.at(i).unicode();
        if( tmp < 0x20) {
            result += QString("\\u00");
            char c = (tmp>>4) & 15;
            if( c < 10 )
                result += QChar(static_cast<char>(c + 0x30));
            else
                result += QChar(static_cast<char>(c + 0x37));
            c = tmp & 15;
            if( c < 10 )
                result += QChar(static_cast<char>(c + 0x30));
            else
                result += QChar(static_cast<char>(c + 0x37));
        }
        else if( tmp == 0x22 ) {
            result += QString("\\\"");
        }
        else if( tmp == 0x5C ) {
            result += QString("\\\\");
        }
        else {
            result += s.at(i);
        }
    }
    return QString("\"%1\"").arg(result);
}

QString FormGenElement::stringSet()
{
    return tr("set");
}

QString FormGenElement::stringUnset()
{
    return tr("unset");
}

QString FormGenElement::stringTrue()
{
    return tr("true");
}

QString FormGenElement::stringFalse()
{
    return tr("false");
}

const QRegularExpression *FormGenElement::tagPattern()
{
    static const QRegularExpression tagLiteral("\\A[^\\p{C}/]+\\z");

    return &tagLiteral;
}

QString FormGenElement::joinedValueStringList(const QStringList &list)
{
    return QString("[%1]").arg(list.join(QStringLiteral(", ")));
}

QString FormGenElement::keyStringValuePair(const QString &key, const QString &value)
{
    return QString("%1: %2").arg(quotedString(key), value);
}

QString FormGenElement::objectString(const QStringList &keyStringValuePairs)
{
    return QString("{%1}").arg(keyStringValuePairs.join(QStringLiteral(", ")));
}

QMetaType::Type FormGenElement::variantType(const QVariant &v)
{
    return static_cast<QMetaType::Type>(v.type());
}


FormGenUnframedBase::FormGenUnframedBase(FormGenElement::ElementType type, QWidget *parent)
    : FormGenElement(type, parent)
    , mValueSet(nullptr)
{
    connect(this, &FormGenElement::valueChanged, this, &FormGenUnframedBase::updateInputWidgets);

    mLayout = new QHBoxLayout;
    mLayout->setContentsMargins(0, 0, 0, 0);
    if( elementType() == Optional ) {
        mValueSet = new QCheckBox;
        mValueSet->setChecked(false);
        mLayout->addWidget(mValueSet);
        connect(mValueSet, &QCheckBox::toggled, this, &FormGenUnframedBase::setValueSet);
        connect(this, &FormGenElement::valueSetChanged, mValueSet, &QCheckBox::setChecked);
        mValueSet->setChecked(isValueSet());
    }
    setLayout(mLayout);
}

QHBoxLayout *FormGenUnframedBase::hboxLayout() const
{
    return mLayout;
}


FormGenFramedBase::FormGenFramedBase(FormGenElement::ElementType type, QWidget *parent)
    : FormGenElement(type, parent)
{
    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    mFrame = new QGroupBox;

    if( elementType() == Optional ) {
        mFrame->setCheckable(true);
        connect(mFrame, &QGroupBox::toggled, this, &FormGenFramedBase::setValueSet);
        connect(this, &FormGenElement::valueSetChanged, mFrame, &QGroupBox::setChecked);
        mFrame->setChecked(isValueSet());
    }

    layout->addWidget(mFrame);
    setLayout(layout);
}

QGroupBox *FormGenFramedBase::frameWidget() const
{
    return mFrame;
}
