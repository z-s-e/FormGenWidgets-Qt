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

#include "formgencompositionwidgets.h"
#include "formgencompositionwidgets_p.h"

#include "ui_formgenlistbaghead.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QSignalMapper>
#include <QStackedLayout>
#include <QStringListModel>


static const int s_frameSubContentMargin = 8;


FormGenRecordComposition::FormGenRecordComposition(FormGenElement::ElementType type, QWidget *parent)
    : FormGenFramedBase(type, parent)
    , mLayout(new QFormLayout)
    , mUpdating(NotUpdatingState)
{
    mLayout->setContentsMargins(0, 0, 0, 0);
    frameWidget()->setLayout(mLayout);
}

void FormGenRecordComposition::addElement(const QString &tag,
                                          FormGenElement *element,
                                          const QString &label)
{
    if( ! tagPattern()->match(tag).hasMatch() ) {
        qWarning("FormGenRecordComposition::addElement: tag must be nonempty and without / and control chars.");
        return;
    }

    if( mTagIndexMap.contains(tag) ) {
        qWarning("FormGenRecordComposition::addElement: duplicated tag %s.", qPrintable(tag));
        return;
    }

    mElements.append(CompositionElement(tag, element));
    mTagIndexMap[tag] = mElements.size() - 1;
    connect(element, &FormGenElement::valueChanged,
            this, &FormGenRecordComposition::childValueChanged);

    if( element->frameWidget() ) {
        element->frameWidget()->setTitle(label.isEmpty() ? tag : label);
        mLayout->addRow(element);
    } else {
        mLayout->addRow(label.isEmpty() ? tag : label, element);
    }

    emit valueChanged();
}

FormGenElement *FormGenRecordComposition::element(const QString &tag) const
{
    QHash<QString, int>::const_iterator it = mTagIndexMap.find(tag);
    if( it == mTagIndexMap.cend() )
        return nullptr;

    return mElements.at(it.value()).element;
}

QVariant FormGenRecordComposition::defaultValue() const
{
    QVariantHash map;
    for( const auto &elm : mElements )
        map[elm.tag] = elm.element->defaultValue();
    return map;
}

QVariant FormGenRecordComposition::valueImpl() const
{
    QVariantHash map;
    for( const auto &elm : mElements )
        map[elm.tag] = elm.element->value();
    return map;
}

QString FormGenRecordComposition::valueStringImpl() const
{
    QStringList list;

    for( auto it = mElements.cbegin(); it != mElements.cend(); ++it )
        list.append(keyStringValuePair(it->tag, it->element->valueString()));

    return objectString(list);
}

FormGenAcceptResult FormGenRecordComposition::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) != QMetaType::QVariantHash )
        return FormGenAcceptResult::reject({}, val);

    QVariantHash hash = val.toHash();
    QStringList valueStringList;
    QSet<QString> processedTags;

    for( const auto &elm : mElements ) {
        auto elementAccepts = elm.element->acceptsValue(hash.value(elm.tag));
        if( ! elementAccepts.acceptable ) {
            QString path = elm.tag;
            if( ! elementAccepts.path.isEmpty() )
                path += QString("/%1").arg(elementAccepts.path);
            return FormGenAcceptResult::reject(path, elementAccepts.value);
        }
        valueStringList.append(FormGenElement::keyStringValuePair(elm.tag, elementAccepts.valueString));
        processedTags.insert(elm.tag);
    }

    QSet<QString> remainingTags = hash.keys().toSet() - processedTags;
    if( ! remainingTags.isEmpty() )
        return FormGenAcceptResult::reject(*remainingTags.cbegin(),
                                           hash.value(*remainingTags.cbegin()));

    return FormGenAcceptResult::accept(val, FormGenElement::objectString(valueStringList));
}

void FormGenRecordComposition::setVaidatedValueImpl(const QVariant &val)
{
    mUpdating = UpdatingState;

    const QVariantHash map = val.toHash();
    for( auto it = map.cbegin(); it != map.cend(); ++it )
        mElements.at(mTagIndexMap.value(it.key())).element->setValidatedValue(it.value());

    if( mUpdating == UpdatingWithChangeState )
        emit valueChanged();

    mUpdating = NotUpdatingState;
}

void FormGenRecordComposition::childValueChanged()
{
    if( mUpdating == NotUpdatingState )
        emit valueChanged();
    else
        mUpdating = UpdatingWithChangeState;
}


FormGenChoiceComposition::FormGenChoiceComposition(FormGenElement::ElementType type, Style style, QWidget *parent)
    : FormGenFramedBase(type, parent)
    , mComboBox(new QComboBox)
    , mElementContainer(new QWidget)
    , mElementLayout(new QStackedLayout)
{
    switch( style ) {
    case ComboBoxStyle:
        mContainer = new FormGenChoiceCompositionComboListContainer(false);
        break;
    case ListStyle:
        mContainer = new FormGenChoiceCompositionComboListContainer(true);
        break;
    default:
        mContainer = new FormGenChoiceCompositionRadioContainer;
    }

    connect(mContainer, &FormGenChoiceCompositionContainer::currentIndexChanged,
            this, &FormGenChoiceComposition::valueChanged);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mContainer);
    frameWidget()->setLayout(layout);
}

void FormGenChoiceComposition::addElement(const QString &tag, FormGenElement *element, const QString &label)
{
    if( ! tagPattern()->match(tag).hasMatch() ) {
        qWarning("FormGenChoiceComposition::addElement: tag must be nonempty and without / and control chars.");
        return;
    }

    if( mTagIndexMap.contains(tag) ) {
        qWarning("FormGenChoiceComposition::addElement: duplicated tag %s.", qPrintable(tag));
        return;
    }

    mElements.append(CompositionElement(tag, element));
    mTagIndexMap[tag] = mElements.size() - 1;

    const QString l = label.isEmpty() ? tag : label;

    mContainer->addElement(l, element);

    connect(element, &FormGenElement::valueChanged, this, &FormGenElement::valueChanged);

    mContainer->setCurrentIndex(0);
}

FormGenElement *FormGenChoiceComposition::element(const QString &tag) const
{
    QHash<QString, int>::const_iterator it = mTagIndexMap.find(tag);
    if( it == mTagIndexMap.cend() )
        return nullptr;

    return mElements.at(it.value()).element;
}

QVariant FormGenChoiceComposition::defaultValue() const
{
    QVariantHash map;

    if( mElements.size() > 0)
        map[mElements.first().tag] = mElements.first().element->defaultValue();

    return map;
}

QVariant FormGenChoiceComposition::valueImpl() const
{
    QVariantHash map;
    const int idx = mContainer->currentIndex();

    if( idx < 0 )
        return map;

    map[ mElements.at(idx).tag ] = mElements.at(idx).element->value();
    return map;
}

QString FormGenChoiceComposition::valueStringImpl() const
{
    const int idx = mContainer->currentIndex();

    if( idx < 0 )
        return objectString(QStringList());

    QString keyValue = keyStringValuePair(mElements.at(idx).tag,
                                          mElements.at(idx).element->valueString());

    return objectString(QStringList({keyValue}));
}

FormGenAcceptResult FormGenChoiceComposition::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) != QMetaType::QVariantHash )
        return FormGenAcceptResult::reject({}, val);

    QVariantHash hash = val.toHash();
    if( hash.size() != 1 )
        return FormGenAcceptResult::reject({}, hash);

    auto it = mTagIndexMap.find(hash.cbegin().key());
    if( it == mTagIndexMap.cend() )
        return FormGenAcceptResult::reject({}, hash);

    auto elementAccepts = mElements.at(it.value()).element->acceptsValue(hash.cbegin().value());
    if( ! elementAccepts.acceptable ) {
        QString path = it.key();
        if( ! elementAccepts.path.isEmpty() )
            path += QString("/%1").arg(elementAccepts.path);
        return FormGenAcceptResult::reject(path, elementAccepts.value);
    }

    QString keyValue = FormGenElement::keyStringValuePair(it.key(), elementAccepts.valueString);
    return FormGenAcceptResult::accept(val, FormGenElement::objectString(QStringList({keyValue})));
}

void FormGenChoiceComposition::setVaidatedValueImpl(const QVariant &val)
{
    int idx;
    QVariant choiceVal;

    if( variantType(val) == QMetaType::QVariantMap ) {
        auto map = val.toMap();
        idx = mTagIndexMap.value(map.cbegin().key());
        choiceVal = map.cbegin().value();
    } else {
        auto map = val.toHash();
        idx = mTagIndexMap.value(map.cbegin().key());
        choiceVal = map.cbegin().value();
    }

    mContainer->setCurrentIndex(idx);
    mElements.at(idx).element->setValidatedValue(choiceVal);
}


FormGenChoiceCompositionComboListContainer::FormGenChoiceCompositionComboListContainer(bool listMode, QWidget *parent)
    : FormGenChoiceCompositionContainer(parent)
    , mComboBox(nullptr)
    , mListView(nullptr)
    , mListModel(nullptr)
    , mElementContainer(new QWidget)
    , mElementLayout(new QStackedLayout)
{
    mElementContainer->setLayout(mElementLayout);

    auto outerLayout = new QGridLayout;
    outerLayout->setContentsMargins(0, 0, 0, 0);

    if( listMode ) {
        mListView = new QListView;
        mListModel = new QStringListModel;
        mListView->setModel(mListModel);
        connect(mListView->selectionModel(), &QItemSelectionModel::currentChanged,
                [this] ()
        {
            const int row = mListView->selectionModel()->currentIndex().row();
            mElementLayout->setCurrentIndex(row);
            emit currentIndexChanged(row);
        });
        outerLayout->addWidget(mListView, 0, 0, 1, 2);
    } else {
        mComboBox = new QComboBox;
        connect(mComboBox, SIGNAL(currentIndexChanged(int)), mElementLayout, SLOT(setCurrentIndex(int)));
        connect(mComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChanged(int)));
        outerLayout->addWidget(mComboBox, 0, 0, 1, 2);
    }

    outerLayout->addItem(new QSpacerItem(s_frameSubContentMargin, 1), 1, 0);
    outerLayout->addWidget(mElementContainer, 1, 1);

    setLayout(outerLayout);
}

void FormGenChoiceCompositionComboListContainer::addElement(const QString &label, FormGenElement *element)
{
    mLabels.append(label);

    if( mListView )
        mListModel->setStringList(mLabels);
    else
        mComboBox->addItem(label);

    if( element->frameWidget() ) {
        element->frameWidget()->setTitle(label);
        mElementLayout->addWidget(element);
    } else {
        QWidget *w = new QWidget;
        auto *layout = new QFormLayout;
        layout->setContentsMargins(0, s_frameSubContentMargin, 0, 0);
        layout->addRow(label, element);
        w->setLayout(layout);
        mElementLayout->addWidget(w);
    }
}
int FormGenChoiceCompositionComboListContainer::currentIndex() const
{
    if( mListView )
        return mListView->selectionModel()->currentIndex().row();
    else
        return mComboBox->currentIndex();
}

void FormGenChoiceCompositionComboListContainer::setCurrentIndex(int idx)
{
    if( mListView )
        return mListView->selectionModel()->setCurrentIndex(mListModel->index(idx),
                                                            QItemSelectionModel::ClearAndSelect);
    else
        return mComboBox->setCurrentIndex(idx);
}


FormGenChoiceCompositionRadioContainer::FormGenChoiceCompositionRadioContainer(QWidget *parent)
    : FormGenChoiceCompositionContainer(parent)
    , mCurrentIndex(-1)
    , mGroup(new QButtonGroup)
    , mLayout(new QFormLayout)
    , mMapper(new QSignalMapper)
{
    mLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mLayout);

    connect(mMapper, SIGNAL(mapped(int)), this, SLOT(radioToggled(int)));
}

void FormGenChoiceCompositionRadioContainer::addElement(const QString &label, FormGenElement *element)
{
    auto *w = new QWidget;
    auto *radio = new QRadioButton;
    mGroup->addButton(radio);

    element->setEnabled(false);
    connect(radio, SIGNAL(toggled(bool)), mMapper, SLOT(map()));
    mMapper->setMapping(radio, mContainerList.size());
    mContainerList.append(ElementContainer(radio, element));

    if( element->frameWidget() ) {
        element->frameWidget()->setTitle(label);

        auto innerLayout = new QGridLayout;
        innerLayout->setContentsMargins(0, 0, 0, 0);
        innerLayout->addWidget(radio, 0, 0);
        innerLayout->addWidget(element, 0, 1, 2, 1);
        innerLayout->setRowStretch(1, 1);
        innerLayout->setColumnStretch(1, 1);

        w->setLayout(innerLayout);
        mLayout->addRow(w);
    } else {
        auto innerLayout = new QHBoxLayout;
        innerLayout->setContentsMargins(0, 0, 0, 0);
        innerLayout->addWidget(radio);
        innerLayout->addWidget(element);

        w->setLayout(innerLayout);
        mLayout->addRow(label, w);
    }

    mContainerList.first().button->setChecked(true);
}

int FormGenChoiceCompositionRadioContainer::currentIndex() const
{
    return mCurrentIndex;
}

void FormGenChoiceCompositionRadioContainer::setCurrentIndex(int idx)
{
    if( mCurrentIndex == idx )
        return;

    if( mCurrentIndex >= 0 )
        mContainerList.at(mCurrentIndex).element->setEnabled(false);

    mCurrentIndex = idx;

    mContainerList.at(mCurrentIndex).element->setEnabled(true);

    emit currentIndexChanged(mCurrentIndex);
}

void FormGenChoiceCompositionRadioContainer::radioToggled(int i)
{
    if( mContainerList.at(i).button->isChecked() )
        setCurrentIndex(i);
}


FormGenListBagComposition::FormGenListBagComposition(Mode mode, FormGenElement::ElementType type, QWidget *parent)
    : FormGenFramedBase(type, parent)
    , mMode(mode)
    , mHead(new Ui::FormGenListBagHead)
    , mHeadWidget(new QWidget)
    , mElement(nullptr)
    , mElementWrapper(nullptr)
    , mUpdating(false)
{
    mHead->setupUi(mHeadWidget);

    if( mMode == ListMode ) {
        mModel.list = new FormGenListModel(this);
        mHead->listView->setModel(mModel.list);
    } else {
        mModel.bag = new FormGenBagModel(this);
        mHead->listView->setModel(mModel.bag);
    }

    mHead->labelPosition->setVisible(mMode == ListMode);
    mHead->spinPosition->setVisible(mMode == ListMode);

    connect(model(), &QAbstractListModel::dataChanged, this, &FormGenListBagComposition::valueChanged);
    connect(model(), &QAbstractListModel::modelReset, this, &FormGenListBagComposition::valueChanged);
    connect(model(), &QAbstractListModel::rowsInserted, this, &FormGenListBagComposition::valueChanged);
    connect(model(), &QAbstractListModel::rowsMoved, this, &FormGenListBagComposition::valueChanged);
    connect(model(), &QAbstractListModel::rowsRemoved, this, &FormGenListBagComposition::valueChanged);
    connect(model(), &QAbstractListModel::layoutChanged, this, &FormGenListBagComposition::valueChanged);
    connect(this, &FormGenElement::valueChanged, this, &FormGenListBagComposition::updateInputWidgets);

    connect(selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &FormGenListBagComposition::updateInputWidgets);
    connect(mHead->buttonDelete, &QPushButton::clicked, this, &FormGenListBagComposition::deleteCurrent);
    connect(mHead->buttonClear, &QPushButton::clicked, this, &FormGenListBagComposition::clearAll);
    connect(mHead->spinPosition, SIGNAL(valueChanged(int)), this, SLOT(moveCurrent()));
    connect(mHead->buttonCopy, &QPushButton::clicked, this, &FormGenListBagComposition::insertCopy);
    connect(mHead->buttonNew, &QPushButton::clicked, this, &FormGenListBagComposition::insertNew);

    auto layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(mHeadWidget, 0, 0, 1, 3);
    layout->addItem(new QSpacerItem(s_frameSubContentMargin, 1), 1, 0);

    frameWidget()->setLayout(layout);

    updateInputWidgets();
}

void FormGenListBagComposition::setContentElement(FormGenElement *element, const QString &label)
{
    if( mElementWrapper )
        mElementWrapper->deleteLater();

    mElement = element;

    if( mElement ) {
        connect(mElement, &FormGenElement::valueChanged, this, &FormGenListBagComposition::childValueChanged);

        if( mElement->frameWidget() ) {
            mElement->frameWidget()->setTitle(label);
            mElementWrapper = mElement;
        } else {
            mElementWrapper= new QWidget;
            auto *layout = new QFormLayout;
            layout->setContentsMargins(0, s_frameSubContentMargin, 0, 0);
            layout->addRow(label, element);
            mElementWrapper->setLayout(layout);
        }

        static_cast<QGridLayout*>(frameWidget()->layout())->addWidget(mElementWrapper, 1, 2);
    }

    updateInputWidgets();
}

FormGenElement *FormGenListBagComposition::contentElement() const
{
    return mElement;
}

FormGenListBagComposition::Mode FormGenListBagComposition::mode() const
{
    return mMode;
}

void FormGenListBagComposition::setCompareOperator(const FormGenBagModel::Compare &comparison)
{
    if( mMode == ListMode )
        return;

    mModel.bag->setCompareOperator(comparison);
}

QVariant FormGenListBagComposition::defaultValue() const
{
    return QVariantList();
}

QVariant FormGenListBagComposition::valueImpl() const
{
    QVariantList list;

    for( int i = 0; i < model()->rowCount(); ++i )
        list.append(model()->data(model()->index(i, 0), Qt::EditRole));

    return list;
}

QString FormGenListBagComposition::valueStringImpl() const
{
    QStringList list;

    for( int i = 0; i < model()->rowCount(); ++i )
        list.append(model()->data(model()->index(i, 0), Qt::DisplayRole).toString());

    return joinedValueStringList(list);
}

FormGenAcceptResult FormGenListBagComposition::acceptsValueImpl(const QVariant &val) const
{
    if( variantType(val) != QMetaType::QVariantList )
        return FormGenAcceptResult::reject({}, val);

    const QVariantList list = val.toList();

    if( list.size() > 0 && mElement == nullptr )
        return FormGenAcceptResult::reject({}, val);

    QStringList valueStrings;

    for( int i = 0; i < list.size(); ++i ) {
        auto elementAccepts = mElement->acceptsValue(list.at(i));
        if( ! elementAccepts.acceptable ) {
            QString path = QString::number(i);
            if( ! elementAccepts.path.isEmpty() )
                path += QString("/%1").arg(elementAccepts.path);
            return FormGenAcceptResult::reject(path, elementAccepts.value);
        }
        valueStrings.append(elementAccepts.valueString);
    }

    return FormGenAcceptResult::accept(val, joinedValueStringList(valueStrings));
}

void FormGenListBagComposition::setVaidatedValueImpl(const QVariant &val)
{
    const auto list = val.toList();

    if( list.size() == 0 && model()->rowCount() == 0 )
        return;

    if( mMode == ListMode ) {
        mModel.list->clear();
        for( const auto &v : list ) {
            const auto elementAccepts = mElement->acceptsValue(v);
            mModel.list->appendRow(elementAccepts.valueString, v);
        }
    } else {
        mModel.bag->clear();
        for( const auto &v : list ) {
            const auto elementAccepts = mElement->acceptsValue(v);
            mModel.bag->insertRow(elementAccepts.valueString, v);
        }
    }
}

void FormGenListBagComposition::updateInputWidgets()
{
    if( mUpdating )
        return;

    const int currentRow = selectionModel()->currentIndex().row();
    const int rowCount = model()->rowCount();

    if( ! isValueSet() || mElement == nullptr || currentRow < 0) {
        mUpdating = true;
        if( mElement )
            mElement->setValidatedValue(mElement->defaultValue());
        mHead->spinPosition->setValue(0);
        mUpdating = false;
    } else {
        mUpdating = true;
        mElement->setValidatedValue(model()->data(model()->index(currentRow, 0), Qt::EditRole));
        mHead->spinPosition->setMaximum(rowCount - 1);
        mHead->spinPosition->setValue(currentRow);
        mUpdating = false;
    }

    if( ! isValueSet() || mElement == nullptr ) {
        mHeadWidget->setEnabled(false);
        if( mElementWrapper )
            mElementWrapper->setEnabled(false);
    } else if( currentRow < 0 ) {
        mHeadWidget->setEnabled(true);
        mElementWrapper->setEnabled(false);
        mHead->buttonDelete->setEnabled(false);
        mHead->buttonClear->setEnabled(rowCount > 0);
        mHead->spinPosition->setEnabled(false);
        mHead->buttonCopy->setEnabled(false);
    } else {
        mHeadWidget->setEnabled(true);
        mElementWrapper->setEnabled(true);
        mHead->buttonDelete->setEnabled(true);
        mHead->buttonClear->setEnabled(true);
        mHead->spinPosition->setEnabled(true);
        mHead->buttonCopy->setEnabled(true);
    }
}

void FormGenListBagComposition::childValueChanged()
{
    if( mUpdating )
        return;

    const int currentRow = selectionModel()->currentIndex().row();
    Q_ASSERT(currentRow >= 0);

    mUpdating = true;
    if( mMode == ListMode ) {
        mModel.list->editRow(currentRow, mElement->valueString(), mElement->value());
    } else {
        mModel.bag->editRow(currentRow, mElement->valueString(), mElement->value());
    }
    mUpdating = false;
}

void FormGenListBagComposition::deleteCurrent()
{
    const int currentRow = selectionModel()->currentIndex().row();
    Q_ASSERT(currentRow >= 0);

    if( mMode == ListMode )
        mModel.list->removeRow(currentRow);
    else
        mModel.bag->removeRow(currentRow);
}

void FormGenListBagComposition::clearAll()
{
    if( mMode == ListMode )
        mModel.list->clear();
    else
        mModel.bag->clear();
}

void FormGenListBagComposition::moveCurrent()
{
    if( mUpdating )
        return;

    if( mMode != ListMode ) {
        qWarning("Move element in bag mode invalid");
        return;
    }

    const int currentRow = selectionModel()->currentIndex().row();
    Q_ASSERT(currentRow >= 0);

    int targetRow = mHead->spinPosition->value();

    mModel.list->moveRow(currentRow, targetRow);
}

void FormGenListBagComposition::insertCopy()
{
    const int currentRow = selectionModel()->currentIndex().row();
    Q_ASSERT(currentRow >= 0);

    const QString s = model()->data(model()->index(currentRow, 0), Qt::DisplayRole).toString();
    const QVariant v = model()->data(model()->index(currentRow, 0), Qt::EditRole);

    int newRow;
    if( mMode == ListMode ) {
        mModel.list->insertRow(currentRow, s, v);
        newRow = currentRow + 1;
    } else {
        newRow = mModel.bag->insertRow(s, v);
    }
    selectionModel()->setCurrentIndex(model()->index(newRow, 0), QItemSelectionModel::ClearAndSelect);
}

void FormGenListBagComposition::insertNew()
{
    const int currentRow = selectionModel()->currentIndex().row();

    Q_ASSERT(mElement);
    QVariant val = mElement->defaultValue();
    auto elementAccepts = mElement->acceptsValue(val);
    Q_ASSERT(elementAccepts.acceptable);

    int newRow;
    if( mMode == ListMode ) {
        newRow = currentRow + 1;
        mModel.list->insertRow(newRow, elementAccepts.valueString, val);
    } else {
        newRow = mModel.bag->insertRow(elementAccepts.valueString, val);
    }
    selectionModel()->setCurrentIndex(model()->index(newRow, 0), QItemSelectionModel::ClearAndSelect);
}

QAbstractItemModel *FormGenListBagComposition::model() const
{
    return mHead->listView->model();
}

QItemSelectionModel *FormGenListBagComposition::selectionModel() const
{
    return mHead->listView->selectionModel();
}
