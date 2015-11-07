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

#ifndef FORMGENWIDGETS_QT_BASE_H
#define FORMGENWIDGETS_QT_BASE_H

#include <QVariant>
#include <QWidget>

#include "formgenwidgets_global.h"

class QCheckBox;
class QGroupBox;
class QHBoxLayout;


class FORMGENWIDGETS_EXPORT FormGenAcceptResult {
public:
    static FormGenAcceptResult accept(QVariant value, const QString &valueString);
    static FormGenAcceptResult reject(QString path, QVariant value);

    bool acceptable;
    QString path;
    QVariant value;
    QString valueString;

private:
    FormGenAcceptResult(bool accept_, QString path_, QVariant value_, const QString &valueString_)
        : acceptable(accept_)
        , path(path_)
        , value(value_)
        , valueString(valueString_)
    {}
};


class FORMGENWIDGETS_EXPORT FormGenElement : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    enum ElementType {
        Required, Optional
    };

    explicit FormGenElement(ElementType type = Required, QWidget * parent = nullptr);

    ElementType elementType() const;

    QVariant value() const;
    QString valueString() const;
    FormGenAcceptResult acceptsValue(const QVariant &val) const;
    void setValue(const QVariant &val);

    virtual QVariant defaultValue() const = 0;

    virtual QGroupBox *frameWidget() const;

    static QString quotedString(const QString &s);
    static const QRegularExpression *tagPattern();
    static QString joinedValueStringList(const QStringList &list);
    static QString keyStringValuePair(const QString &key, const QString &value);
    static QString objectString(const QStringList &keyStringValuePairs);
    static QMetaType::Type variantType(const QVariant &v);

    static QString stringSet();
    static QString stringUnset();
    static QString stringTrue();
    static QString stringFalse();

signals:
    void valueChanged();
    void valueSetChanged(bool isSet);

protected:
    bool isValueSet() const;

    virtual QVariant valueImpl() const = 0;
    virtual QString valueStringImpl() const = 0;
    virtual FormGenAcceptResult acceptsValueImpl(const QVariant &val) const = 0;

    void setValidatedValue(const QVariant &val);
    virtual void setVaidatedValueImpl(const QVariant &val) = 0;

    struct CompositionElement {
        CompositionElement(const QString &_tag = QString(), FormGenElement *_element = nullptr)
            : tag(_tag)
            , element(_element)
        {}

        QString tag;
        FormGenElement *element;
    };

protected slots:
     void setValueSet(bool valueSet);

private:
    ElementType mType;
    bool mValueSet;

    friend class FormGenRecordComposition;
    friend class FormGenChoiceComposition;
    friend class FormGenListBagComposition;
};


class FORMGENWIDGETS_EXPORT FormGenUnframedBase : public FormGenElement {
    Q_OBJECT

public:
    explicit FormGenUnframedBase(ElementType type = Required, QWidget * parent = nullptr);

protected:
    QHBoxLayout *hboxLayout() const;

protected slots:
    virtual void updateInputWidgets() = 0;

private:
    QCheckBox * mValueSet;
    QHBoxLayout * mLayout;
};


class FORMGENWIDGETS_EXPORT FormGenFramedBase : public FormGenElement {
    Q_OBJECT

public:
    explicit FormGenFramedBase(ElementType type = Required, QWidget * parent = nullptr);

    QGroupBox *frameWidget() const override;

private:
    QGroupBox * mFrame;
};


#endif // FORMGENWIDGETS_QT_BASE_H
