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

#ifndef FORMGENWIDGETS_QT_REGULARWIDGETS_H
#define FORMGENWIDGETS_QT_REGULARWIDGETS_H

#include "formgenwidgetsbase.h"

#include <QSet>

#include "formgenwidgets_global.h"

class QComboBox;
class QDateEdit;
class QDateTimeEdit;
class QLineEdit;
class QMenu;
class QPushButton;
class QSlider;
class QSpinBox;
class QTextEdit;
class QTimeEdit;
class QToolButton;

namespace Ui {
class FormGenFileListHead;
}


class FORMGENWIDGETS_EXPORT FormGenVoidWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenVoidWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

    static QVariant voidValue();

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;
};


class FORMGENWIDGETS_EXPORT FormGenBoolWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenBoolWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private:
    QComboBox * mValue;
};


class FORMGENWIDGETS_EXPORT FormGenEnumWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenEnumWidget(ElementType type = Required, QWidget * parent = nullptr);

    void addEnumValue(const QString & tag, const QString & label = QString());

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private:
    QComboBox * mValue;
    QStringList mTags;
};


class FORMGENWIDGETS_EXPORT FormGenIntWidget : public FormGenUnframedBase {
    Q_OBJECT
    Q_PROPERTY(InputStyle inputStyle READ inputStyle WRITE setInputStyle NOTIFY inputStyleChanged)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
    Q_ENUMS(InputStyle)

public:
    enum InputStyle {
        Spinner, Slider, SpinnerSlider, Plain
    };

    explicit FormGenIntWidget(InputStyle inputStyle = Spinner, ElementType type = Required, QWidget * parent = nullptr);

    InputStyle inputStyle() const;
    void setInputStyle(InputStyle s);

    int minimum() const;
    void setMinimum(int min);

    int maximum() const;
    void setMaximum(int max);

    QVariant defaultValue() const override;

signals:
    void inputStyleChanged();
    void minimumChanged();
    void maximumChanged();

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private slots:
    void checkNewValue();

private:
    void setIntValue(int val);

    void setupStyle();
    void setSpinBox(bool set);
    void setSlider(bool set);
    void setPlain(bool set);

    InputStyle mStyle;
    int mMinimum;
    int mMaximum;
    int mValue;
    QSpinBox * mSpinBox;
    QSlider * mSlider;
    QLineEdit * mPlain;
};


class FORMGENWIDGETS_EXPORT FormGenFloatWidget : public FormGenUnframedBase {
    Q_OBJECT
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)

public:
    explicit FormGenFloatWidget(ElementType type = Required, QWidget * parent = nullptr);

    double minimum() const;
    void setMinimum(double min);

    double maximum() const;
    void setMaximum(double max);

    QVariant defaultValue() const override;

signals:
    void minimumChanged();
    void maximumChanged();

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private slots:
    void checkNewValue();

private:
    void setDoubleValue(double val);

    double mMinimum;
    double mMaximum;
    double mValue;
    QLineEdit * mEdit;
};


class FORMGENWIDGETS_EXPORT FormGenDateWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenDateWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private:
    QDateEdit * mEdit;
};


class FORMGENWIDGETS_EXPORT FormGenTimeWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenTimeWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private:
    QTimeEdit * mEdit;
};


class FORMGENWIDGETS_EXPORT FormGenDateTimeWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenDateTimeWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private:
    QDateTimeEdit * mEdit;
};


class FORMGENWIDGETS_EXPORT FormGenColorWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenColorWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private slots:
    void showColorChooser();

private:
    QColor mValue;
    QPushButton * mButton;
};


class FORMGENWIDGETS_EXPORT FormGenTextWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenTextWidget(ElementType type = Required, QWidget * parent = nullptr);

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private:
    QLineEdit * mEdit;
};


enum FormGenFileUriChooseOptions {
    FormGenUriChooseFile = 1,
    FormGenUriChooseDirectpory = 2,
    FormGenUriChooseFileOrDirectory = FormGenUriChooseFile | FormGenUriChooseDirectpory
};

class FORMGENWIDGETS_EXPORT FormGenFileUrlWidget : public FormGenTextWidget {
    Q_OBJECT

public:
    explicit FormGenFileUrlWidget(ElementType type = Required, QWidget * parent = nullptr);

    void setMimeTypes(const QStringList &mimeList);
    QStringList mimeTypes() const;

    void setChooseOptions(FormGenFileUriChooseOptions opt);
    FormGenFileUriChooseOptions chooseOptions() const;

protected:
    void updateInputWidgets() override;

private slots:
    void showFileChooser();
    void showDirChooser();
    void showMenu();

private:
    FormGenFileUriChooseOptions mChooseOptions;
    QToolButton * mFileChooser;
    QMenu * mChooseMenu;
    QStringList mMimeTypes;

    friend class FormGenFileUrlList;
};


class FormGenFileUrlListModel;
class FORMGENWIDGETS_EXPORT FormGenFileUrlList : public FormGenFramedBase {
    Q_OBJECT

public:
    explicit FormGenFileUrlList(ElementType type = Required, QWidget * parent = nullptr);

    void setMimeTypes(const QStringList &mimeList);
    QStringList mimeTypes() const;

    void setChooseOptions(FormGenFileUriChooseOptions opt);
    FormGenFileUriChooseOptions chooseOptions() const;

    QVariant defaultValue() const override;

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

protected slots:
    void updateInputWidgets();

private slots:
    void removeSelection();
    void clearAll();
    void moveCurrent();
    void insertEmpty();
    void insertDir();
    void insertFiles();

private:
    FormGenFileUriChooseOptions mChooseOptions;
    QStringList mMimeTypes;
    FormGenFileUrlListModel * const mModel;
    Ui::FormGenFileListHead * const mHead;
    QWidget * const mHeadWidget;
};


class FORMGENWIDGETS_EXPORT FormGenFormatStringWidget : public FormGenUnframedBase {
    Q_OBJECT

public:
    explicit FormGenFormatStringWidget(ElementType type = Required, QWidget * parent = nullptr);

    void addVoidElement(const QString & tag);

    QVariant defaultValue() const override;

    static QString textTag();

protected:
    QVariant valueImpl() const override;
    QString valueStringImpl() const override;
    FormGenAcceptResult acceptsValueImpl(const QVariant &val) const override;
    void setVaidatedValueImpl(const QVariant &val) override;

    void updateInputWidgets() override;

private slots:
    void insertVoidElement();

private:
    void insertVoidElement(const QString &voidTag);

    QSet<QString> mVoidTags;
    QTextEdit * const mTextEdit;
    QComboBox * const mInsertMenu;
};

#endif // FORMGENWIDGETS_QT_REGULARWIDGETS_H
