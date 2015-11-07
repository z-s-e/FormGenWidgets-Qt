#include "formgenwidgets-qt.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto * test = new FormGenChoiceComposition(FormGenElement::Optional);
    test->addElement("void", new FormGenVoidWidget);
    test->addElement("bool", new FormGenBoolWidget);
    test->addElement("int", new FormGenIntWidget(FormGenIntWidget::SpinnerSlider));
    test->addElement("float", new FormGenFloatWidget(FormGenElement::Optional));
    {
        auto * f = new FormGenFileUrlWidget(FormGenElement::Optional);
        f->setChooseOptions(FormGenUriChooseFileOrDirectory);
        test->addElement("url", f);

    }
    test->addElement("date", new FormGenDateWidget);
    test->addElement("time", new FormGenTimeWidget);
    test->addElement("datetime", new FormGenDateTimeWidget);
    test->addElement("color", new FormGenColorWidget);
    {
        auto * fmtstr = new FormGenFormatStringWidget(FormGenElement::Optional);
        fmtstr->addVoidElement("foo");
        fmtstr->addVoidElement("bar");
        test->addElement("fmtstr", fmtstr);
    }
    {
        auto * files = new FormGenFileUrlList;
        //files->setMimeTypes( {"inode/directory"} );
        test->addElement("files", files);
    }
    {
        auto * list = new FormGenListBagComposition(FormGenListBagComposition::BagMode);
        list->setContentElement(new FormGenIntWidget);
        test->addElement("list", list);
        list->setCompareOperator(FormGenBagModel::Compare([] (const FormGenBagModel::DataElement& x, const FormGenBagModel::DataElement& y) {
            return x.second.toInt() < y.second.toInt();
        }));
    }
    test->connect(test, &FormGenElement::valueChanged, [&test] { qDebug() << test->valueString(); } );

    test->show();

    return a.exec();
}
