/********************************************************************************
** Form generated from reading UI file 'sourcefileform.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SOURCEFILEFORM_H
#define UI_SOURCEFILEFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "basic4gltextedit.h"

QT_BEGIN_NAMESPACE

class Ui_SourceFileForm
{
public:
    QVBoxLayout *verticalLayout;
    Basic4GLTextEdit *sourceTextEdit;

    void setupUi(QWidget *SourceFileForm)
    {
        if (SourceFileForm->objectName().isEmpty())
            SourceFileForm->setObjectName(QStringLiteral("SourceFileForm"));
        SourceFileForm->resize(400, 300);
        verticalLayout = new QVBoxLayout(SourceFileForm);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        sourceTextEdit = new Basic4GLTextEdit(SourceFileForm);
        sourceTextEdit->setObjectName(QStringLiteral("sourceTextEdit"));
        QFont font;
        font.setFamily(QStringLiteral("Consolas"));
        font.setPointSize(10);
        sourceTextEdit->setFont(font);
        sourceTextEdit->setContextMenuPolicy(Qt::ActionsContextMenu);
        sourceTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        verticalLayout->addWidget(sourceTextEdit);


        retranslateUi(SourceFileForm);

        QMetaObject::connectSlotsByName(SourceFileForm);
    } // setupUi

    void retranslateUi(QWidget *SourceFileForm)
    {
        SourceFileForm->setWindowTitle(QApplication::translate("SourceFileForm", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class SourceFileForm: public Ui_SourceFileForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SOURCEFILEFORM_H
