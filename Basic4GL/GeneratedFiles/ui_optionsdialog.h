/********************************************************************************
** Form generated from reading UI file 'optionsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPTIONSDIALOG_H
#define UI_OPTIONSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OptionsDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout_4;
    QLabel *label_6;
    QComboBox *resCombo;
    QGroupBox *groupBox_4;
    QFormLayout *formLayout_5;
    QRadioButton *depthDesktopBtn;
    QRadioButton *depth16Btn;
    QRadioButton *depth32Btn;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_9;
    QGroupBox *groupBox_6;
    QFormLayout *formLayout_6;
    QCheckBox *stencilCheck;
    QCheckBox *fullScreenCheck;
    QCheckBox *borderCheck;
    QCheckBox *resizeCheck;
    QSpacerItem *verticalSpacer_3;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *label;
    QComboBox *languageCombo;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_4;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QCheckBox *safeModeCheck;
    QSpacerItem *verticalSpacer;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_6;
    QFormLayout *formLayout_2;
    QLabel *label_4;
    QCheckBox *autoCompleteCheck;
    QLabel *label_5;
    QCheckBox *functionSignatureCheck;
    QLabel *label_7;
    QSpacerItem *verticalSpacer_4;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *OptionsDialog)
    {
        if (OptionsDialog->objectName().isEmpty())
            OptionsDialog->setObjectName(QStringLiteral("OptionsDialog"));
        OptionsDialog->resize(569, 316);
        verticalLayout_2 = new QVBoxLayout(OptionsDialog);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        tabWidget = new QTabWidget(OptionsDialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout_5 = new QVBoxLayout(tab);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        groupBox_3 = new QGroupBox(tab);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        formLayout_4 = new QFormLayout(groupBox_3);
        formLayout_4->setObjectName(QStringLiteral("formLayout_4"));
        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QStringLiteral("label_6"));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_6);

        resCombo = new QComboBox(groupBox_3);
        resCombo->setObjectName(QStringLiteral("resCombo"));

        formLayout_4->setWidget(0, QFormLayout::FieldRole, resCombo);


        verticalLayout_3->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(tab);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy);
        formLayout_5 = new QFormLayout(groupBox_4);
        formLayout_5->setObjectName(QStringLiteral("formLayout_5"));
        depthDesktopBtn = new QRadioButton(groupBox_4);
        depthDesktopBtn->setObjectName(QStringLiteral("depthDesktopBtn"));
        depthDesktopBtn->setChecked(true);

        formLayout_5->setWidget(0, QFormLayout::LabelRole, depthDesktopBtn);

        depth16Btn = new QRadioButton(groupBox_4);
        depth16Btn->setObjectName(QStringLiteral("depth16Btn"));

        formLayout_5->setWidget(1, QFormLayout::LabelRole, depth16Btn);

        depth32Btn = new QRadioButton(groupBox_4);
        depth32Btn->setObjectName(QStringLiteral("depth32Btn"));

        formLayout_5->setWidget(2, QFormLayout::LabelRole, depth32Btn);


        verticalLayout_3->addWidget(groupBox_4);


        horizontalLayout_4->addLayout(verticalLayout_3);

        widget_2 = new QWidget(tab);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy1);
        widget_2->setMinimumSize(QSize(0, 0));
        verticalLayout_9 = new QVBoxLayout(widget_2);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        groupBox_6 = new QGroupBox(widget_2);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBox_6->sizePolicy().hasHeightForWidth());
        groupBox_6->setSizePolicy(sizePolicy2);
        formLayout_6 = new QFormLayout(groupBox_6);
        formLayout_6->setObjectName(QStringLiteral("formLayout_6"));
        stencilCheck = new QCheckBox(groupBox_6);
        stencilCheck->setObjectName(QStringLiteral("stencilCheck"));

        formLayout_6->setWidget(0, QFormLayout::LabelRole, stencilCheck);


        verticalLayout_9->addWidget(groupBox_6);

        fullScreenCheck = new QCheckBox(widget_2);
        fullScreenCheck->setObjectName(QStringLiteral("fullScreenCheck"));
        QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(fullScreenCheck->sizePolicy().hasHeightForWidth());
        fullScreenCheck->setSizePolicy(sizePolicy3);

        verticalLayout_9->addWidget(fullScreenCheck);

        borderCheck = new QCheckBox(widget_2);
        borderCheck->setObjectName(QStringLiteral("borderCheck"));
        sizePolicy3.setHeightForWidth(borderCheck->sizePolicy().hasHeightForWidth());
        borderCheck->setSizePolicy(sizePolicy3);
        borderCheck->setChecked(true);

        verticalLayout_9->addWidget(borderCheck);

        resizeCheck = new QCheckBox(widget_2);
        resizeCheck->setObjectName(QStringLiteral("resizeCheck"));
        sizePolicy3.setHeightForWidth(resizeCheck->sizePolicy().hasHeightForWidth());
        resizeCheck->setSizePolicy(sizePolicy3);

        verticalLayout_9->addWidget(resizeCheck);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_3);


        horizontalLayout_4->addWidget(widget_2);


        verticalLayout_5->addLayout(horizontalLayout_4);

        groupBox = new QGroupBox(tab);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        languageCombo = new QComboBox(groupBox);
        languageCombo->setObjectName(QStringLiteral("languageCombo"));

        formLayout->setWidget(0, QFormLayout::FieldRole, languageCombo);


        verticalLayout_5->addWidget(groupBox);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_2);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        verticalLayout_4 = new QVBoxLayout(tab_2);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        frame_2 = new QFrame(tab_2);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        verticalLayout = new QVBoxLayout(frame_2);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(frame_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy4);
        label_2->setWordWrap(true);

        verticalLayout->addWidget(label_2);

        safeModeCheck = new QCheckBox(frame_2);
        safeModeCheck->setObjectName(QStringLiteral("safeModeCheck"));

        verticalLayout->addWidget(safeModeCheck);


        verticalLayout_4->addWidget(frame_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        verticalLayout_6 = new QVBoxLayout(tab_3);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName(QStringLiteral("formLayout_2"));
        label_4 = new QLabel(tab_3);
        label_4->setObjectName(QStringLiteral("label_4"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_4);

        autoCompleteCheck = new QCheckBox(tab_3);
        autoCompleteCheck->setObjectName(QStringLiteral("autoCompleteCheck"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, autoCompleteCheck);

        label_5 = new QLabel(tab_3);
        label_5->setObjectName(QStringLiteral("label_5"));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, label_5);

        functionSignatureCheck = new QCheckBox(tab_3);
        functionSignatureCheck->setObjectName(QStringLiteral("functionSignatureCheck"));

        formLayout_2->setWidget(2, QFormLayout::FieldRole, functionSignatureCheck);

        label_7 = new QLabel(tab_3);
        label_7->setObjectName(QStringLiteral("label_7"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, label_7);


        verticalLayout_6->addLayout(formLayout_2);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_4);

        tabWidget->addTab(tab_3, QString());

        verticalLayout_2->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(OptionsDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(OptionsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), OptionsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), OptionsDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(OptionsDialog);
    } // setupUi

    void retranslateUi(QDialog *OptionsDialog)
    {
        OptionsDialog->setWindowTitle(QApplication::translate("OptionsDialog", "BASIC runtime settings", 0));
        groupBox_3->setTitle(QApplication::translate("OptionsDialog", "Screen/window resolution", 0));
        label_6->setText(QApplication::translate("OptionsDialog", "Resolution", 0));
        groupBox_4->setTitle(QApplication::translate("OptionsDialog", "Colour depth", 0));
        depthDesktopBtn->setText(QApplication::translate("OptionsDialog", "Use desktop", 0));
        depth16Btn->setText(QApplication::translate("OptionsDialog", "16 bit", 0));
        depth32Btn->setText(QApplication::translate("OptionsDialog", "32 bit", 0));
        groupBox_6->setTitle(QApplication::translate("OptionsDialog", "Advanced", 0));
        stencilCheck->setText(QApplication::translate("OptionsDialog", "Enable stencil buffer", 0));
        fullScreenCheck->setText(QApplication::translate("OptionsDialog", "Full screen", 0));
        borderCheck->setText(QApplication::translate("OptionsDialog", "Border around window", 0));
        resizeCheck->setText(QApplication::translate("OptionsDialog", "Allow resizing", 0));
        groupBox->setTitle(QApplication::translate("OptionsDialog", "Language syntax", 0));
        label->setText(QApplication::translate("OptionsDialog", "Default syntax", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("OptionsDialog", "Settings", 0));
        label_2->setText(QApplication::translate("OptionsDialog", "<html><head/><body><p>Safe mode stops Basic4GL programs from deleting files, and reading or writing files outside of the folder the program is running in.</p><p>When safe mode is switched <span style=\" font-weight:600;\">on</span>, programs cannot delete important system files, write a virus to your Start Menu, or scan through your &quot;My Documents&quot; folder and transmit your personal files to a file server on the Internet somewhere.</p><p>If safe mode is switched <span style=\" font-weight:600;\">off</span>, then <span style=\" font-style:italic;\">you are responsible for making sure that any program you run will not damage your computer.</span></p><p><span style=\" font-weight:600;\">I strongly recommend leaving safe mode ON, unless you have a really good reason.</span></p></body></html>", 0));
        safeModeCheck->setText(QApplication::translate("OptionsDialog", "Enable safe mode", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("OptionsDialog", "Safe mode", 0));
        label_4->setText(QApplication::translate("OptionsDialog", "Popup information", 0));
        autoCompleteCheck->setText(QApplication::translate("OptionsDialog", "Auto-complete function names", 0));
        label_5->setText(QApplication::translate("OptionsDialog", "<html><head/><body><p>Displays a drop-down list of matching BASIC function names as you type.</p><p>Alternatively you can press <span style=\" font-weight:600;\">Ctrl+Space</span> to bring this up.</p></body></html>", 0));
        functionSignatureCheck->setText(QApplication::translate("OptionsDialog", "Show function signatures", 0));
        label_7->setText(QApplication::translate("OptionsDialog", "<html><head/><body><p>Displays a popup hint showing the current function parameter types and return type.</p><p>Alternatively you can press <span style=\" font-weight:600;\">Ctrl+Shift+Space</span> to bring this up.</p></body></html>", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("OptionsDialog", "Editor", 0));
    } // retranslateUi

};

namespace Ui {
    class OptionsDialog: public Ui_OptionsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPTIONSDIALOG_H
