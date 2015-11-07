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

#ifndef FORMGENWIDGETS_QT_COMPOSITIONWIDGETS_H
#define FORMGENWIDGETS_QT_COMPOSITIONWIDGETS_H

#include "formgencompositionmodels.h"
#include "formgenwidgetsbase.h"

#include "formgenwidgets_global.h"

class QAbstractItemModel;
class QComboBox;
class QFormLayout;
class QItemSelectionModel;
class QLabel;
class QStackedLayout;


namespace Ui {
class FormGenListBagHead;
}


class FORMGENWIDGETS_EXPORT FormGenRecordComposition : public FormGenFramedBase {
    Q_OBJECT

public:
    explicit FormGenRecordComposition(ElementType type = Required, QWidget * parent = nullptr);

    void addElement(const QString & tag, FormGenElement * element, const QString & label = QString());
    FormGenElement *element(const QString & tag) const;

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

private slots:
    void childValueChanged();

private:
    enum CompositionUpdateState {
        NotUpdatingState,
        UpdatingState,
        UpdatingWithChangeState
    };

    QFormLayout *mLayout;
    QVector<CompositionElement> mElements;
    QHash<QString, int> mTagIndexMap;
    CompositionUpdateState mUpdating;
};


class FormGenChoiceCompositionContainer;
class FORMGENWIDGETS_EXPORT FormGenChoiceComposition : public FormGenFramedBase {
    Q_OBJECT

public:
    enum Style {
        RadioStyle, ListStyle, ComboBoxStyle
    };

    explicit FormGenChoiceComposition(ElementType type = Required, Style style = RadioStyle, QWidget * parent = nullptr);

    void addElement(const QString & tag, FormGenElement * element, const QString & label = QString());
    FormGenElement *element(const QString & tag) const;

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

private:
    QComboBox *mComboBox;
    QWidget *mElementContainer;
    QStackedLayout *mElementLayout;
    FormGenChoiceCompositionContainer *mContainer;
    QVector<CompositionElement> mElements;
    QHash<QString, int> mTagIndexMap;
};


class FORMGENWIDGETS_EXPORT FormGenListBagComposition : public FormGenFramedBase {
    Q_OBJECT

public:
    enum Mode {
        ListMode, BagMode
    };

    /* TODO
    enum PositionIndex {
        ZeroBased, OneBased
    };*/

    explicit FormGenListBagComposition(Mode mode, ElementType type = Required, QWidget * parent = nullptr);

    void setContentElement(FormGenElement * element, const QString & label = QString());
    FormGenElement *contentElement() const;

    Mode mode() const;

    void setCompareOperator(const FormGenBagModel::Compare &comparison);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

protected slots:
    void updateInputWidgets();

private slots:
    void childValueChanged();

    void deleteCurrent();
    void clearAll();
    void moveCurrent();
    void insertCopy();
    void insertNew();

private:
    QAbstractItemModel *model() const;
    QItemSelectionModel *selectionModel() const;

    Mode mMode;
    union {
        FormGenListModel * list;
        FormGenBagModel * bag;
    } mModel;
    Ui::FormGenListBagHead * const mHead;
    QWidget * const mHeadWidget;
    FormGenElement * mElement;
    QWidget * mElementWrapper;
    bool mUpdating;
};

#endif // FORMGENWIDGETS_QT_COMPOSITIONWIDGETS_H
