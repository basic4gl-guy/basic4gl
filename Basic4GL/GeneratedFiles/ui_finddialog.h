/********************************************************************************
** Form generated from reading UI file 'finddialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FINDDIALOG_H
#define UI_FINDDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FindDialog
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *gridLayoutWidget;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *findEdit;
    QLabel *replaceLabel;
    QLineEdit *replaceEdit;
    QCheckBox *matchCaseCheckbox;
    QCheckBox *wholeWordsCheckbox;

    void setupUi(QDialog *FindDialog)
    {
        if (FindDialog->objectName().isEmpty())
            FindDialog->setObjectName(QStringLiteral("FindDialog"));
        FindDialog->resize(381, 129);
        buttonBox = new QDialogButtonBox(FindDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(290, 10, 81, 221));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
        gridLayoutWidget = new QWidget(FindDialog);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 10, 271, 111));
        formLayout = new QFormLayout(gridLayoutWidget);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        findEdit = new QLineEdit(gridLayoutWidget);
        findEdit->setObjectName(QStringLiteral("findEdit"));

        formLayout->setWidget(0, QFormLayout::FieldRole, findEdit);

        replaceLabel = new QLabel(gridLayoutWidget);
        replaceLabel->setObjectName(QStringLiteral("replaceLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, replaceLabel);

        replaceEdit = new QLineEdit(gridLayoutWidget);
        replaceEdit->setObjectName(QStringLiteral("replaceEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, replaceEdit);

        matchCaseCheckbox = new QCheckBox(gridLayoutWidget);
        matchCaseCheckbox->setObjectName(QStringLiteral("matchCaseCheckbox"));

        formLayout->setWidget(2, QFormLayout::FieldRole, matchCaseCheckbox);

        wholeWordsCheckbox = new QCheckBox(gridLayoutWidget);
        wholeWordsCheckbox->setObjectName(QStringLiteral("wholeWordsCheckbox"));

        formLayout->setWidget(3, QFormLayout::FieldRole, wholeWordsCheckbox);


        retranslateUi(FindDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FindDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FindDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FindDialog);
    } // setupUi

    void retranslateUi(QDialog *FindDialog)
    {
        FindDialog->setWindowTitle(QApplication::translate("FindDialog", "Dialog", 0));
        label->setText(QApplication::translate("FindDialog", "Search for", 0));
        replaceLabel->setText(QApplication::translate("FindDialog", "Replace with", 0));
        matchCaseCheckbox->setText(QApplication::translate("FindDialog", "Match case", 0));
        wholeWordsCheckbox->setText(QApplication::translate("FindDialog", "Whole words only", 0));
    } // retranslateUi

};

namespace Ui {
    class FindDialog: public Ui_FindDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINDDIALOG_H
