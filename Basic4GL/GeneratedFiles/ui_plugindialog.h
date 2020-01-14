/********************************************************************************
** Form generated from reading UI file 'plugindialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLUGINDIALOG_H
#define UI_PLUGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PluginDialog
{
public:
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QSplitter *splitter;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_3;
    QListWidget *pluginList;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_4;
    QTabWidget *tabWidget;
    QWidget *functionsTab;
    QVBoxLayout *verticalLayout_7;
    QListWidget *functionsList;
    QWidget *constantsTab;
    QVBoxLayout *verticalLayout_2;
    QListWidget *constantsList;
    QWidget *structureTab;
    QVBoxLayout *verticalLayout_6;
    QPlainTextEdit *structuresTextEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *PluginDialog)
    {
        if (PluginDialog->objectName().isEmpty())
            PluginDialog->setObjectName(QStringLiteral("PluginDialog"));
        PluginDialog->setWindowModality(Qt::NonModal);
        PluginDialog->resize(727, 509);
        verticalLayout_5 = new QVBoxLayout(PluginDialog);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(PluginDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setMaximumSize(QSize(16777215, 16));

        verticalLayout->addWidget(label);

        splitter = new QSplitter(PluginDialog);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        verticalLayoutWidget = new QWidget(splitter);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        pluginList = new QListWidget(verticalLayoutWidget);
        pluginList->setObjectName(QStringLiteral("pluginList"));

        verticalLayout_3->addWidget(pluginList);

        splitter->addWidget(verticalLayoutWidget);
        verticalLayoutWidget_2 = new QWidget(splitter);
        verticalLayoutWidget_2->setObjectName(QStringLiteral("verticalLayoutWidget_2"));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(verticalLayoutWidget_2);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        functionsTab = new QWidget();
        functionsTab->setObjectName(QStringLiteral("functionsTab"));
        verticalLayout_7 = new QVBoxLayout(functionsTab);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        functionsList = new QListWidget(functionsTab);
        functionsList->setObjectName(QStringLiteral("functionsList"));

        verticalLayout_7->addWidget(functionsList);

        tabWidget->addTab(functionsTab, QString());
        constantsTab = new QWidget();
        constantsTab->setObjectName(QStringLiteral("constantsTab"));
        verticalLayout_2 = new QVBoxLayout(constantsTab);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        constantsList = new QListWidget(constantsTab);
        constantsList->setObjectName(QStringLiteral("constantsList"));

        verticalLayout_2->addWidget(constantsList);

        tabWidget->addTab(constantsTab, QString());
        structureTab = new QWidget();
        structureTab->setObjectName(QStringLiteral("structureTab"));
        verticalLayout_6 = new QVBoxLayout(structureTab);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        structuresTextEdit = new QPlainTextEdit(structureTab);
        structuresTextEdit->setObjectName(QStringLiteral("structuresTextEdit"));
        structuresTextEdit->setReadOnly(true);
        structuresTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_6->addWidget(structuresTextEdit);

        tabWidget->addTab(structureTab, QString());

        verticalLayout_4->addWidget(tabWidget);

        splitter->addWidget(verticalLayoutWidget_2);

        verticalLayout->addWidget(splitter);

        buttonBox = new QDialogButtonBox(PluginDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        verticalLayout_5->addLayout(verticalLayout);


        retranslateUi(PluginDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), PluginDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), PluginDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(PluginDialog);
    } // setupUi

    void retranslateUi(QDialog *PluginDialog)
    {
        PluginDialog->setWindowTitle(QApplication::translate("PluginDialog", "Plugin Libraries", 0));
        label->setText(QApplication::translate("PluginDialog", "Tick the plugin libraries to use", 0));
        tabWidget->setTabText(tabWidget->indexOf(functionsTab), QApplication::translate("PluginDialog", "Functions", 0));
        tabWidget->setTabText(tabWidget->indexOf(constantsTab), QApplication::translate("PluginDialog", "Constants", 0));
        tabWidget->setTabText(tabWidget->indexOf(structureTab), QApplication::translate("PluginDialog", "Structures", 0));
    } // retranslateUi

};

namespace Ui {
    class PluginDialog: public Ui_PluginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLUGINDIALOG_H
