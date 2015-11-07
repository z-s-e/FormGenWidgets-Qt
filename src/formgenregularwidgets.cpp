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

#include "formgenregularwidgets.h"
#include "formgenregularwidgets_p.h"

#include "ui_formgenfilelisthead.h"

#include "mathutils.h"

#include <QColorDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QMenu>
#include <QTextBlock>
#include <QTextEdit>
#include <QTimeEdit>
#include <QToolButton>

#include <math.h>


static const double s_doubleMax = (2.0 - pow(2, -52)) * pow(2, 1023);



FormGenVoidWidget::FormGenVoidWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
{
}

QVariant FormGenVoidWidget::defaultValue() const
{
    return voidValue();
}

QVariant FormGenVoidWidget::voidValue()
{
    return QVariant(QMetaType::VoidStar, nullptr);
}

QVariant FormGenVoidWidget::valueImpl() const
{
    return defaultValue();
}

QString FormGenVoidWidget::valueStringImpl() const
{
    return stringSet();
}

FormGenAcceptResult FormGenVoidWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::VoidStar && val.value<void *>() == nullptr )
        return FormGenAcceptResult::accept(val, stringSet());

    return FormGenAcceptResult::reject({}, val);
}

void FormGenVoidWidget::setVaidatedValueImpl(const QVariant &)
{
}

void FormGenVoidWidget::updateInputWidgets()
{
}


FormGenBoolWidget::FormGenBoolWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
{
    mValue = new QComboBox;
    mValue->addItem(stringFalse());
    mValue->addItem(stringTrue());
    connect(mValue, SIGNAL(currentIndexChanged(int)), this, SIGNAL(valueChanged()));
    hboxLayout()->addWidget(mValue, 1);
    mValue->setCurrentIndex(0);

    updateInputWidgets();
}

QVariant FormGenBoolWidget::defaultValue() const
{
    return QVariant(false);
}

QVariant FormGenBoolWidget::valueImpl() const
{
    if( mValue->currentIndex() == 0)
        return QVariant(false);
    else
        return QVariant(true);
}

QString FormGenBoolWidget::valueStringImpl() const
{
    return valueImpl().toBool() ? stringTrue() : stringFalse();
}

FormGenAcceptResult FormGenBoolWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::Bool )
        return FormGenAcceptResult::accept(val, val.toBool() ? stringTrue() : stringFalse());

    return FormGenAcceptResult::reject({}, val);
}

void FormGenBoolWidget::setVaidatedValueImpl(const QVariant &val)
{
    mValue->setCurrentIndex(val.toBool() ? 1 : 0);
}

void FormGenBoolWidget::updateInputWidgets()
{
    mValue->setEnabled(isValueSet());
}


FormGenEnumWidget::FormGenEnumWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
{
    mValue = new QComboBox;
    connect(mValue, SIGNAL(currentIndexChanged(int)), this, SIGNAL(valueChanged()));
    hboxLayout()->addWidget(mValue, 1);

    updateInputWidgets();
}

void FormGenEnumWidget::addEnumValue(const QString &tag, const QString &label)
{
    if( ! tagPattern()->match(tag).hasMatch() ) {
        qWarning("FormGenChoiceComposition::addElement: tag must be nonempty and without / and control chars.");
        return;
    }

    mTags.append(tag);

    mValue->addItem(label.isEmpty() ? tag : label);
    if( mValue->currentIndex() < 0 )
        mValue->setCurrentIndex(0);
}

QVariant FormGenEnumWidget::defaultValue() const
{
    QVariantHash hash;
    if( mTags.size() > 0 )
        hash[mTags.first()] = QVariant(QMetaType::VoidStar, nullptr);
    return hash;
}

QVariant FormGenEnumWidget::valueImpl() const
{
    QVariantHash hash;
    if( mValue->currentIndex() >= 0 )
        hash[mTags.at(mValue->currentIndex())] = QVariant(QMetaType::VoidStar, nullptr);
    return hash;
}

QString FormGenEnumWidget::valueStringImpl() const
{
    if( mValue->currentIndex() < 0 )
        return objectString(QStringList());

    QString keyValue = keyStringValuePair(mTags.at(mValue->currentIndex()),
                                          stringSet());
    return objectString(QStringList({keyValue}));
}

FormGenAcceptResult FormGenEnumWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) != QMetaType::QVariantHash )
        return FormGenAcceptResult::reject({}, val);

    QVariantHash hash = val.toHash();
    if( hash.size() != 1 )
        return FormGenAcceptResult::reject({}, val);

    const QString key = hash.cbegin().key();
    if( hash.cbegin().value() != QVariant(QMetaType::VoidStar, nullptr) )
        return FormGenAcceptResult::reject(key, val);

    if( ! mTags.contains(key) )
        return FormGenAcceptResult::reject(key, val);

    return FormGenAcceptResult::accept(val, objectString(QStringList( {keyStringValuePair(key, stringSet())} )));
}

void FormGenEnumWidget::setVaidatedValueImpl(const QVariant &val)
{
    QVariantHash hash = val.toHash();
    int idx = mTags.indexOf(hash.cbegin().key());
    mValue->setCurrentIndex(idx);
}

void FormGenEnumWidget::updateInputWidgets()
{
    mValue->setEnabled(isValueSet());
}


FormGenIntWidget::FormGenIntWidget(FormGenIntWidget::InputStyle inputStyle, FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
    , mStyle(inputStyle)
    , mMinimum(0)
    , mMaximum(100)
    , mValue(0)
    , mSpinBox(nullptr)
    , mSlider(nullptr)
    , mPlain(nullptr)
{
    setupStyle();
}

FormGenIntWidget::InputStyle FormGenIntWidget::inputStyle() const
{
    return mStyle;
}

void FormGenIntWidget::setInputStyle(FormGenIntWidget::InputStyle s)
{
    if( mStyle == s )
        return;

    mStyle = s;
    setupStyle();
    emit inputStyleChanged();
}

int FormGenIntWidget::minimum() const
{
    return mMinimum;
}

void FormGenIntWidget::setMinimum(int min)
{
    if( mMinimum == min )
        return;

    mMinimum = min;
    if( mSpinBox )
        mSpinBox->setMinimum(min);
    if( mSlider )
        mSlider->setMinimum(min);
    emit minimumChanged();
    setMaximum(qMax(minimum(), maximum()));
    setIntValue(qBound(minimum(), mValue, maximum()));
}

int FormGenIntWidget::maximum() const
{
    return mMaximum;
}

void FormGenIntWidget::setMaximum(int max)
{
    if( mMaximum == max )
        return;

    mMaximum = max;
    if( mSpinBox )
        mSpinBox->setMaximum(max);
    if( mSlider )
        mSlider->setMaximum(max);
    emit maximumChanged();
    setMinimum(qMin(minimum(), maximum()));
    setIntValue(qBound(minimum(), mValue, maximum()));
}

QVariant FormGenIntWidget::defaultValue() const
{
    return QVariant(qBound(minimum(), 0, maximum()));
}

QVariant FormGenIntWidget::valueImpl() const
{
    return mValue;
}

QString FormGenIntWidget::valueStringImpl() const
{
    return QString::number(mValue);
}

FormGenAcceptResult FormGenIntWidget::acceptsValueImpl(const QVariant &val) const
{
    if( MathUtils::isIntegerType(val) ) {
        bool ok;
        int v = val.toInt(&ok);
        if( ok ) {
            if( v == qBound(minimum(), v, maximum()) )
                return FormGenAcceptResult::accept(val, QString::number(v));
        }
    }

    return FormGenAcceptResult::reject({}, val);
}

void FormGenIntWidget::setVaidatedValueImpl(const QVariant &val)
{
    setIntValue(val.toInt());
}

void FormGenIntWidget::updateInputWidgets()
{
    if( mSpinBox ) {
        mSpinBox->setValue(mValue);
        mSpinBox->setEnabled(isValueSet());
    }
    if( mSlider ) {
        mSlider->setValue(mValue);
        mSlider->setEnabled(isValueSet());
    }
    if( mPlain ) {
        mPlain->setText(valueStringImpl());
        mPlain->setEnabled(isValueSet());
    }
}

void FormGenIntWidget::checkNewValue()
{
    if( mSpinBox ) {
        int v = mSpinBox->value();
        if( v != mValue  && v == qBound(minimum(), v, maximum()))
            setIntValue(v);
    }

    if( mSlider ) {
        int v = mSlider->value();
        if( v != mValue  && v == qBound(minimum(), v, maximum()))
            setIntValue(v);
    }

    if( mPlain ) {
        bool ok;
        int v = mPlain->text().toInt(&ok);
        if( ok && v == qBound(minimum(), v, maximum()))
            setIntValue(v);
    }

    updateInputWidgets();
}

void FormGenIntWidget::setIntValue(int val)
{
    if (mValue == val)
        return;

    mValue = val;
    emit valueChanged();
}

void FormGenIntWidget::setupStyle()
{
    switch( inputStyle() ) {
    case Spinner:
        setSpinBox(true);
        setSlider(false);
        setPlain(false);
        break;
    case Slider:
        setSpinBox(false);
        setSlider(true);
        setPlain(false);
        break;
    case SpinnerSlider:
        if( mSlider && ! mSpinBox )
            setSpinBox(false);
        setSlider(true);
        setSpinBox(true);
        setPlain(false);
        break;
    case Plain:
        setSpinBox(false);
        setSlider(false);
        setPlain(true);
        break;
    }

    updateInputWidgets();
}

void FormGenIntWidget::setSpinBox(bool set)
{
    if( set && ! mSpinBox ) {
        mSpinBox = new QSpinBox;
        mSpinBox->setMinimum(minimum());
        mSpinBox->setMaximum(maximum());
        mSpinBox->setValue(mValue);
        connect(mSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkNewValue()));
        hboxLayout()->addWidget(mSpinBox, 1);
    } else if ( ! set && mSpinBox ) {
        mSpinBox->hide();
        mSpinBox->deleteLater();
        mSpinBox = nullptr;
    }
}

void FormGenIntWidget::setSlider(bool set)
{
    if( set && ! mSlider ) {
        mSlider = new QSlider(Qt::Horizontal);
        mSlider->setMinimum(minimum());
        mSlider->setMaximum(maximum());
        mSlider->setValue(mValue);
        connect(mSlider, &QSlider::valueChanged, this, &FormGenIntWidget::checkNewValue);
        hboxLayout()->addWidget(mSlider, 2);
    } else if ( ! set && mSlider ) {
        mSlider->hide();
        mSlider->deleteLater();
        mSlider = nullptr;
    }
}

void FormGenIntWidget::setPlain(bool set)
{
    if( set && ! mPlain ) {
        mPlain = new QLineEdit;
        mPlain->setText(valueString());
        connect(mPlain, &QLineEdit::editingFinished, this, &FormGenIntWidget::checkNewValue);
        hboxLayout()->addWidget(mPlain, 1);
    } else if ( ! set && mPlain ) {
        mPlain->hide();
        mPlain->deleteLater();
        mPlain = nullptr;
    }
}


FormGenFloatWidget::FormGenFloatWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
    , mMinimum(-s_doubleMax)
    , mMaximum(s_doubleMax)
{
    mValue = defaultValue().toDouble();

    mEdit = new QLineEdit;
    connect(mEdit, &QLineEdit::editingFinished, this, &FormGenFloatWidget::checkNewValue);
    hboxLayout()->addWidget(mEdit, 1);

    updateInputWidgets();
}

double FormGenFloatWidget::minimum() const
{
    return mMinimum;
}

void FormGenFloatWidget::setMinimum(double min)
{
    if( mMinimum == min )
        return;

    if( ! std::isfinite(min) )
        return;

    mMinimum = min;
    emit minimumChanged();
    setMaximum(qMax(minimum(), maximum()));
    setDoubleValue(qBound(minimum(), mValue, maximum()));
}

double FormGenFloatWidget::maximum() const
{
    return mMaximum;
}

void FormGenFloatWidget::setMaximum(double max)
{
    if( mMaximum == max )
        return;

    if( ! std::isfinite(max) )
        return;

    mMaximum = max;
    emit maximumChanged();
    setMinimum(qMin(minimum(), maximum()));
    setDoubleValue(qBound(minimum(), mValue, maximum()));
}

QVariant FormGenFloatWidget::defaultValue() const
{
    return QVariant(qBound(minimum(), double(0.0), maximum()));
}

QVariant FormGenFloatWidget::valueImpl() const
{
    return mValue;
}

QString FormGenFloatWidget::valueStringImpl() const
{
    return MathUtils::floatB64ToString_RoundTripPrecision(mValue);
}

FormGenAcceptResult FormGenFloatWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::Float || variantType(val) == QMetaType::Double ) {
        float d = val.toDouble();

        if( ! std::isfinite(d) )
            return FormGenAcceptResult::reject({}, val);

        if( d < minimum() || d > maximum() )
            return FormGenAcceptResult::reject({}, val);

        return FormGenAcceptResult::accept(val, MathUtils::floatB64ToString_RoundTripPrecision(d));
    }

    return FormGenAcceptResult::reject({}, val);
}

void FormGenFloatWidget::setVaidatedValueImpl(const QVariant &val)
{
    setDoubleValue(val.toDouble());
}

void FormGenFloatWidget::updateInputWidgets()
{
    mEdit->setText(valueStringImpl());
    mEdit->setEnabled(isValueSet());
}

void FormGenFloatWidget::checkNewValue()
{
    double newVal;
    if( MathUtils::decimalToFloatB64(mEdit->text(), MathUtils::RoundNearestEven, &newVal) == MathUtils::NoError )
        setValue(newVal);

    updateInputWidgets();
}

void FormGenFloatWidget::setDoubleValue(double val)
{
    if( mValue == val )
        return;

    mValue = val;
    emit valueChanged();
}


FormGenDateWidget::FormGenDateWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
{
    mEdit = new QDateEdit;
    mEdit->setCalendarPopup(true);
    mEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    connect(mEdit, &QDateEdit::dateChanged, this, &FormGenElement::valueChanged);
    hboxLayout()->addWidget(mEdit, 1);

    updateInputWidgets();
}

QVariant FormGenDateWidget::defaultValue() const
{
    return QDate();
}

QVariant FormGenDateWidget::valueImpl() const
{
    return mEdit->date();
}

QString FormGenDateWidget::valueStringImpl() const
{
    return mEdit->date().toString(Qt::ISODate);
}

FormGenAcceptResult FormGenDateWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::QDate )
        return FormGenAcceptResult::accept(val, val.toDate().toString(Qt::ISODate));

    return FormGenAcceptResult::reject({}, val);
}

void FormGenDateWidget::setVaidatedValueImpl(const QVariant &val)
{
    mEdit->setDate(val.toDate());
}

void FormGenDateWidget::updateInputWidgets()
{
    mEdit->setEnabled(isValueSet());
}


FormGenTimeWidget::FormGenTimeWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
{
    mEdit = new QTimeEdit;
    mEdit->setDisplayFormat(QStringLiteral("HH:mm:ss.zzz"));
    connect(mEdit, &QTimeEdit::timeChanged, this, &FormGenElement::valueChanged);
    hboxLayout()->addWidget(mEdit, 1);

    updateInputWidgets();
}

QVariant FormGenTimeWidget::defaultValue() const
{
    return QTime();
}

QVariant FormGenTimeWidget::valueImpl() const
{
    return mEdit->time();
}

QString FormGenTimeWidget::valueStringImpl() const
{
    return mEdit->time().toString(Qt::ISODate);
}

FormGenAcceptResult FormGenTimeWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::QTime )
        return FormGenAcceptResult::accept(val, val.toTime().toString(Qt::ISODate));

    return FormGenAcceptResult::reject({}, val);
}

void FormGenTimeWidget::setVaidatedValueImpl(const QVariant &val)
{
    mEdit->setTime(val.toTime());
}

void FormGenTimeWidget::updateInputWidgets()
{
    mEdit->setEnabled(isValueSet());
}


FormGenDateTimeWidget::FormGenDateTimeWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
{
    mEdit = new QDateTimeEdit;
    mEdit->setCalendarPopup(true);
    mEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd T HH:mm:ss.zzz t"));
    connect(mEdit, &QDateTimeEdit::dateTimeChanged, this, &FormGenElement::valueChanged);
    hboxLayout()->addWidget(mEdit, 1);

    updateInputWidgets();
}

QVariant FormGenDateTimeWidget::defaultValue() const
{
    return QDateTime();
}

QVariant FormGenDateTimeWidget::valueImpl() const
{
    return mEdit->dateTime();
}

QString FormGenDateTimeWidget::valueStringImpl() const
{
    return mEdit->dateTime().toString(Qt::ISODate);
}

FormGenAcceptResult FormGenDateTimeWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::QDateTime )
        return FormGenAcceptResult::accept(val, val.toDateTime().toString(Qt::ISODate));

    return FormGenAcceptResult::reject({}, val);
}

void FormGenDateTimeWidget::setVaidatedValueImpl(const QVariant &val)
{
    mEdit->setDateTime(val.toDateTime());
}

void FormGenDateTimeWidget::updateInputWidgets()
{
    mEdit->setEnabled(isValueSet());
}


FormGenColorWidget::FormGenColorWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
    , mValue(Qt::black)
{
    mButton = new QPushButton;
    connect(mButton, &QPushButton::clicked, this, &FormGenColorWidget::showColorChooser);
    hboxLayout()->addWidget(mButton, 1);

    updateInputWidgets();
}

QVariant FormGenColorWidget::defaultValue() const
{
    return QColor(Qt::black);
}

QVariant FormGenColorWidget::valueImpl() const
{
    return mValue;
}

QString FormGenColorWidget::valueStringImpl() const
{
    return mValue.name();
}

FormGenAcceptResult FormGenColorWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::QColor ) {
        auto c = val.value<QColor>();
        if( c.isValid() ) {
            if( c.alpha() == 255 )
                return FormGenAcceptResult::accept(val, c.name());
            else
                qWarning("Color with alpha channel not supported"); // Color dialog has no alpha support, so be consistent
        }
    }

    return FormGenAcceptResult::reject({}, val);
}

void FormGenColorWidget::setVaidatedValueImpl(const QVariant &val)
{
    QColor c = val.value<QColor>();

    if( mValue == c )
        return;

    mValue = c;
    emit valueChanged();
}

void FormGenColorWidget::updateInputWidgets()
{
    QPalette p = mButton->palette();
    p.setColor(QPalette::Button, mValue);
    mButton->setPalette(p);
    mButton->setEnabled(isValueSet());
}

void FormGenColorWidget::showColorChooser()
{
    auto c = QColorDialog::getColor(mValue, this);
    if( c.isValid() )
        setValue(c);
}


FormGenTextWidget::FormGenTextWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
    , mEdit(new QLineEdit)
{
    connect(mEdit, &QLineEdit::textChanged, this, &FormGenElement::valueChanged);
    hboxLayout()->addWidget(mEdit, 1);

    updateInputWidgets();
}

QVariant FormGenTextWidget::defaultValue() const
{
    return QString();
}

QVariant FormGenTextWidget::valueImpl() const
{
    return mEdit->text();
}

QString FormGenTextWidget::valueStringImpl() const
{
    return quotedString(mEdit->text());
}

FormGenAcceptResult FormGenTextWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) == QMetaType::QString )
        return FormGenAcceptResult::accept(val, quotedString(val.toString()));

    return FormGenAcceptResult::reject({}, val);
}

void FormGenTextWidget::setVaidatedValueImpl(const QVariant &val)
{
    const QString s = val.toString();
    if( mEdit->text() != s )
        mEdit->setText(s);
}

void FormGenTextWidget::updateInputWidgets()
{
    mEdit->setEnabled(isValueSet());
}


FormGenFileUrlWidget::FormGenFileUrlWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenTextWidget(type, parent)
    , mChooseOptions(FormGenUriChooseFile)
    , mFileChooser(new QToolButton)
    , mChooseMenu(new QMenu(this))
{
    mFileChooser->setText(tr("..."));
    connect(mFileChooser, &QToolButton::clicked, this, &FormGenFileUrlWidget::showFileChooser);
    layout()->addWidget(mFileChooser);

    mChooseMenu->addAction(tr("Select file..."), this, SLOT(showFileChooser()));
    mChooseMenu->addAction(tr("Select directory..."), this, SLOT(showDirChooser()));

    updateInputWidgets();
}

void FormGenFileUrlWidget::setMimeTypes(const QStringList &mimeList)
{
    mMimeTypes = mimeList;
}

QStringList FormGenFileUrlWidget::mimeTypes() const
{
    return mMimeTypes;
}

void FormGenFileUrlWidget::setChooseOptions(FormGenFileUriChooseOptions opt)
{
    if( mChooseOptions == opt )
        return;

    mFileChooser->disconnect(this);
    switch( opt ) {
    case FormGenUriChooseFile:
        connect(mFileChooser, &QToolButton::clicked, this, &FormGenFileUrlWidget::showFileChooser);
        break;
    case FormGenUriChooseDirectpory:
        connect(mFileChooser, &QToolButton::clicked, this, &FormGenFileUrlWidget::showDirChooser);
        break;
    case FormGenUriChooseFileOrDirectory:
        connect(mFileChooser, &QToolButton::clicked, this, &FormGenFileUrlWidget::showMenu);
    }
    mChooseOptions = opt;
}

FormGenFileUriChooseOptions FormGenFileUrlWidget::chooseOptions() const
{
    return mChooseOptions;
}

void FormGenFileUrlWidget::updateInputWidgets()
{
    mFileChooser->setEnabled(isValueSet());
    FormGenTextWidget::updateInputWidgets();
}

void FormGenFileUrlWidget::showFileChooser()
{
    QFileDialog dialog(this, tr("Select file"));
    dialog.setFileMode(QFileDialog::AnyFile);
    if( ! mMimeTypes.isEmpty() )
         dialog.setMimeTypeFilters(mMimeTypes);

    if( dialog.exec() != QDialog::Accepted )
        return;
    if( dialog.selectedUrls().size() < 1 )
        return;
    setVaidatedValueImpl(dialog.selectedUrls().first().toString());
}

void FormGenFileUrlWidget::showDirChooser()
{
    QFileDialog dialog(this, tr("Select directory"));
    dialog.setFileMode(QFileDialog::Directory);

    if( dialog.exec() != QDialog::Accepted )
        return;
    if( dialog.selectedUrls().size() < 1 )
        return;
    setVaidatedValueImpl(dialog.selectedUrls().first().toString());
}

void FormGenFileUrlWidget::showMenu()
{
    mChooseMenu->exec(mFileChooser->mapToGlobal(QPoint(0, mFileChooser->height())));
}


FormGenFileUrlList::FormGenFileUrlList(FormGenElement::ElementType type, QWidget *parent)
    : FormGenFramedBase(type, parent)
    , mChooseOptions(FormGenUriChooseFileOrDirectory)
    , mModel(new FormGenFileUrlListModel(this))
    , mHead(new Ui::FormGenFileListHead)
    , mHeadWidget(new QWidget)
{
    mHead->setupUi(mHeadWidget);
    mModel->setEmptyUrlColor(palette().color(QPalette::Disabled, QPalette::Text));

    connect(mModel, &QAbstractListModel::dataChanged, this, &FormGenFileUrlList::valueChanged);
    connect(mModel, &QAbstractListModel::modelReset, this, &FormGenFileUrlList::valueChanged);
    connect(mModel, &QAbstractListModel::rowsInserted, this, &FormGenFileUrlList::valueChanged);
    connect(mModel, &QAbstractListModel::rowsMoved, this, &FormGenFileUrlList::valueChanged);
    connect(mModel, &QAbstractListModel::rowsRemoved, this, &FormGenFileUrlList::valueChanged);
    connect(this, &FormGenElement::valueChanged, this, &FormGenFileUrlList::updateInputWidgets);

    mHead->listView->setModel(mModel);
    connect(mHead->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &FormGenFileUrlList::updateInputWidgets);
    connect(mHead->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormGenFileUrlList::updateInputWidgets);
    connect(mHead->buttonRemove, &QPushButton::clicked, this, &FormGenFileUrlList::removeSelection);
    connect(mHead->buttonClear, &QPushButton::clicked, this, &FormGenFileUrlList::clearAll);
    connect(mHead->spinPosition, SIGNAL(valueChanged(int)), this, SLOT(moveCurrent()));
    connect(mHead->buttonAddEmpty, &QPushButton::clicked, this, &FormGenFileUrlList::insertEmpty);
    connect(mHead->buttonAddDir, &QPushButton::clicked, this, &FormGenFileUrlList::insertDir);
    connect(mHead->buttonAddFiles, &QPushButton::clicked, this, &FormGenFileUrlList::insertFiles);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mHeadWidget);
    frameWidget()->setLayout(layout);

    updateInputWidgets();
}

void FormGenFileUrlList::setMimeTypes(const QStringList &mimeList)
{
    mMimeTypes = mimeList;
}

QStringList FormGenFileUrlList::mimeTypes() const
{
    return mMimeTypes;
}

void FormGenFileUrlList::setChooseOptions(FormGenFileUriChooseOptions opt)
{
    if( mChooseOptions == opt )
        return;

    mChooseOptions = opt;
    updateInputWidgets();
}

FormGenFileUriChooseOptions FormGenFileUrlList::chooseOptions() const
{
    return mChooseOptions;
}

QVariant FormGenFileUrlList::defaultValue() const
{
    return QVariantList();
}

QVariant FormGenFileUrlList::valueImpl() const
{
    QVariantList list;

    for( int i = 0; i < mModel->rowCount({}); ++i )
        list.append(mModel->urlAt(i));

    return list;
}

QString FormGenFileUrlList::valueStringImpl() const
{
    QStringList list;

    for( int i = 0; i <  mModel->rowCount({}); ++i )
        list.append(quotedString(mModel->urlAt(i)));

    return joinedValueStringList(list);
}

FormGenAcceptResult FormGenFileUrlList::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) != QMetaType::QVariantList )
        return FormGenAcceptResult::reject({}, val);

    const QVariantList list = val.toList();

    QStringList valueStrings;

    for( int i = 0; i < list.size(); ++i ) {
        if( variantType(list.at(i)) != QMetaType::QString )
            return FormGenAcceptResult::reject(QString::number(i), list.at(i));
        valueStrings.append(quotedString(list.at(i).toString()));
    }

    return FormGenAcceptResult::accept(val, joinedValueStringList(valueStrings));
}

void FormGenFileUrlList::setVaidatedValueImpl(const QVariant &val)
{
    mModel->resetData(val.toList());
}

void FormGenFileUrlList::updateInputWidgets()
{
    mHeadWidget->setEnabled(isValueSet());

    const int rows = mModel->rowCount({});
    const int currentRow = mHead->listView->selectionModel()->currentIndex().row();
    const bool hasSelection = mHead->listView->selectionModel()->selectedRows().size() > 0;

    mHead->buttonRemove->setEnabled(hasSelection);
    mHead->buttonClear->setEnabled(rows > 0);
    mHead->buttonAddDir->setEnabled(mChooseOptions & FormGenUriChooseDirectpory);
    mHead->buttonAddFiles->setEnabled(mChooseOptions & FormGenUriChooseFile);

    if( currentRow < 0 ) {
        mHead->spinPosition->setEnabled(false);
        mHead->spinPosition->setValue(0);
    } else {
        mHead->spinPosition->setEnabled(true);
        mHead->spinPosition->setMaximum(rows - 1);
        mHead->spinPosition->setValue(currentRow);
    }
}

void FormGenFileUrlList::removeSelection()
{
    const int currentRow = mHead->listView->selectionModel()->currentIndex().row();

    mModel->removeUrls(mHead->listView->selectionModel()->selectedRows());

    const auto newCurrent = mModel->index( qMin(currentRow, mModel->urlCount() - 1) );
    mHead->listView->selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
}

void FormGenFileUrlList::clearAll()
{
    mModel->clear();
}

void FormGenFileUrlList::moveCurrent()
{
    const int currentRow = mHead->listView->selectionModel()->currentIndex().row();
    const int targetRow = mHead->spinPosition->value();

    mModel->moveUrl(currentRow, targetRow);
}

void FormGenFileUrlList::insertEmpty()
{
    const int currentRow = mHead->listView->selectionModel()->currentIndex().row();

    mModel->insertUrl(QString(), currentRow + 1);

    const auto newRow = mModel->index(currentRow + 1);
    mHead->listView->selectionModel()->setCurrentIndex(newRow, QItemSelectionModel::NoUpdate);
    mHead->listView->edit(newRow);
}

void FormGenFileUrlList::insertDir()
{
    const int currentRow = mHead->listView->selectionModel()->currentIndex().row();

    QFileDialog dialog(this, tr("Select directory"));
    dialog.setFileMode(QFileDialog::Directory);

    if( dialog.exec() != QDialog::Accepted )
        return;
    if( dialog.selectedUrls().size() < 1 )
        return;

    mModel->insertUrl(dialog.selectedUrls().first().toString(), currentRow + 1);

    const auto newRow = mModel->index(currentRow + 1);
    mHead->listView->selectionModel()->setCurrentIndex(newRow, QItemSelectionModel::NoUpdate);
}

void FormGenFileUrlList::insertFiles()
{
    const int currentRow = mHead->listView->selectionModel()->currentIndex().row();

    QFileDialog dialog(this, tr("Select files"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if( ! mMimeTypes.isEmpty() ) {
        dialog.setMimeTypeFilters(mMimeTypes);
    }

    if( dialog.exec() != QDialog::Accepted )
        return;
    if( dialog.selectedUrls().size() < 1 )
        return;

    mModel->insertUrls(dialog.selectedUrls(), currentRow + 1);

    const auto newRow = mModel->index(currentRow + dialog.selectedUrls().size());
    mHead->listView->selectionModel()->setCurrentIndex(newRow, QItemSelectionModel::NoUpdate);
}


FormGenFormatStringWidget::FormGenFormatStringWidget(FormGenElement::ElementType type, QWidget *parent)
    : FormGenUnframedBase(type, parent)
    , mTextEdit(new QTextEdit)
    , mInsertMenu(new QComboBox)
{
    connect(mTextEdit, &QTextEdit::textChanged, this, &FormGenElement::valueChanged);
    mTextEdit->setMaximumHeight(fontMetrics().height() * 2);
    hboxLayout()->addWidget(mTextEdit, 1);

    mInsertMenu->addItem(QString());
    connect(mInsertMenu, SIGNAL(currentIndexChanged(int)), this, SLOT(insertVoidElement()));
    hboxLayout()->addWidget(mInsertMenu);

    mTextEdit->setAcceptRichText(false);
    mTextEdit->document()->documentLayout()->registerHandler(FormGenFormatStringTextObject::FormGenVoidTextFormat,
                                                             new FormGenFormatStringTextObject(mTextEdit, this));

    updateInputWidgets();
}

void FormGenFormatStringWidget::addVoidElement(const QString &tag)
{
    if( ! tagPattern()->match(tag).hasMatch() ) {
        qWarning("FormGenFormatStringWidget::addVoidElement: tag must be nonempty and without / and control chars.");
        return;
    }

    if( tag == textTag() ) {
        qWarning("FormGenFormatStringWidget::addVoidElement: text tag reserved.");
        return;
    }

    mInsertMenu->addItem(tag);
    mVoidTags.insert(tag);
}

QVariant FormGenFormatStringWidget::defaultValue() const
{
    return QVariantList();
}

QString FormGenFormatStringWidget::textTag()
{
    return QStringLiteral("text");
}

QVariant FormGenFormatStringWidget::valueImpl() const
{
    QVariantList list;

    const QTextDocument * doc = mTextEdit->document();

    QString tmp;
    for( auto blockIt = doc->begin(); blockIt != doc->end(); blockIt = blockIt.next()) {
        for( auto fragIt = blockIt.begin(); fragIt != blockIt.end(); ++fragIt ) {
            int objectType = fragIt.fragment().charFormat().objectType();
            if( objectType == FormGenFormatStringTextObject::FormGenVoidTextFormat ) {
                if( ! tmp.isEmpty() ) {
                    QVariantHash txt;
                    txt[textTag()] = tmp;
                    list.append(txt);
                    tmp.clear();
                }
                QVariantHash v;
                v[ fragIt.fragment().charFormat().property(FormGenFormatStringTextObject::VoidTag).toString() ]
                        = QVariant(QMetaType::VoidStar, nullptr);
                list.append(v);
            } else {
                tmp += fragIt.fragment().text();
            }
        }
    }
    if( ! tmp.isEmpty() ) {
        QVariantHash txt;
        txt[textTag()] = tmp;
        list.append(txt);
    }

    return list;
}

QString FormGenFormatStringWidget::valueStringImpl() const
{
    const QVariantList variantList = valueImpl().toList();
    QStringList stringList;

    for( const auto &v : variantList ) {
        const QVariantHash hash = v.toHash();
        const QString key = hash.begin().key();

        QString keyValue = keyStringValuePair(key,  key == textTag() ? hash.begin().value().toString()
                                                                     : stringSet());
        stringList.append(objectString(QStringList({keyValue})));
    }

    return joinedValueStringList(stringList);
}

FormGenAcceptResult FormGenFormatStringWidget::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) != QMetaType::QVariantList )
        return FormGenAcceptResult::reject({}, val);

    const QVariantList variantList = val.toList();
    QStringList stringList;
    for( int i = 0; i < variantList.size(); ++i ) {
        if( variantType(variantList.at(i)) != QMetaType::QVariantHash )
            return FormGenAcceptResult::reject(QString::number(i), val);

        const QVariantHash element = variantList.at(i).toHash();
        if( element.size() != 1 )
            return FormGenAcceptResult::reject(QString::number(i), val);

        const QString key = element.begin().key();

        if( key == textTag() ) {
            const QVariant v = element.begin().value();
            if( variantType(v) != QMetaType::QString )
                return FormGenAcceptResult::reject(QString::number(i), val);

            QString keyValue = keyStringValuePair(key, v.toString());
            stringList.append(objectString(QStringList({keyValue})));
        } else {
            if( ! mVoidTags.contains(key) || element.begin().value() != QVariant(QMetaType::VoidStar, nullptr) )
                return FormGenAcceptResult::reject(QString::number(i), val);

            QString keyValue = keyStringValuePair(key, stringSet());
            stringList.append(objectString(QStringList({keyValue})));
        }
    }

    return FormGenAcceptResult::accept(val, joinedValueStringList(stringList));
}

void FormGenFormatStringWidget::setVaidatedValueImpl(const QVariant &val)
{
    mTextEdit->clear();

    const QVariantList variantList = val.toList();
    for( const auto &elementVariant : variantList ) {
        const QVariantHash element = elementVariant.toHash();
        const QString key = element.begin().key();

        if( key == textTag() ) {
            QTextCursor cursor = mTextEdit->textCursor();
            cursor.insertText(element.begin().value().toString());
            mTextEdit->setTextCursor(cursor);
        } else {
            insertVoidElement(key);
        }
    }
}

void FormGenFormatStringWidget::updateInputWidgets()
{
    mTextEdit->setEnabled(isValueSet());
    mInsertMenu->setEnabled(isValueSet());
}

void FormGenFormatStringWidget::insertVoidElement()
{
    if( mInsertMenu->currentIndex() < 1 )
        return;

    insertVoidElement(mInsertMenu->currentText());

    mInsertMenu->setCurrentIndex(0);
}

void FormGenFormatStringWidget::insertVoidElement(const QString &voidTag)
{
    QTextCharFormat myCharFormat;
    myCharFormat.setObjectType(FormGenFormatStringTextObject::FormGenVoidTextFormat);
    myCharFormat.setProperty(FormGenFormatStringTextObject::VoidTag, voidTag);

    QTextCursor cursor = mTextEdit->textCursor();
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), myCharFormat);
    mTextEdit->setTextCursor(cursor);
}
