/********************************************************************************
** Form generated from reading UI file 'createstandalonedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATESTANDALONEDIALOG_H
#define UI_CREATESTANDALONEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_createstandalonedialog
{
public:
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *programTab;
    QVBoxLayout *verticalLayout_3;
    QFormLayout *formLayout;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *titleEdit;
    QLabel *label_3;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout;
    QLineEdit *filenameEdit;
    QPushButton *filenameButton;
    QWidget *optionsTab;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *groupBox_2;
    QFormLayout *formLayout_2;
    QRadioButton *finishCloseBtn;
    QRadioButton *finishWaitKeyBtn;
    QRadioButton *finishWaitCloseBtn;
    QGroupBox *groupBox;
    QFormLayout *formLayout_3;
    QRadioButton *errorCloseBtn;
    QRadioButton *errorGeneralBtn;
    QRadioButton *errorSpecificBtn;
    QCheckBox *escQuitCheck;
    QLabel *label_5;
    QWidget *widget;
    QWidget *screenModeTab;
    QVBoxLayout *verticalLayout_8;
    QHBoxLayout *horizontalLayout_4;
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
    QGroupBox *groupBox_5;
    QFormLayout *formLayout_7;
    QRadioButton *createWindowBtn;
    QRadioButton *dontCreateWindowBtn;
    QWidget *widget_3;
    QWidget *embeddedFilesTab;
    QVBoxLayout *verticalLayout_7;
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout_4;
    QListWidget *embedList;
    QGridLayout *gridLayout;
    QPushButton *embedAddBtn;
    QPushButton *embedRemoveBtn;
    QLabel *label_7;
    QDialogButtonBox *buttonBox;
    QLabel *label;

    void setupUi(QDialog *createstandalonedialog)
    {
        if (createstandalonedialog->objectName().isEmpty())
            createstandalonedialog->setObjectName(QStringLiteral("createstandalonedialog"));
        createstandalonedialog->resize(653, 372);
        gridLayout_2 = new QGridLayout(createstandalonedialog);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabWidget = new QTabWidget(createstandalonedialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setTabShape(QTabWidget::Rounded);
        programTab = new QWidget();
        programTab->setObjectName(QStringLiteral("programTab"));
        verticalLayout_3 = new QVBoxLayout(programTab);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        label_2 = new QLabel(programTab);
        label_2->setObjectName(QStringLiteral("label_2"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        titleEdit = new QLineEdit(programTab);
        titleEdit->setObjectName(QStringLiteral("titleEdit"));
        titleEdit->setMaxLength(255);

        horizontalLayout_3->addWidget(titleEdit);

        label_3 = new QLabel(programTab);
        label_3->setObjectName(QStringLiteral("label_3"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setMinimumSize(QSize(0, 0));
        label_3->setWordWrap(true);

        horizontalLayout_3->addWidget(label_3);


        formLayout->setLayout(0, QFormLayout::FieldRole, horizontalLayout_3);

        label_4 = new QLabel(programTab);
        label_4->setObjectName(QStringLiteral("label_4"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        filenameEdit = new QLineEdit(programTab);
        filenameEdit->setObjectName(QStringLiteral("filenameEdit"));

        horizontalLayout->addWidget(filenameEdit);

        filenameButton = new QPushButton(programTab);
        filenameButton->setObjectName(QStringLiteral("filenameButton"));

        horizontalLayout->addWidget(filenameButton);


        formLayout->setLayout(1, QFormLayout::FieldRole, horizontalLayout);


        verticalLayout_3->addLayout(formLayout);

        tabWidget->addTab(programTab, QString());
        optionsTab = new QWidget();
        optionsTab->setObjectName(QStringLiteral("optionsTab"));
        verticalLayout_5 = new QVBoxLayout(optionsTab);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        groupBox_2 = new QGroupBox(optionsTab);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy1);
        formLayout_2 = new QFormLayout(groupBox_2);
        formLayout_2->setObjectName(QStringLiteral("formLayout_2"));
        finishCloseBtn = new QRadioButton(groupBox_2);
        finishCloseBtn->setObjectName(QStringLiteral("finishCloseBtn"));
        finishCloseBtn->setChecked(true);

        formLayout_2->setWidget(0, QFormLayout::LabelRole, finishCloseBtn);

        finishWaitKeyBtn = new QRadioButton(groupBox_2);
        finishWaitKeyBtn->setObjectName(QStringLiteral("finishWaitKeyBtn"));
        finishWaitKeyBtn->setChecked(false);

        formLayout_2->setWidget(1, QFormLayout::LabelRole, finishWaitKeyBtn);

        finishWaitCloseBtn = new QRadioButton(groupBox_2);
        finishWaitCloseBtn->setObjectName(QStringLiteral("finishWaitCloseBtn"));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, finishWaitCloseBtn);


        horizontalLayout_2->addWidget(groupBox_2);

        groupBox = new QGroupBox(optionsTab);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        formLayout_3 = new QFormLayout(groupBox);
        formLayout_3->setObjectName(QStringLiteral("formLayout_3"));
        errorCloseBtn = new QRadioButton(groupBox);
        errorCloseBtn->setObjectName(QStringLiteral("errorCloseBtn"));
        errorCloseBtn->setChecked(true);

        formLayout_3->setWidget(0, QFormLayout::LabelRole, errorCloseBtn);

        errorGeneralBtn = new QRadioButton(groupBox);
        errorGeneralBtn->setObjectName(QStringLiteral("errorGeneralBtn"));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, errorGeneralBtn);

        errorSpecificBtn = new QRadioButton(groupBox);
        errorSpecificBtn->setObjectName(QStringLiteral("errorSpecificBtn"));
        errorSpecificBtn->setChecked(false);

        formLayout_3->setWidget(2, QFormLayout::LabelRole, errorSpecificBtn);


        horizontalLayout_2->addWidget(groupBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        escQuitCheck = new QCheckBox(optionsTab);
        escQuitCheck->setObjectName(QStringLiteral("escQuitCheck"));
        escQuitCheck->setChecked(true);

        verticalLayout_2->addWidget(escQuitCheck);

        label_5 = new QLabel(optionsTab);
        label_5->setObjectName(QStringLiteral("label_5"));
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);

        verticalLayout_2->addWidget(label_5);

        widget = new QWidget(optionsTab);
        widget->setObjectName(QStringLiteral("widget"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(widget);


        verticalLayout_5->addLayout(verticalLayout_2);

        tabWidget->addTab(optionsTab, QString());
        screenModeTab = new QWidget();
        screenModeTab->setObjectName(QStringLiteral("screenModeTab"));
        verticalLayout_8 = new QVBoxLayout(screenModeTab);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        groupBox_3 = new QGroupBox(screenModeTab);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        formLayout_4 = new QFormLayout(groupBox_3);
        formLayout_4->setObjectName(QStringLiteral("formLayout_4"));
        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QStringLiteral("label_6"));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_6);

        resCombo = new QComboBox(groupBox_3);
        resCombo->setObjectName(QStringLiteral("resCombo"));

        formLayout_4->setWidget(0, QFormLayout::FieldRole, resCombo);


        horizontalLayout_4->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(screenModeTab);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBox_4->sizePolicy().hasHeightForWidth());
        groupBox_4->setSizePolicy(sizePolicy3);
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


        horizontalLayout_4->addWidget(groupBox_4);

        widget_2 = new QWidget(screenModeTab);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        sizePolicy3.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy3);
        widget_2->setMinimumSize(QSize(0, 0));
        verticalLayout_9 = new QVBoxLayout(widget_2);
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        groupBox_6 = new QGroupBox(widget_2);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        QSizePolicy sizePolicy4(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(groupBox_6->sizePolicy().hasHeightForWidth());
        groupBox_6->setSizePolicy(sizePolicy4);
        formLayout_6 = new QFormLayout(groupBox_6);
        formLayout_6->setObjectName(QStringLiteral("formLayout_6"));
        stencilCheck = new QCheckBox(groupBox_6);
        stencilCheck->setObjectName(QStringLiteral("stencilCheck"));

        formLayout_6->setWidget(0, QFormLayout::LabelRole, stencilCheck);


        verticalLayout_9->addWidget(groupBox_6);

        fullScreenCheck = new QCheckBox(widget_2);
        fullScreenCheck->setObjectName(QStringLiteral("fullScreenCheck"));
        QSizePolicy sizePolicy5(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(fullScreenCheck->sizePolicy().hasHeightForWidth());
        fullScreenCheck->setSizePolicy(sizePolicy5);

        verticalLayout_9->addWidget(fullScreenCheck);

        borderCheck = new QCheckBox(widget_2);
        borderCheck->setObjectName(QStringLiteral("borderCheck"));
        sizePolicy5.setHeightForWidth(borderCheck->sizePolicy().hasHeightForWidth());
        borderCheck->setSizePolicy(sizePolicy5);
        borderCheck->setChecked(true);

        verticalLayout_9->addWidget(borderCheck);

        resizeCheck = new QCheckBox(widget_2);
        resizeCheck->setObjectName(QStringLiteral("resizeCheck"));
        sizePolicy5.setHeightForWidth(resizeCheck->sizePolicy().hasHeightForWidth());
        resizeCheck->setSizePolicy(sizePolicy5);

        verticalLayout_9->addWidget(resizeCheck);


        horizontalLayout_4->addWidget(widget_2);


        verticalLayout_8->addLayout(horizontalLayout_4);

        groupBox_5 = new QGroupBox(screenModeTab);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        sizePolicy1.setHeightForWidth(groupBox_5->sizePolicy().hasHeightForWidth());
        groupBox_5->setSizePolicy(sizePolicy1);
        formLayout_7 = new QFormLayout(groupBox_5);
        formLayout_7->setObjectName(QStringLiteral("formLayout_7"));
        createWindowBtn = new QRadioButton(groupBox_5);
        createWindowBtn->setObjectName(QStringLiteral("createWindowBtn"));
        createWindowBtn->setChecked(true);

        formLayout_7->setWidget(0, QFormLayout::LabelRole, createWindowBtn);

        dontCreateWindowBtn = new QRadioButton(groupBox_5);
        dontCreateWindowBtn->setObjectName(QStringLiteral("dontCreateWindowBtn"));

        formLayout_7->setWidget(1, QFormLayout::LabelRole, dontCreateWindowBtn);


        verticalLayout_8->addWidget(groupBox_5);

        widget_3 = new QWidget(screenModeTab);
        widget_3->setObjectName(QStringLiteral("widget_3"));
        sizePolicy2.setHeightForWidth(widget_3->sizePolicy().hasHeightForWidth());
        widget_3->setSizePolicy(sizePolicy2);

        verticalLayout_8->addWidget(widget_3);

        tabWidget->addTab(screenModeTab, QString());
        embeddedFilesTab = new QWidget();
        embeddedFilesTab->setObjectName(QStringLiteral("embeddedFilesTab"));
        verticalLayout_7 = new QVBoxLayout(embeddedFilesTab);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        embedList = new QListWidget(embeddedFilesTab);
        embedList->setObjectName(QStringLiteral("embedList"));
        embedList->setSelectionMode(QAbstractItemView::MultiSelection);

        verticalLayout_4->addWidget(embedList);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        embedAddBtn = new QPushButton(embeddedFilesTab);
        embedAddBtn->setObjectName(QStringLiteral("embedAddBtn"));

        gridLayout->addWidget(embedAddBtn, 0, 0, 1, 1);

        embedRemoveBtn = new QPushButton(embeddedFilesTab);
        embedRemoveBtn->setObjectName(QStringLiteral("embedRemoveBtn"));

        gridLayout->addWidget(embedRemoveBtn, 0, 1, 1, 1);


        verticalLayout_4->addLayout(gridLayout);


        horizontalLayout_5->addLayout(verticalLayout_4);

        label_7 = new QLabel(embeddedFilesTab);
        label_7->setObjectName(QStringLiteral("label_7"));
        QSizePolicy sizePolicy6(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy6);
        label_7->setWordWrap(true);

        horizontalLayout_5->addWidget(label_7);


        verticalLayout_7->addLayout(horizontalLayout_5);

        tabWidget->addTab(embeddedFilesTab, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(createstandalonedialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        gridLayout_2->addLayout(verticalLayout, 1, 0, 1, 1);

        label = new QLabel(createstandalonedialog);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        QWidget::setTabOrder(titleEdit, filenameEdit);
        QWidget::setTabOrder(filenameEdit, filenameButton);
        QWidget::setTabOrder(filenameButton, finishCloseBtn);
        QWidget::setTabOrder(finishCloseBtn, finishWaitKeyBtn);
        QWidget::setTabOrder(finishWaitKeyBtn, finishWaitCloseBtn);
        QWidget::setTabOrder(finishWaitCloseBtn, errorCloseBtn);
        QWidget::setTabOrder(errorCloseBtn, errorGeneralBtn);
        QWidget::setTabOrder(errorGeneralBtn, errorSpecificBtn);
        QWidget::setTabOrder(errorSpecificBtn, escQuitCheck);

        retranslateUi(createstandalonedialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), createstandalonedialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), createstandalonedialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(createstandalonedialog);
    } // setupUi

    void retranslateUi(QDialog *createstandalonedialog)
    {
        createstandalonedialog->setWindowTitle(QApplication::translate("createstandalonedialog", "Create Standalone .exe", 0));
        label_2->setText(QApplication::translate("createstandalonedialog", "Program Title", 0));
        label_3->setText(QApplication::translate("createstandalonedialog", "This is displayed on the window (in windowed mode) and on the task bar.", 0));
        label_4->setText(QApplication::translate("createstandalonedialog", "Filename", 0));
        filenameButton->setText(QApplication::translate("createstandalonedialog", "...", 0));
        tabWidget->setTabText(tabWidget->indexOf(programTab), QApplication::translate("createstandalonedialog", "Program", 0));
        groupBox_2->setTitle(QApplication::translate("createstandalonedialog", "When program finishes", 0));
        finishCloseBtn->setText(QApplication::translate("createstandalonedialog", "Close immediately", 0));
        finishWaitKeyBtn->setText(QApplication::translate("createstandalonedialog", "Wait for keypress then close", 0));
        finishWaitCloseBtn->setText(QApplication::translate("createstandalonedialog", "Wait for window to be closed", 0));
        groupBox->setTitle(QApplication::translate("createstandalonedialog", "If a runtime error occurs", 0));
        errorCloseBtn->setText(QApplication::translate("createstandalonedialog", "Just close", 0));
        errorGeneralBtn->setText(QApplication::translate("createstandalonedialog", "Show a general \"An error has occurred\" message", 0));
        errorSpecificBtn->setText(QApplication::translate("createstandalonedialog", "Show error complete with line number", 0));
        escQuitCheck->setText(QApplication::translate("createstandalonedialog", "\"Esc\" key or closing window quits", 0));
        label_5->setText(QApplication::translate("createstandalonedialog", "If you disable this option, make sure you build a QUIT function into your program!", 0));
        tabWidget->setTabText(tabWidget->indexOf(optionsTab), QApplication::translate("createstandalonedialog", "Options", 0));
        groupBox_3->setTitle(QApplication::translate("createstandalonedialog", "Screen/window resolution", 0));
        label_6->setText(QApplication::translate("createstandalonedialog", "Resolution", 0));
        groupBox_4->setTitle(QApplication::translate("createstandalonedialog", "Colour depth", 0));
        depthDesktopBtn->setText(QApplication::translate("createstandalonedialog", "Use desktop", 0));
        depth16Btn->setText(QApplication::translate("createstandalonedialog", "16 bit", 0));
        depth32Btn->setText(QApplication::translate("createstandalonedialog", "32 bit", 0));
        groupBox_6->setTitle(QApplication::translate("createstandalonedialog", "Advanced", 0));
        stencilCheck->setText(QApplication::translate("createstandalonedialog", "Enable stencil buffer", 0));
        fullScreenCheck->setText(QApplication::translate("createstandalonedialog", "Full screen", 0));
        borderCheck->setText(QApplication::translate("createstandalonedialog", "Border around window", 0));
        resizeCheck->setText(QApplication::translate("createstandalonedialog", "Allow resizing", 0));
        groupBox_5->setTitle(QApplication::translate("createstandalonedialog", "Startup", 0));
        createWindowBtn->setText(QApplication::translate("createstandalonedialog", "Create window when application starts", 0));
        dontCreateWindowBtn->setText(QApplication::translate("createstandalonedialog", "Don't create window until UpdateWindow() is called", 0));
        tabWidget->setTabText(tabWidget->indexOf(screenModeTab), QApplication::translate("createstandalonedialog", "Screen Mode", 0));
        embedAddBtn->setText(QApplication::translate("createstandalonedialog", "Add...", 0));
        embedRemoveBtn->setText(QApplication::translate("createstandalonedialog", "Remove", 0));
        label_7->setText(QApplication::translate("createstandalonedialog", "<html><head/><body><p>Choose the files that will be embedded into your program.</p><p>These files will be stored in the actual .exe file.</p><p>Whenever the program loads a file (using LoadTex, OpenFileRead etc) it will look in the embedded files list first. If it finds the file, it will load it from there instead of from the disk.</p></body></html>", 0));
        tabWidget->setTabText(tabWidget->indexOf(embeddedFilesTab), QApplication::translate("createstandalonedialog", "Embedded Files", 0));
        label->setText(QApplication::translate("createstandalonedialog", "This will create an executable file that does not require Basic4GL to run.", 0));
    } // retranslateUi

};

namespace Ui {
    class createstandalonedialog: public Ui_createstandalonedialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATESTANDALONEDIALOG_H
