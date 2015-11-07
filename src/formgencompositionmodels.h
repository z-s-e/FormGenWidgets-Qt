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

#ifndef FORMGENWIDGETS_QT_COMPOSITIONMODELS_H
#define FORMGENWIDGETS_QT_COMPOSITIONMODELS_H

#include "sorted_sequence.h"

#include <QAbstractListModel>
#include <QPair>

#include "formgenwidgets_global.h"


class FORMGENWIDGETS_EXPORT FormGenListModel : public QAbstractListModel {
    Q_OBJECT

public:
    FormGenListModel(QObject * parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent) const override;

    void editRow(int row, const QString &newDisplay, const QVariant &newData);
    void appendRow(const QString &display, const QVariant &data);
    void insertRow(int row, const QString &display, const QVariant &data);
    void removeRow(int row);
    void moveRow(int sourceRow, int targetRow);
    void clear();

private:
    QStringList mDisplayItems;
    QVariantList mDataItems;
};


class FORMGENWIDGETS_EXPORT FormGenBagModel : public QAbstractListModel {
    Q_OBJECT

public:
    typedef QPair<QString, QVariant> DataElement;
    typedef sorted_sequence::lambda_compare<DataElement> Compare;

    FormGenBagModel(QObject * parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent) const override;

    int insertRow(const QString &display, const QVariant &data);
    int editRow(int row, const QString &newDisplay, const QVariant &newData);
    void removeRow(int row);
    void clear();

    void setCompareOperator(const Compare &comparison);

private:
    sorted_sequence::adaptor< QVector<DataElement>, Compare > mItems;
};

#endif // FORMGENWIDGETS_QT_COMPOSITIONMODELS_H
