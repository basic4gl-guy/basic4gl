/********************************************************************************
** Form generated from reading UI file 'argumentsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ARGUMENTSDIALOG_H
#define UI_ARGUMENTSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ArgumentsDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *argumentsEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ArgumentsDialog)
    {
        if (ArgumentsDialog->objectName().isEmpty())
            ArgumentsDialog->setObjectName(QStringLiteral("ArgumentsDialog"));
        ArgumentsDialog->resize(444, 87);
        verticalLayout_2 = new QVBoxLayout(ArgumentsDialog);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label = new QLabel(ArgumentsDialog);
        label->setObjectName(QStringLiteral("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        argumentsEdit = new QLineEdit(ArgumentsDialog);
        argumentsEdit->setObjectName(QStringLiteral("argumentsEdit"));

        formLayout->setWidget(0, QFormLayout::FieldRole, argumentsEdit);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(ArgumentsDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(ArgumentsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ArgumentsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ArgumentsDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ArgumentsDialog);
    } // setupUi

    void retranslateUi(QDialog *ArgumentsDialog)
    {
        ArgumentsDialog->setWindowTitle(QApplication::translate("ArgumentsDialog", "Set BASIC program arguments", 0));
        label->setText(QApplication::translate("ArgumentsDialog", "Program arguments", 0));
    } // retranslateUi

};

namespace Ui {
    class ArgumentsDialog: public Ui_ArgumentsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ARGUMENTSDIALOG_H
