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

#ifndef FORMGENWIDGETS_QT_REGULARWIDGETS_P_H
#define FORMGENWIDGETS_QT_REGULARWIDGETS_P_H

#include <QAbstractListModel>
#include <QTextObjectInterface>

class QPainter;
class QRectF;
class QSizeF;
class QTextDocument;
class QTextEdit;
class QTextFormat;


class FormGenFormatStringTextObject : public QObject, public QTextObjectInterface {
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    enum { FormGenVoidTextFormat = QTextFormat::UserObject + 1 };
    enum FormGenVoidProperties { VoidTag = 1 };

    explicit FormGenFormatStringTextObject(QTextEdit *edit, QObject * parent = nullptr)
        : QObject(parent)
        , mTextEdit(edit)
    {}

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override;
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override;

private:
    QTextEdit *mTextEdit;
};


class FormGenFileUrlListModel : public QAbstractListModel {
    Q_OBJECT

public:
    FormGenFileUrlListModel(QObject * parent = 0);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;

    void clear();
    void removeUrl(int row);
    void removeUrls(const QModelIndexList &rows);
    void insertUrl(const QUrl &url, int row);
    void insertUrls(const QList<QUrl> &urls, int row);
    void moveUrl(int sourceRow, int targetRow);
    void resetData(const QVariantList &list);

    void setEmptyUrlColor(const QColor &color);

    QString urlAt(int row) const;

    int urlCount() const;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList & indexes) const override;
    Qt::DropActions supportedDropActions() const override;

private:
    QStringList mItems;
    QColor mEmptyUrlColor;
};

#endif // FORMGENWIDGETS_QT_REGULARWIDGETS_P_H
