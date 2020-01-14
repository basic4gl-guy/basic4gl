/********************************************************************************
** Form generated from reading UI file 'virtualmachineviewdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIRTUALMACHINEVIEWDIALOG_H
#define UI_VIRTUALMACHINEVIEWDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_VirtualMachineViewDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_6;
    QLabel *label_5;
    QLineEdit *reg1Int;
    QLineEdit *reg1Float;
    QLineEdit *reg1String;
    QLineEdit *reg2Int;
    QLineEdit *reg2Float;
    QLineEdit *reg2String;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QToolButton *stepButton;
    QHBoxLayout *horizontalLayout_2;
    QTableWidget *codeGrid;
    QScrollBar *codeScroll;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_6;
    QTableWidget *callGrid;
    QGroupBox *groupBox_5;
    QHBoxLayout *horizontalLayout_7;
    QTableWidget *variableGrid;
    QScrollBar *variableScroll;
    QHBoxLayout *horizontalLayout_3;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_5;
    QTableWidget *heapGrid;
    QScrollBar *heapScroll;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QTableWidget *stackGrid;
    QScrollBar *stackScroll;

    void setupUi(QDialog *VirtualMachineViewDialog)
    {
        if (VirtualMachineViewDialog->objectName().isEmpty())
            VirtualMachineViewDialog->setObjectName(QStringLiteral("VirtualMachineViewDialog"));
        VirtualMachineViewDialog->resize(1070, 712);
        verticalLayout = new QVBoxLayout(VirtualMachineViewDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_2 = new QLabel(VirtualMachineViewDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(VirtualMachineViewDialog);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 0, 1, 1, 1);

        label_4 = new QLabel(VirtualMachineViewDialog);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 2, 0, 1, 1);

        label_6 = new QLabel(VirtualMachineViewDialog);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 0, 3, 1, 1);

        label_5 = new QLabel(VirtualMachineViewDialog);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 0, 2, 1, 1);

        reg1Int = new QLineEdit(VirtualMachineViewDialog);
        reg1Int->setObjectName(QStringLiteral("reg1Int"));
        reg1Int->setReadOnly(true);

        gridLayout->addWidget(reg1Int, 1, 1, 1, 1);

        reg1Float = new QLineEdit(VirtualMachineViewDialog);
        reg1Float->setObjectName(QStringLiteral("reg1Float"));
        reg1Float->setReadOnly(true);

        gridLayout->addWidget(reg1Float, 1, 2, 1, 1);

        reg1String = new QLineEdit(VirtualMachineViewDialog);
        reg1String->setObjectName(QStringLiteral("reg1String"));
        reg1String->setReadOnly(true);

        gridLayout->addWidget(reg1String, 1, 3, 1, 1);

        reg2Int = new QLineEdit(VirtualMachineViewDialog);
        reg2Int->setObjectName(QStringLiteral("reg2Int"));
        reg2Int->setReadOnly(true);

        gridLayout->addWidget(reg2Int, 2, 1, 1, 1);

        reg2Float = new QLineEdit(VirtualMachineViewDialog);
        reg2Float->setObjectName(QStringLiteral("reg2Float"));
        reg2Float->setReadOnly(true);

        gridLayout->addWidget(reg2Float, 2, 2, 1, 1);

        reg2String = new QLineEdit(VirtualMachineViewDialog);
        reg2String->setObjectName(QStringLiteral("reg2String"));
        reg2String->setReadOnly(true);

        gridLayout->addWidget(reg2String, 2, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(VirtualMachineViewDialog);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        stepButton = new QToolButton(groupBox);
        stepButton->setObjectName(QStringLiteral("stepButton"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/Images/StepOver2.png"), QSize(), QIcon::Normal, QIcon::Off);
        stepButton->setIcon(icon);
        stepButton->setIconSize(QSize(32, 32));

        verticalLayout_2->addWidget(stepButton);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        codeGrid = new QTableWidget(groupBox);
        if (codeGrid->columnCount() < 5)
            codeGrid->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        codeGrid->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        codeGrid->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        codeGrid->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        codeGrid->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        codeGrid->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        if (codeGrid->rowCount() < 5)
            codeGrid->setRowCount(5);
        codeGrid->setObjectName(QStringLiteral("codeGrid"));
        codeGrid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        codeGrid->setRowCount(5);
        codeGrid->verticalHeader()->setVisible(false);

        horizontalLayout_2->addWidget(codeGrid);

        codeScroll = new QScrollBar(groupBox);
        codeScroll->setObjectName(QStringLiteral("codeScroll"));
        codeScroll->setOrientation(Qt::Vertical);

        horizontalLayout_2->addWidget(codeScroll);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout->addWidget(groupBox);

        groupBox_4 = new QGroupBox(VirtualMachineViewDialog);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        horizontalLayout_6 = new QHBoxLayout(groupBox_4);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        callGrid = new QTableWidget(groupBox_4);
        if (callGrid->columnCount() < 1)
            callGrid->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        callGrid->setHorizontalHeaderItem(0, __qtablewidgetitem5);
        if (callGrid->rowCount() < 3)
            callGrid->setRowCount(3);
        callGrid->setObjectName(QStringLiteral("callGrid"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(callGrid->sizePolicy().hasHeightForWidth());
        callGrid->setSizePolicy(sizePolicy);
        callGrid->setRowCount(3);
        callGrid->verticalHeader()->setVisible(false);

        horizontalLayout_6->addWidget(callGrid);


        horizontalLayout->addWidget(groupBox_4);

        groupBox_5 = new QGroupBox(VirtualMachineViewDialog);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        horizontalLayout_7 = new QHBoxLayout(groupBox_5);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        variableGrid = new QTableWidget(groupBox_5);
        if (variableGrid->columnCount() < 3)
            variableGrid->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        variableGrid->setHorizontalHeaderItem(0, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        variableGrid->setHorizontalHeaderItem(1, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        variableGrid->setHorizontalHeaderItem(2, __qtablewidgetitem8);
        if (variableGrid->rowCount() < 3)
            variableGrid->setRowCount(3);
        variableGrid->setObjectName(QStringLiteral("variableGrid"));
        sizePolicy.setHeightForWidth(variableGrid->sizePolicy().hasHeightForWidth());
        variableGrid->setSizePolicy(sizePolicy);
        variableGrid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        variableGrid->setRowCount(3);
        variableGrid->verticalHeader()->setVisible(false);

        horizontalLayout_7->addWidget(variableGrid);

        variableScroll = new QScrollBar(groupBox_5);
        variableScroll->setObjectName(QStringLiteral("variableScroll"));
        variableScroll->setOrientation(Qt::Vertical);

        horizontalLayout_7->addWidget(variableScroll);


        horizontalLayout->addWidget(groupBox_5);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        groupBox_3 = new QGroupBox(VirtualMachineViewDialog);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        horizontalLayout_5 = new QHBoxLayout(groupBox_3);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        heapGrid = new QTableWidget(groupBox_3);
        if (heapGrid->columnCount() < 4)
            heapGrid->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        heapGrid->setHorizontalHeaderItem(0, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        heapGrid->setHorizontalHeaderItem(1, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        heapGrid->setHorizontalHeaderItem(2, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        heapGrid->setHorizontalHeaderItem(3, __qtablewidgetitem12);
        if (heapGrid->rowCount() < 3)
            heapGrid->setRowCount(3);
        heapGrid->setObjectName(QStringLiteral("heapGrid"));
        heapGrid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        heapGrid->setRowCount(3);
        heapGrid->verticalHeader()->setVisible(false);

        horizontalLayout_5->addWidget(heapGrid);

        heapScroll = new QScrollBar(groupBox_3);
        heapScroll->setObjectName(QStringLiteral("heapScroll"));
        heapScroll->setOrientation(Qt::Vertical);

        horizontalLayout_5->addWidget(heapScroll);


        horizontalLayout_3->addWidget(groupBox_3);

        groupBox_2 = new QGroupBox(VirtualMachineViewDialog);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        stackGrid = new QTableWidget(groupBox_2);
        if (stackGrid->columnCount() < 4)
            stackGrid->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        stackGrid->setHorizontalHeaderItem(0, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        stackGrid->setHorizontalHeaderItem(1, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        stackGrid->setHorizontalHeaderItem(2, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        stackGrid->setHorizontalHeaderItem(3, __qtablewidgetitem16);
        if (stackGrid->rowCount() < 3)
            stackGrid->setRowCount(3);
        stackGrid->setObjectName(QStringLiteral("stackGrid"));
        stackGrid->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stackGrid->setRowCount(3);
        stackGrid->horizontalHeader()->setVisible(true);
        stackGrid->verticalHeader()->setVisible(false);

        horizontalLayout_4->addWidget(stackGrid);

        stackScroll = new QScrollBar(groupBox_2);
        stackScroll->setObjectName(QStringLiteral("stackScroll"));
        stackScroll->setOrientation(Qt::Vertical);

        horizontalLayout_4->addWidget(stackScroll);


        horizontalLayout_3->addWidget(groupBox_2);


        verticalLayout->addLayout(horizontalLayout_3);

        QWidget::setTabOrder(reg1Int, reg1Float);
        QWidget::setTabOrder(reg1Float, reg1String);
        QWidget::setTabOrder(reg1String, reg2Int);
        QWidget::setTabOrder(reg2Int, reg2Float);
        QWidget::setTabOrder(reg2Float, reg2String);
        QWidget::setTabOrder(reg2String, stepButton);
        QWidget::setTabOrder(stepButton, codeGrid);
        QWidget::setTabOrder(codeGrid, callGrid);
        QWidget::setTabOrder(callGrid, variableGrid);
        QWidget::setTabOrder(variableGrid, heapGrid);
        QWidget::setTabOrder(heapGrid, stackGrid);

        retranslateUi(VirtualMachineViewDialog);

        QMetaObject::connectSlotsByName(VirtualMachineViewDialog);
    } // setupUi

    void retranslateUi(QDialog *VirtualMachineViewDialog)
    {
        VirtualMachineViewDialog->setWindowTitle(QApplication::translate("VirtualMachineViewDialog", "Dialog", 0));
        label_2->setText(QApplication::translate("VirtualMachineViewDialog", "Register 1", 0));
        label_3->setText(QApplication::translate("VirtualMachineViewDialog", "Integer", 0));
        label_4->setText(QApplication::translate("VirtualMachineViewDialog", "Register 2", 0));
        label_6->setText(QApplication::translate("VirtualMachineViewDialog", "String", 0));
        label_5->setText(QApplication::translate("VirtualMachineViewDialog", "Floating Pt", 0));
        groupBox->setTitle(QApplication::translate("VirtualMachineViewDialog", "Code", 0));
        stepButton->setText(QApplication::translate("VirtualMachineViewDialog", "Step", 0));
        QTableWidgetItem *___qtablewidgetitem = codeGrid->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("VirtualMachineViewDialog", "#", 0));
        QTableWidgetItem *___qtablewidgetitem1 = codeGrid->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("VirtualMachineViewDialog", "OP", 0));
        QTableWidgetItem *___qtablewidgetitem2 = codeGrid->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("VirtualMachineViewDialog", "Type", 0));
        QTableWidgetItem *___qtablewidgetitem3 = codeGrid->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("VirtualMachineViewDialog", "Val", 0));
        QTableWidgetItem *___qtablewidgetitem4 = codeGrid->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("VirtualMachineViewDialog", "Source", 0));
        groupBox_4->setTitle(QApplication::translate("VirtualMachineViewDialog", "Call stack", 0));
        groupBox_5->setTitle(QApplication::translate("VirtualMachineViewDialog", "Variables", 0));
        QTableWidgetItem *___qtablewidgetitem5 = variableGrid->horizontalHeaderItem(0);
        ___qtablewidgetitem5->setText(QApplication::translate("VirtualMachineViewDialog", "#", 0));
        QTableWidgetItem *___qtablewidgetitem6 = variableGrid->horizontalHeaderItem(1);
        ___qtablewidgetitem6->setText(QApplication::translate("VirtualMachineViewDialog", "Variable", 0));
        QTableWidgetItem *___qtablewidgetitem7 = variableGrid->horizontalHeaderItem(2);
        ___qtablewidgetitem7->setText(QApplication::translate("VirtualMachineViewDialog", "Value", 0));
        groupBox_3->setTitle(QApplication::translate("VirtualMachineViewDialog", "Heap", 0));
        QTableWidgetItem *___qtablewidgetitem8 = heapGrid->horizontalHeaderItem(0);
        ___qtablewidgetitem8->setText(QApplication::translate("VirtualMachineViewDialog", "#", 0));
        QTableWidgetItem *___qtablewidgetitem9 = heapGrid->horizontalHeaderItem(1);
        ___qtablewidgetitem9->setText(QApplication::translate("VirtualMachineViewDialog", "Int", 0));
        QTableWidgetItem *___qtablewidgetitem10 = heapGrid->horizontalHeaderItem(2);
        ___qtablewidgetitem10->setText(QApplication::translate("VirtualMachineViewDialog", "Float", 0));
        QTableWidgetItem *___qtablewidgetitem11 = heapGrid->horizontalHeaderItem(3);
        ___qtablewidgetitem11->setText(QApplication::translate("VirtualMachineViewDialog", "String", 0));
        groupBox_2->setTitle(QApplication::translate("VirtualMachineViewDialog", "Stack", 0));
        QTableWidgetItem *___qtablewidgetitem12 = stackGrid->horizontalHeaderItem(0);
        ___qtablewidgetitem12->setText(QApplication::translate("VirtualMachineViewDialog", "#", 0));
        QTableWidgetItem *___qtablewidgetitem13 = stackGrid->horizontalHeaderItem(1);
        ___qtablewidgetitem13->setText(QApplication::translate("VirtualMachineViewDialog", "Int", 0));
        QTableWidgetItem *___qtablewidgetitem14 = stackGrid->horizontalHeaderItem(2);
        ___qtablewidgetitem14->setText(QApplication::translate("VirtualMachineViewDialog", "Float", 0));
        QTableWidgetItem *___qtablewidgetitem15 = stackGrid->horizontalHeaderItem(3);
        ___qtablewidgetitem15->setText(QApplication::translate("VirtualMachineViewDialog", "String", 0));
    } // retranslateUi

};

namespace Ui {
    class VirtualMachineViewDialog: public Ui_VirtualMachineViewDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIRTUALMACHINEVIEWDIALOG_H
