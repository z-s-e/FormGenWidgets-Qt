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

#include "formgencompositionmodels.h"

FormGenListModel::FormGenListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant FormGenListModel::data(const QModelIndex &index, int role) const
{
    if( index.row() < 0 || index.row() >= mDataItems.size() )
        return {};

    switch( role ) {
    case Qt::DisplayRole:
        return mDisplayItems.at(index.row());
    case Qt::EditRole:
        return mDataItems.at(index.row());
    }

    return {};
}

bool FormGenListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( index.row() < 0 || index.row() >= mDataItems.size() )
        return false;

    if( role == Qt::EditRole ) {
        mDataItems[index.row()] = value;
        emit dataChanged(index, index);
        return true;
    }

    if( role == Qt::DisplayRole ) {
        if( value.type() != QVariant::String )
            return false;

        mDisplayItems[index.row()] = value.toString();
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

int FormGenListModel::rowCount(const QModelIndex &parent) const
{
    if( parent.isValid() )
        return 0;

    return mDataItems.size();
}

void FormGenListModel::editRow(int row, const QString &newDisplay, const QVariant &newData)
{
    if( row < 0 || row >= mDataItems.size() )
        return;

    mDataItems[row] = newData;
    mDisplayItems[row] = newDisplay;
    emit dataChanged(index(row), index(row));
}

void FormGenListModel::appendRow(const QString &display, const QVariant &data)
{
    const int row = mDataItems.size();
    beginInsertRows(QModelIndex(), row, row);
    mDataItems.append(data);
    mDisplayItems.append(display);
    endInsertRows();
}

void FormGenListModel::insertRow(int row, const QString &display, const QVariant &data)
{
    if( row < 0 || row > mDataItems.size() )
        return;

    beginInsertRows(QModelIndex(), row, row);
    mDataItems.insert(row, data);
    mDisplayItems.insert(row, display);
    endInsertRows();
}

void FormGenListModel::removeRow(int row)
{
    if( row < 0 || row >= mDataItems.size() )
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mDataItems.removeAt(row);
    mDisplayItems.removeAt(row);
    endRemoveRows();
}

void FormGenListModel::moveRow(int sourceRow, int targetRow)
{
    if (sourceRow == targetRow || sourceRow < 0 || targetRow < 0)
        return;

    beginMoveRows(QModelIndex(), sourceRow, sourceRow,
                  QModelIndex(), targetRow > sourceRow ? targetRow + 1 : targetRow);
    const QVariant tmpData = mDataItems.takeAt(sourceRow);
    const QString tmpDisplay = mDisplayItems.takeAt(sourceRow);
    mDataItems.insert(targetRow, tmpData);
    mDisplayItems.insert(targetRow, tmpDisplay);
    endMoveRows();
}

void FormGenListModel::clear()
{
    beginResetModel();
    mDataItems.clear();
    mDisplayItems.clear();
    endResetModel();
}


FormGenBagModel::FormGenBagModel(QObject *parent)
    : QAbstractListModel(parent)
    , mItems(Compare([] (const DataElement &lhs, const DataElement &rhs) {
                return QString::localeAwareCompare(lhs.first, rhs.first) < 0;
             }))
{
}

QVariant FormGenBagModel::data(const QModelIndex &index, int role) const
{
    if( index.row() < 0 || index.row() >= mItems.size() )
        return {};

    switch( role ) {
    case Qt::DisplayRole:
        return mItems.at(index.row()).first;
    case Qt::EditRole:
        return mItems.at(index.row()).second;
    }

    return {};
}

int FormGenBagModel::rowCount(const QModelIndex &parent) const
{
    if( parent.isValid() )
        return 0;

    return mItems.size();
}

int FormGenBagModel::insertRow(const QString &display, const QVariant &data)
{
    const auto pair = QPair<QString, QVariant>(display, data);
    const int row = mItems.insertPosition(pair);
    beginInsertRows(QModelIndex(), row, row);
    mItems.insert(pair, sorted_sequence::InsertLast, row);
    endInsertRows();
    return row;
}

int FormGenBagModel::editRow(int row, const QString &newDisplay, const QVariant &newData)
{
    if( row < 0 || row >= mItems.size() )
        return -1;

    const auto pair = QPair<QString, QVariant>(newDisplay, newData);
    const int newRow = mItems.insertPosition(pair);
    const int newRowAfterRemove = newRow > row ? newRow - 1 : newRow;
    const bool needMove = newRowAfterRemove != row;

    if( needMove )
        beginMoveRows(QModelIndex(), row, row, QModelIndex(), newRow);
    mItems.change(row, pair, sorted_sequence::InsertLast, newRow);
    if( needMove )
        endMoveRows();
    emit dataChanged(index(newRow), index(newRow));
    return newRowAfterRemove;
}

void FormGenBagModel::removeRow(int row)
{
    if( row < 0 || row >= mItems.size() )
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mItems.removeAt(row);
    endRemoveRows();
}

void FormGenBagModel::clear()
{
    beginResetModel();
    mItems.clear();
    endResetModel();
}

void FormGenBagModel::setCompareOperator(const Compare &comparison)
{
    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

    QHash<int, int> persistentRows;
    {
        auto tmp = persistentIndexList();
        for( const auto &idx: tmp )
            persistentRows.insert(idx.row(), -1);
    }

    mItems.setCompareOperatorGetReorderMap(comparison, &persistentRows);

    {
        QModelIndexList oldList, newList;
        oldList.reserve(persistentRows.size());
        newList.reserve(persistentRows.size());
        for( auto it = persistentRows.cbegin(); it != persistentRows.cend(); ++it ) {
            //changePersistentIndex(index(it.key()), index(it.value()));
            oldList.append(index(it.key()));
            newList.append(index(it.value()));
        }
        changePersistentIndexList(oldList, newList);
    }

    emit layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
}
