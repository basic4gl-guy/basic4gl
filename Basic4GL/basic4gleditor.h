#ifndef BASIC4GLEDITOR_H
#define BASIC4GLEDITOR_H

#include "TomComp.h"
#include "EmbeddedFiles.h"
#include "glSpriteEngine.h"
#include "PluginDLL.h"
#include "compPreprocessor.h"
#include "sourceFileForm.h"
#include "EditorSourceFileServer.h"
#include "TomCompilerBasicLib.h"
#include "vmDebugger.h"
#include "AppSettings.h"
#include "plugindialog.h"
#include "createstandalonedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "argumentsdialog.h"
#include "finddialog.h"
#include "virtualmachineviewdialog.h"
#include "settings.h"
#include "GlfwWindowManager.h"
#include "Content2DManager.h"
#include "GlfwKeyboard.h"
#include "GlfwMouse.h"
#include <QMainWindow>
#include <QTimer>

enum class RunMode
{
	STOPPED,
	PAUSED,
	RUNNING
};

namespace Ui {
class Basic4GLEditor;
}

class Basic4GLEditor : public QMainWindow, public ISourceFileFormHost, public ISourceFileHost, public IVMLongRunningFnDoneNotified, public VMHostApplication, public IAppSettings
{
    Q_OBJECT

public:
	explicit Basic4GLEditor(QWidget *parent = 0);
	~Basic4GLEditor();

	RunMode GetRunMode() const { return runMode; }
	
	// ISourceFormHost
	void SourceIsModified() override;
	TomBasicCompiler& GetCompiler() override { return compiler; }
	void FilenameIsModified(bool isMainFile) override;
	void OpenIncludeFile(QString relativeFilename) override;

	// ISourceFileHost
	int FileCount() override;
	QString GetSourceAbsoluteFilename(int index) override;
	QString GetSourceText(int index) override;
	bool IsBreakpoint(QString absoluteFilename, int lineNumber) override;
	void DoToggleBreakpoint(int lineNumber, std::string relativeFilename);
	void ToggleBreakpoint(QString absoluteFilename, int lineNumber) override;
	bool IsInstructionPointer(QString absoluteFilename, int lineNumber) override;
	QString EvaluateVariable(QString variable, bool canCallFunctions) override;
	bool IsProgramRunning() override;
	void WriteFileHeader(QTextStream& stream) override;
	QCompleter* GetCompleter() override;
	QString GetFunctionSignature(QString function, int paramNumber) override;
	bool IsAutocompleteEnabled() override;
	bool IsFunctionSignatureEnabled() override;

	// IVMLongRunningFnDoneNotified
	void LongRunningFnDone(bool cancelled) override;

	// VMHostApplication
	bool ProcessMessages() override;
	
	// IAppSettings
	bool IsSandboxMode() override { return settings.isSandboxMode; }
	int Syntax() override { return settings.defaultSyntax; }

	// IDE command line related methods
	void OpenMainFile(QString filename);
	void RunAndQuit();
	void SetDebugMode(bool debugMode);

protected:
	void closeEvent(QCloseEvent*) override;

private slots:
	void on_actionRun_triggered();

    void on_actionSave_triggered();

    void on_actionSave_As_triggered();

	void on_actionOpen_Program_triggered();

	void on_actionNew_Program_triggered();

	void IdleTimerTimeout();
	void PollTimerTimeout();

    void on_actionSave_All_triggered();

    void on_sourceTabs_currentChanged(int index);

    void on_sourceTabs_tabCloseRequested(int index);

    void on_actionToggle_Breakpoint_triggered();

    void on_actionPause_triggered();

    void on_actionStep_Into_triggered();

    void on_actionStep_Over_triggered();

    void on_actionStep_Out_Of_triggered();

	void on_actionDebug_Mode_toggled(bool arg1);

	void on_watchList_itemDoubleClicked(QListWidgetItem *item);

    void on_actionCreate_Edit_watch_triggered();

    void on_actionDelete_watch_triggered();

    void on_actionAdd_watch_at_cursor_triggered();
	void on_stackList_itemDoubleClicked(QListWidgetItem* item);

    void on_actionShow_Gosub_Function_call_triggered();

    void on_actionPlugin_Libraries_triggered();

    void on_actionCreate_Standalone_exe_triggered();

    void on_actionBASIC_runtime_settings_triggered();

	void on_actionGetting_started_triggered();

    void on_actionProgrammer_s_guide_triggered();

    void on_actionLanguage_guide_triggered();

    void on_actionNetwork_engine_guide_triggered();

    void on_actionOpenGL_guide_triggered();

    void on_actionSymbolic_debugger_triggered();

    void on_actionSprite_library_guide_triggered();

    void on_actionCreating_standalone_exes_triggered();

    void on_actionPlugin_dlls_triggered();

    void on_actionAbout_triggered();

    void on_actionArguments_triggered();

	void on_actionFind_triggered();

	void on_actionReplace_triggered();

    void on_actionFind_next_triggered();

    void on_actionCut_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionVirtual_Machine_triggered();

private:
    Ui::Basic4GLEditor *ui;

	// Images
	QIcon iconGo;
	QIcon iconStop;
	QIcon iconPause;
	QIcon iconPlay;

	// Components
	PluginDLLManager pluginManager;
	FileOpener files;
	compPreprocessor preprocessor;
	Debugger debugger;

	// Compiler and VM
	TomVM vm;
	TomBasicCompiler compiler;

	// State
	bool isCompiled;
	RunMode	runMode;
	std::vector<SourceFileForm*> sourceFileForms;
	bool delayScreenSwitch;
	std::vector<QString> watches;
	Basic4GLSettings settings;
	bool isRunAndQuitMode;

	// OpenGL window
	GlfwWindowManager windowManager;
	Content2DManager contentManager;
	glTextGrid* glText;
	GlfwKeyboard keyboard;
	GlfwMouse mouse;

	// Timing/event handling
	QTimer idleTimer;
	QTimer pollTimer;

	// Dialogs
	PluginDialog pluginDialog;
	createstandalonedialog createStandaloneDialog;
	OptionsDialog optionsDialog;
	AboutDialog aboutDialog;
	ArgumentsDialog argumentsDialog;
	FindDialog findDialog;
	VirtualMachineViewDialog vmViewDialog;

	// Widgets
	QStringList functionNames;
	QCompleter* completer;


	void InitLibraries();
	void ShutDownLibraries();

	void UpdateUI();
	void UpdateDebugUI();

	bool Compile();
	void Run();
	void Pause();
	void Continue();
	void Stop();
	bool SetupOpenGLWindow();
	bool SetupForRun();
	void ActivateForContinue();
	void DeactivateForStop();
	void DeactivateForPause();
	void PlaceCursorAtIP();
	bool DoExecuteVMSteps(int numSteps);
	void ExecuteVMSteps(int numSteps);
	void DriveVM();

	void SwitchToFolder(QString filename);
	bool IsModified();
	bool SaveChangesAs();
	bool SaveChanges();
	bool SaveAllChanges();
	bool CheckSaveChanges();
	void DoLoadMainFile(QString newFilename);
	void Open();
	void New();
	void ReadFileHeader(QTextStream& in);
	QString DoLoad(QString absoluteFilename, bool& succeeded, bool isMainFile);

	void UpdateCompleter();
	void CloseProgram();
	void AddTab(QString filename, QString text);
	void PlaceCursor(int expandedLine, int col);
	int GetWatchIndex(QListWidgetItem* item);
	QString EvaluateWatch(QString expression, bool canCallFunctions);
	std::string InternalEvaluateWatch(std::string expression, bool canCallFunctions);
	void InternalValToString(vmValType valType, std::string& result);
	int InternalDisplayVariable(vmValType valType, std::string& result);
	std::string DisplayVariable(vmValType valType);
	void CreateOrEditWatch(QListWidgetItem* item);
	void ShowStackCall(QListWidgetItem* item);
	void ShowHelp(QString filename);
	void TextSearch(QString findText, bool isMatchCase, bool isWholeWordsOnly);
	void TextReplace(QString findText, QString replaceText, bool isReplaceAll, bool isMatchCase, bool isWholeWordsOnly);
	QStringList GetFunctionNames();
	QString DescribeType(vmValType type);
	QString DescribeBasicType(vmBasicValType type);
};

#endif // BASIC4GLEDITOR_H
