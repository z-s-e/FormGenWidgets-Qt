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

#include "formgenregularwidgets_p.h"

#include <QMimeData>
#include <QPainter>
#include <QTextEdit>


static const QString s_mimeUriList = QStringLiteral("text/uri-list"); // TODO: add proper char set parameter
static const QString s_mimePlainText = QStringLiteral("text/plain"); // TODO: as above
static const QString s_mimeLineBreak = QStringLiteral("\r\n");


QSizeF FormGenFormatStringTextObject::intrinsicSize(QTextDocument *doc, int, const QTextFormat &format)
{
    QString s = format.property(VoidTag).toString();

    QFont font(doc->defaultFont());
    font.setItalic(true);
    QFontMetrics fm(font);

    return fm.size(Qt::TextSingleLine, s) + QSize(4, 2);
}

void FormGenFormatStringTextObject::drawObject(QPainter *painter,
                                               const QRectF &rect,
                                               QTextDocument *doc,
                                               int posInDocument,
                                               const QTextFormat &format)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(mTextEdit->palette().color(QPalette::Window));
    painter->drawRect(rect.adjusted(1, 1, -1, 0));

    const QString s = format.property(VoidTag).toString();

    QFont font(doc->defaultFont());
    font.setItalic(true);
    painter->setFont(font);

    if( posInDocument >= mTextEdit->textCursor().selectionStart()
            && posInDocument < mTextEdit->textCursor().selectionEnd())
        painter->setPen(mTextEdit->palette().color(QPalette::HighlightedText));
    else
        painter->setPen(mTextEdit->palette().color(QPalette::WindowText));

    painter->drawText(rect.bottomLeft() + QPoint(3, 0), s);
}



FormGenFileUrlListModel::FormGenFileUrlListModel(QObject *parent)
    : QAbstractListModel(parent)
    , mEmptyUrlColor(Qt::lightGray)
{
}

QVariant FormGenFileUrlListModel::data(const QModelIndex &index, int role) const
{
    if( index.row() < 0 || index.row() >= mItems.size() )
        return {};

    const QString &s = mItems.at(index.row());

    switch( role ) {
    case Qt::DisplayRole:
        return s.isEmpty() ? tr("< empty url >") : s;
    case Qt::EditRole:
        return s;
    case Qt::ForegroundRole:
        return s.isEmpty() ? QBrush(mEmptyUrlColor) : QVariant();
    }

    return {};
}

QVariant FormGenFileUrlListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return tr("Url");

    return {};
}

bool FormGenFileUrlListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( role != Qt::EditRole )
        return true;

    if( index.row() < 0 || index.row() >= mItems.size() )
        return false;

    if( value.type() != QVariant::String )
        return false;

    mItems[index.row()] = value.toString();
    emit dataChanged(index, index);
    return true;
}

int FormGenFileUrlListModel::rowCount(const QModelIndex &parent) const
{
    if( parent.isValid() )
        return 0;

    return mItems.size();
}

Qt::ItemFlags FormGenFileUrlListModel::flags(const QModelIndex &index) const
{
    if( index.isValid() )
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

bool FormGenFileUrlListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if( parent.isValid() )
        return false;

    beginInsertRows(parent, row, row + count - 1);
    for( int i = 0; i < count; ++i )
        mItems.insert(row, {});
    endInsertRows();

    return true;
}

bool FormGenFileUrlListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if( parent.isValid() )
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for( int i = 0; i < count; ++i )
        mItems.removeAt(row);
    endRemoveRows();

    return true;
}

void FormGenFileUrlListModel::clear()
{
    beginResetModel();
    mItems.clear();
    endResetModel();
}

void FormGenFileUrlListModel::removeUrl(int row)
{
    if( row < 0 || row >= mItems.size() )
        return;

    beginRemoveRows(QModelIndex(), row, row);
    mItems.removeAt(row);
    endRemoveRows();
}

void FormGenFileUrlListModel::removeUrls(const QModelIndexList &rows)
{
    QMap<int, bool> rowIdx;
    for( const auto r : rows )
        rowIdx[-r.row()] = true;

    for( auto it = rowIdx.cbegin(); it != rowIdx.cend(); ++it )
        removeUrl(-it.key());
}

void FormGenFileUrlListModel::insertUrl(const QUrl &url, int row)
{
    if( row > mItems.size() )
        return;
    row = qMax(row, 0);

    beginInsertRows(QModelIndex(), row, row);
    mItems.insert(row, url.toString());
    endInsertRows();
}

void FormGenFileUrlListModel::insertUrls(const QList<QUrl> &urls, int row)
{
    if( row > mItems.size() || urls.isEmpty() )
        return;
    row = qMax(row, 0);

    beginInsertRows(QModelIndex(), row, row + urls.size() - 1);
    for( int i = 0; i < urls.size(); ++i )
        mItems.insert(row + i, urls.at(i).toString());
    endInsertRows();
}

void FormGenFileUrlListModel::moveUrl(int sourceRow, int targetRow)
{
    if (sourceRow == targetRow || sourceRow < 0 || targetRow < 0)
        return;

    beginMoveRows(QModelIndex(), sourceRow, sourceRow,
                  QModelIndex(), targetRow > sourceRow ? targetRow + 1 : targetRow);
    const QString tmp = mItems.takeAt(sourceRow);
    mItems.insert(targetRow, tmp);
    endMoveRows();
}

void FormGenFileUrlListModel::resetData(const QVariantList &list)
{
    beginResetModel();
    mItems.clear();
    for( const auto & v : list )
        mItems.append(v.toString());
    endResetModel();
}

void FormGenFileUrlListModel::setEmptyUrlColor(const QColor &color)
{
    if( mEmptyUrlColor == color )
        return;

    beginResetModel();
    mEmptyUrlColor = color;
    endResetModel();
}

QString FormGenFileUrlListModel::urlAt(int row) const
{
    if( row < 0 || row >= mItems.size() )
        return {};

    return mItems.at(row);
}

int FormGenFileUrlListModel::urlCount() const
{
    return mItems.size();
}

bool FormGenFileUrlListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if( parent.isValid() || column > 0 || row > mItems.size() )
        return false;

    if( row < 0 )
        row = mItems.size();

    if( data->hasFormat( mimeTypes().first()) )
        return QAbstractListModel::dropMimeData(data, action, row, column, parent);

    if( data->hasFormat(s_mimeUriList) || data->hasFormat(s_mimePlainText) ) {
        QString list;
        if( data->hasFormat(s_mimeUriList) )
            list = QString::fromUtf8(data->data(s_mimeUriList)); // TODO: correct char set handling
        else
            list = QString::fromUtf8(data->data(s_mimePlainText)); // TODO: correct char set handling

        const QStringList l = list.split(s_mimeLineBreak);
        for( int i = 0; i < l.size(); ++i )
            insertUrl(l.at(i), row + i);
        return true;

    }

    return false;
}

QStringList FormGenFileUrlListModel::mimeTypes() const
{
    static const QStringList mime( { QAbstractListModel::mimeTypes().first(), s_mimeUriList, s_mimePlainText } );
    return mime;
}

QMimeData *FormGenFileUrlListModel::mimeData(const QModelIndexList &indexes) const
{
    if( indexes.isEmpty() )
        return nullptr;

    QMimeData *result = QAbstractListModel::mimeData(indexes);
    QString uris;
    for( const auto &i : indexes )
        uris.append( QString("%1%2").arg(urlAt(i.row()), s_mimeLineBreak));

    const auto data = uris.toUtf8(); // TODO: correct char set handling
    result->setData(s_mimeUriList, data);
    result->setData(s_mimePlainText, data);
    return result;
}

Qt::DropActions FormGenFileUrlListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
