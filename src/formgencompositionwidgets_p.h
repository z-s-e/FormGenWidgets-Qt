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

#ifndef FORMGENWIDGETS_QT_COMPOSITIONWIDGETS_P_H
#define FORMGENWIDGETS_QT_COMPOSITIONWIDGETS_P_H

#include <QWidget>


class FormGenElement;
class FormGenChoiceCompositionContainer : public QWidget {
    Q_OBJECT

public:
    using QWidget::QWidget;

    virtual void addElement(const QString & label, FormGenElement * element) = 0;

    virtual int currentIndex() const = 0;

public slots:
    virtual void setCurrentIndex(int idx) = 0;

signals:
    void currentIndexChanged(int idx);
};

class QComboBox;
class QGridLayout;
class QListView;
class QStackedLayout;
class QStringListModel;
class FormGenChoiceCompositionComboListContainer : public FormGenChoiceCompositionContainer {
    Q_OBJECT

public:
    FormGenChoiceCompositionComboListContainer(bool listMode, QWidget * parent = nullptr);

    void addElement(const QString &label, FormGenElement *element);
    int currentIndex() const;

public slots:
    void setCurrentIndex(int idx);

private:
    QStringList mLabels;
    QComboBox *mComboBox;
    QListView *mListView;
    QStringListModel *mListModel;
    QWidget *mElementContainer;
    QStackedLayout *mElementLayout;
};

class QButtonGroup;
class QFormLayout;
class QRadioButton;
class QSignalMapper;
class FormGenChoiceCompositionRadioContainer : public FormGenChoiceCompositionContainer {
    Q_OBJECT

public:
    FormGenChoiceCompositionRadioContainer(QWidget * parent = nullptr);

    void addElement(const QString &label, FormGenElement *element);
    int currentIndex() const;

public slots:
    void setCurrentIndex(int idx);

    void radioToggled(int i);

private:
    struct ElementContainer {
        ElementContainer(QRadioButton *b = nullptr, FormGenElement *e = nullptr) : button(b), element(e) {}
        ElementContainer(const ElementContainer &other) = default;
        ElementContainer &operator =(const ElementContainer &other) = default;

        QRadioButton *button;
        FormGenElement *element;
    };

    int mCurrentIndex;
    QVector<ElementContainer> mContainerList;
    QButtonGroup *mGroup;
    QFormLayout *mLayout;
    QSignalMapper *mMapper;
};


#endif // FORMGENWIDGETS_QT_COMPOSITIONWIDGETS_P_H
