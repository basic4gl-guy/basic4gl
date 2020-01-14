/********************************************************************************
** Form generated from reading UI file 'basic4gleditor.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BASIC4GLEDITOR_H
#define UI_BASIC4GLEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Basic4GLEditor
{
public:
    QAction *actionOpen_Program;
    QAction *actionNew_Program;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionCreate_Standalone_exe;
    QAction *actionArguments;
    QAction *actionPlugin_Libraries;
    QAction *actionRun;
    QAction *actionSave_All;
    QAction *actionToggle_Breakpoint;
    QAction *actionPause;
    QAction *actionStep_Into;
    QAction *actionStep_Over;
    QAction *actionStep_Out_Of;
    QAction *actionDebug_Mode;
    QAction *actionCreate_Edit_watch;
    QAction *actionDelete_watch;
    QAction *actionAdd_watch_at_cursor;
    QAction *actionShow_Gosub_Function_call;
    QAction *actionBASIC_runtime_settings;
    QAction *actionGetting_started;
    QAction *actionProgrammer_s_guide;
    QAction *actionLanguage_guide;
    QAction *actionNetwork_engine_guide;
    QAction *actionOpenGL_guide;
    QAction *actionSymbolic_debugger;
    QAction *actionSprite_library_guide;
    QAction *actionCreating_standalone_exes;
    QAction *actionPlugin_dlls;
    QAction *actionAbout;
    QAction *actionFind;
    QAction *actionReplace;
    QAction *actionFind_next;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionVirtual_Machine;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QSplitter *sourceCodeSplitter;
    QTabWidget *sourceTabs;
    QWidget *tab;
    QVBoxLayout *verticalLayout;
    QSplitter *debugSplitter;
    QListWidget *watchList;
    QListWidget *stackList;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuProgram;
    QMenu *menuDebug;
    QMenu *menuSettings;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *debugToolbar;

    void setupUi(QMainWindow *Basic4GLEditor)
    {
        if (Basic4GLEditor->objectName().isEmpty())
            Basic4GLEditor->setObjectName(QStringLiteral("Basic4GLEditor"));
        Basic4GLEditor->resize(920, 700);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/Images/Icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        Basic4GLEditor->setWindowIcon(icon);
        actionOpen_Program = new QAction(Basic4GLEditor);
        actionOpen_Program->setObjectName(QStringLiteral("actionOpen_Program"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/icons/Images/ImgOpen.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen_Program->setIcon(icon1);
        actionNew_Program = new QAction(Basic4GLEditor);
        actionNew_Program->setObjectName(QStringLiteral("actionNew_Program"));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/Images/ImgNew.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew_Program->setIcon(icon2);
        actionSave = new QAction(Basic4GLEditor);
        actionSave->setObjectName(QStringLiteral("actionSave"));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/Images/ImgSave.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon3);
        actionSave_As = new QAction(Basic4GLEditor);
        actionSave_As->setObjectName(QStringLiteral("actionSave_As"));
        actionCreate_Standalone_exe = new QAction(Basic4GLEditor);
        actionCreate_Standalone_exe->setObjectName(QStringLiteral("actionCreate_Standalone_exe"));
        actionArguments = new QAction(Basic4GLEditor);
        actionArguments->setObjectName(QStringLiteral("actionArguments"));
        actionPlugin_Libraries = new QAction(Basic4GLEditor);
        actionPlugin_Libraries->setObjectName(QStringLiteral("actionPlugin_Libraries"));
        actionRun = new QAction(Basic4GLEditor);
        actionRun->setObjectName(QStringLiteral("actionRun"));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/Images/Go.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRun->setIcon(icon4);
        actionSave_All = new QAction(Basic4GLEditor);
        actionSave_All->setObjectName(QStringLiteral("actionSave_All"));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/icons/Images/ImgSaveAll.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_All->setIcon(icon5);
        actionToggle_Breakpoint = new QAction(Basic4GLEditor);
        actionToggle_Breakpoint->setObjectName(QStringLiteral("actionToggle_Breakpoint"));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/icons/Images/BreakPt.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionToggle_Breakpoint->setIcon(icon6);
        actionPause = new QAction(Basic4GLEditor);
        actionPause->setObjectName(QStringLiteral("actionPause"));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/icons/Images/Pause.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPause->setIcon(icon7);
        actionStep_Into = new QAction(Basic4GLEditor);
        actionStep_Into->setObjectName(QStringLiteral("actionStep_Into"));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/icons/Images/StepInto2.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStep_Into->setIcon(icon8);
        actionStep_Over = new QAction(Basic4GLEditor);
        actionStep_Over->setObjectName(QStringLiteral("actionStep_Over"));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/icons/Images/StepOver2.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStep_Over->setIcon(icon9);
        actionStep_Out_Of = new QAction(Basic4GLEditor);
        actionStep_Out_Of->setObjectName(QStringLiteral("actionStep_Out_Of"));
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/icons/Images/StepOutOf2.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStep_Out_Of->setIcon(icon10);
        actionDebug_Mode = new QAction(Basic4GLEditor);
        actionDebug_Mode->setObjectName(QStringLiteral("actionDebug_Mode"));
        actionDebug_Mode->setCheckable(true);
        QIcon icon11;
        icon11.addFile(QStringLiteral(":/icons/Images/Bug.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDebug_Mode->setIcon(icon11);
        actionCreate_Edit_watch = new QAction(Basic4GLEditor);
        actionCreate_Edit_watch->setObjectName(QStringLiteral("actionCreate_Edit_watch"));
        actionDelete_watch = new QAction(Basic4GLEditor);
        actionDelete_watch->setObjectName(QStringLiteral("actionDelete_watch"));
        actionAdd_watch_at_cursor = new QAction(Basic4GLEditor);
        actionAdd_watch_at_cursor->setObjectName(QStringLiteral("actionAdd_watch_at_cursor"));
        actionShow_Gosub_Function_call = new QAction(Basic4GLEditor);
        actionShow_Gosub_Function_call->setObjectName(QStringLiteral("actionShow_Gosub_Function_call"));
        actionBASIC_runtime_settings = new QAction(Basic4GLEditor);
        actionBASIC_runtime_settings->setObjectName(QStringLiteral("actionBASIC_runtime_settings"));
        actionGetting_started = new QAction(Basic4GLEditor);
        actionGetting_started->setObjectName(QStringLiteral("actionGetting_started"));
        actionProgrammer_s_guide = new QAction(Basic4GLEditor);
        actionProgrammer_s_guide->setObjectName(QStringLiteral("actionProgrammer_s_guide"));
        actionLanguage_guide = new QAction(Basic4GLEditor);
        actionLanguage_guide->setObjectName(QStringLiteral("actionLanguage_guide"));
        actionNetwork_engine_guide = new QAction(Basic4GLEditor);
        actionNetwork_engine_guide->setObjectName(QStringLiteral("actionNetwork_engine_guide"));
        actionOpenGL_guide = new QAction(Basic4GLEditor);
        actionOpenGL_guide->setObjectName(QStringLiteral("actionOpenGL_guide"));
        actionSymbolic_debugger = new QAction(Basic4GLEditor);
        actionSymbolic_debugger->setObjectName(QStringLiteral("actionSymbolic_debugger"));
        actionSprite_library_guide = new QAction(Basic4GLEditor);
        actionSprite_library_guide->setObjectName(QStringLiteral("actionSprite_library_guide"));
        actionCreating_standalone_exes = new QAction(Basic4GLEditor);
        actionCreating_standalone_exes->setObjectName(QStringLiteral("actionCreating_standalone_exes"));
        actionPlugin_dlls = new QAction(Basic4GLEditor);
        actionPlugin_dlls->setObjectName(QStringLiteral("actionPlugin_dlls"));
        actionAbout = new QAction(Basic4GLEditor);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionFind = new QAction(Basic4GLEditor);
        actionFind->setObjectName(QStringLiteral("actionFind"));
        actionReplace = new QAction(Basic4GLEditor);
        actionReplace->setObjectName(QStringLiteral("actionReplace"));
        actionFind_next = new QAction(Basic4GLEditor);
        actionFind_next->setObjectName(QStringLiteral("actionFind_next"));
        actionCut = new QAction(Basic4GLEditor);
        actionCut->setObjectName(QStringLiteral("actionCut"));
        actionCopy = new QAction(Basic4GLEditor);
        actionCopy->setObjectName(QStringLiteral("actionCopy"));
        actionPaste = new QAction(Basic4GLEditor);
        actionPaste->setObjectName(QStringLiteral("actionPaste"));
        actionVirtual_Machine = new QAction(Basic4GLEditor);
        actionVirtual_Machine->setObjectName(QStringLiteral("actionVirtual_Machine"));
        actionVirtual_Machine->setVisible(false);
        centralWidget = new QWidget(Basic4GLEditor);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        sourceCodeSplitter = new QSplitter(centralWidget);
        sourceCodeSplitter->setObjectName(QStringLiteral("sourceCodeSplitter"));
        sourceCodeSplitter->setOrientation(Qt::Vertical);
        sourceCodeSplitter->setChildrenCollapsible(false);
        sourceTabs = new QTabWidget(sourceCodeSplitter);
        sourceTabs->setObjectName(QStringLiteral("sourceTabs"));
        sourceTabs->setTabsClosable(true);
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        verticalLayout = new QVBoxLayout(tab);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        sourceTabs->addTab(tab, QString());
        sourceCodeSplitter->addWidget(sourceTabs);
        debugSplitter = new QSplitter(sourceCodeSplitter);
        debugSplitter->setObjectName(QStringLiteral("debugSplitter"));
        debugSplitter->setOrientation(Qt::Horizontal);
        debugSplitter->setChildrenCollapsible(false);
        watchList = new QListWidget(debugSplitter);
        watchList->setObjectName(QStringLiteral("watchList"));
        watchList->setContextMenuPolicy(Qt::ActionsContextMenu);
        debugSplitter->addWidget(watchList);
        stackList = new QListWidget(debugSplitter);
        stackList->setObjectName(QStringLiteral("stackList"));
        stackList->setContextMenuPolicy(Qt::ActionsContextMenu);
        debugSplitter->addWidget(stackList);
        sourceCodeSplitter->addWidget(debugSplitter);

        verticalLayout_2->addWidget(sourceCodeSplitter);

        Basic4GLEditor->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Basic4GLEditor);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 920, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuProgram = new QMenu(menuBar);
        menuProgram->setObjectName(QStringLiteral("menuProgram"));
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName(QStringLiteral("menuDebug"));
        menuSettings = new QMenu(menuBar);
        menuSettings->setObjectName(QStringLiteral("menuSettings"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        Basic4GLEditor->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Basic4GLEditor);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        mainToolBar->setIconSize(QSize(32, 32));
        Basic4GLEditor->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Basic4GLEditor);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        Basic4GLEditor->setStatusBar(statusBar);
        debugToolbar = new QToolBar(Basic4GLEditor);
        debugToolbar->setObjectName(QStringLiteral("debugToolbar"));
        debugToolbar->setEnabled(true);
        debugToolbar->setIconSize(QSize(32, 32));
        Basic4GLEditor->addToolBar(Qt::TopToolBarArea, debugToolbar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuProgram->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuBar->addAction(menuSettings->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionNew_Program);
        menuFile->addAction(actionOpen_Program);
        menuFile->addSeparator();
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addAction(actionSave_All);
        menuFile->addSeparator();
        menuFile->addAction(actionCreate_Standalone_exe);
        menuProgram->addSeparator();
        menuProgram->addAction(actionPlugin_Libraries);
        menuProgram->addAction(actionArguments);
        menuProgram->addSeparator();
        menuProgram->addAction(actionRun);
        menuDebug->addAction(actionDebug_Mode);
        menuDebug->addSeparator();
        menuDebug->addAction(actionPause);
        menuDebug->addAction(actionStep_Into);
        menuDebug->addAction(actionStep_Over);
        menuDebug->addAction(actionStep_Out_Of);
        menuDebug->addSeparator();
        menuDebug->addAction(actionToggle_Breakpoint);
        menuDebug->addAction(actionVirtual_Machine);
        menuSettings->addAction(actionBASIC_runtime_settings);
        menuHelp->addAction(actionGetting_started);
        menuHelp->addAction(actionProgrammer_s_guide);
        menuHelp->addAction(actionLanguage_guide);
        menuHelp->addAction(actionNetwork_engine_guide);
        menuHelp->addAction(actionOpenGL_guide);
        menuHelp->addAction(actionSprite_library_guide);
        menuHelp->addAction(actionSymbolic_debugger);
        menuHelp->addAction(actionCreating_standalone_exes);
        menuHelp->addAction(actionPlugin_dlls);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);
        mainToolBar->addAction(actionNew_Program);
        mainToolBar->addAction(actionOpen_Program);
        mainToolBar->addAction(actionSave);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionRun);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionDebug_Mode);
        debugToolbar->addAction(actionPause);
        debugToolbar->addAction(actionStep_Over);
        debugToolbar->addAction(actionStep_Into);
        debugToolbar->addAction(actionStep_Out_Of);

        retranslateUi(Basic4GLEditor);

        sourceTabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Basic4GLEditor);
    } // setupUi

    void retranslateUi(QMainWindow *Basic4GLEditor)
    {
        Basic4GLEditor->setWindowTitle(QApplication::translate("Basic4GLEditor", "Basic4GL - Copyright Tom Mulgrew 2003-2017", 0));
        actionOpen_Program->setText(QApplication::translate("Basic4GLEditor", "Open...", 0));
        actionOpen_Program->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+O", 0));
        actionNew_Program->setText(QApplication::translate("Basic4GLEditor", "New", 0));
        actionNew_Program->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+N", 0));
        actionSave->setText(QApplication::translate("Basic4GLEditor", "Save", 0));
        actionSave->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+S", 0));
        actionSave_As->setText(QApplication::translate("Basic4GLEditor", "Save As...", 0));
        actionCreate_Standalone_exe->setText(QApplication::translate("Basic4GLEditor", "Create Standalone .exe...", 0));
        actionArguments->setText(QApplication::translate("Basic4GLEditor", "Arguments...", 0));
        actionPlugin_Libraries->setText(QApplication::translate("Basic4GLEditor", "Plugin Libraries...", 0));
        actionRun->setText(QApplication::translate("Basic4GLEditor", "Run!", 0));
        actionRun->setShortcut(QApplication::translate("Basic4GLEditor", "F5", 0));
        actionSave_All->setText(QApplication::translate("Basic4GLEditor", "Save All", 0));
        actionSave_All->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+Shift+S", 0));
        actionToggle_Breakpoint->setText(QApplication::translate("Basic4GLEditor", "Toggle Breakpoint", 0));
        actionToggle_Breakpoint->setShortcut(QApplication::translate("Basic4GLEditor", "F9", 0));
        actionPause->setText(QApplication::translate("Basic4GLEditor", "Pause", 0));
        actionStep_Into->setText(QApplication::translate("Basic4GLEditor", "Step Into", 0));
        actionStep_Into->setShortcut(QApplication::translate("Basic4GLEditor", "F11", 0));
        actionStep_Over->setText(QApplication::translate("Basic4GLEditor", "Step Over", 0));
        actionStep_Over->setShortcut(QApplication::translate("Basic4GLEditor", "F10", 0));
        actionStep_Out_Of->setText(QApplication::translate("Basic4GLEditor", "Step Out Of", 0));
        actionStep_Out_Of->setShortcut(QApplication::translate("Basic4GLEditor", "Shift+F11", 0));
        actionDebug_Mode->setText(QApplication::translate("Basic4GLEditor", "Debug Mode", 0));
        actionDebug_Mode->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+D", 0));
        actionCreate_Edit_watch->setText(QApplication::translate("Basic4GLEditor", "Create/Edit watch", 0));
#ifndef QT_NO_TOOLTIP
        actionCreate_Edit_watch->setToolTip(QApplication::translate("Basic4GLEditor", "Create or edit watch", 0));
#endif // QT_NO_TOOLTIP
        actionCreate_Edit_watch->setShortcut(QApplication::translate("Basic4GLEditor", "Return", 0));
        actionDelete_watch->setText(QApplication::translate("Basic4GLEditor", "Delete watch", 0));
#ifndef QT_NO_TOOLTIP
        actionDelete_watch->setToolTip(QApplication::translate("Basic4GLEditor", "Delete watch", 0));
#endif // QT_NO_TOOLTIP
        actionDelete_watch->setShortcut(QApplication::translate("Basic4GLEditor", "Del", 0));
        actionAdd_watch_at_cursor->setText(QApplication::translate("Basic4GLEditor", "Add watch", 0));
        actionAdd_watch_at_cursor->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+F5", 0));
        actionShow_Gosub_Function_call->setText(QApplication::translate("Basic4GLEditor", "Show Gosub/Function call", 0));
        actionShow_Gosub_Function_call->setShortcut(QApplication::translate("Basic4GLEditor", "Return", 0));
        actionBASIC_runtime_settings->setText(QApplication::translate("Basic4GLEditor", "BASIC runtime settings...", 0));
#ifndef QT_NO_TOOLTIP
        actionBASIC_runtime_settings->setToolTip(QApplication::translate("Basic4GLEditor", "BASIC runtime settings", 0));
#endif // QT_NO_TOOLTIP
        actionGetting_started->setText(QApplication::translate("Basic4GLEditor", "Getting started", 0));
        actionProgrammer_s_guide->setText(QApplication::translate("Basic4GLEditor", "Programmer's guide", 0));
        actionLanguage_guide->setText(QApplication::translate("Basic4GLEditor", "Language guide", 0));
        actionNetwork_engine_guide->setText(QApplication::translate("Basic4GLEditor", "Network engine guide", 0));
        actionOpenGL_guide->setText(QApplication::translate("Basic4GLEditor", "OpenGL guide", 0));
        actionSymbolic_debugger->setText(QApplication::translate("Basic4GLEditor", "Symbolic debugger", 0));
        actionSprite_library_guide->setText(QApplication::translate("Basic4GLEditor", "Sprite library guide", 0));
        actionCreating_standalone_exes->setText(QApplication::translate("Basic4GLEditor", "Creating standalone exes", 0));
        actionPlugin_dlls->setText(QApplication::translate("Basic4GLEditor", "Plugin libraries", 0));
        actionAbout->setText(QApplication::translate("Basic4GLEditor", "About...", 0));
        actionFind->setText(QApplication::translate("Basic4GLEditor", "Find...", 0));
        actionFind->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+F", 0));
        actionReplace->setText(QApplication::translate("Basic4GLEditor", "Replace...", 0));
        actionReplace->setShortcut(QApplication::translate("Basic4GLEditor", "Ctrl+H", 0));
        actionFind_next->setText(QApplication::translate("Basic4GLEditor", "Find next", 0));
        actionFind_next->setShortcut(QApplication::translate("Basic4GLEditor", "F3", 0));
        actionCut->setText(QApplication::translate("Basic4GLEditor", "Cut", 0));
        actionCopy->setText(QApplication::translate("Basic4GLEditor", "Copy", 0));
        actionPaste->setText(QApplication::translate("Basic4GLEditor", "Paste", 0));
        actionVirtual_Machine->setText(QApplication::translate("Basic4GLEditor", "Virtual Machine...", 0));
        sourceTabs->setTabText(sourceTabs->indexOf(tab), QApplication::translate("Basic4GLEditor", "Tab 1", 0));
        menuFile->setTitle(QApplication::translate("Basic4GLEditor", "File", 0));
        menuProgram->setTitle(QApplication::translate("Basic4GLEditor", "Program", 0));
        menuDebug->setTitle(QApplication::translate("Basic4GLEditor", "Debug", 0));
        menuSettings->setTitle(QApplication::translate("Basic4GLEditor", "Settings", 0));
        menuHelp->setTitle(QApplication::translate("Basic4GLEditor", "Help", 0));
        mainToolBar->setWindowTitle(QApplication::translate("Basic4GLEditor", "Main", 0));
        debugToolbar->setWindowTitle(QApplication::translate("Basic4GLEditor", "Debugging", 0));
    } // retranslateUi

};

namespace Ui {
    class Basic4GLEditor: public Ui_Basic4GLEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BASIC4GLEDITOR_H
