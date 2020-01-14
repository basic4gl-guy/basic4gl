#include "basic4gleditor.h"
#include "ui_basic4gleditor.h"
#include "EditorSourceFileServer.h"
#include "basic4glMisc.h"
#include "diskfile.h"
#include "plugindialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QInputDialog>
#include <QProcess>
#include <QStringListModel>

// Libraries
#include "TomWindowsBasicLib.h"
#include "TomStdBasicLib.h"
#include "TomOpenGLBasicLib.h"
#include "GLBasicLib_gl.h"
#include "GLBasicLib_glu.h"
#include "TomTextBasicLib.h"
#include "TomJoystickBasicLib.h"
#include "TomTrigBasicLib.h"
#include "TomFileIOBasicLib.h"
#include "TomNetBasicLib.h"
#include "TomSoundBasicLib.h"

const int VMSTEPS = 1000;

// Return the application settings (declared in AppSettings.h)
IAppSettings* appSettings = nullptr;
IAppSettings* GetAppSettings() {
	return appSettings;
}

// Basic4GLEditor methods

Basic4GLEditor::Basic4GLEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Basic4GLEditor),
	pluginManager(CppString(qApp->applicationDirPath()), false),
	preprocessor(2, new EditorSourceFileServer(this), new DiskFileServer()),
	debugger(preprocessor.LineNumberMap()),
	vm(pluginManager, debugger),
	compiler(vm, pluginManager),
	isCompiled(false),
	runMode(RunMode::STOPPED),
	delayScreenSwitch(false),
	isRunAndQuitMode(false),
	windowManager(),
	contentManager(windowManager),
	keyboard(windowManager),
	mouse(windowManager),
	glText(nullptr),
	idleTimer(this),
	pollTimer(this),
	pluginDialog(pluginManager, this),
	createStandaloneDialog(CppString(qApp->applicationDirPath()), this),
	optionsDialog(this),
	aboutDialog(this),
	argumentsDialog(this),
	findDialog(this),
	vmViewDialog(compiler, this),
	completer(nullptr)
{
	if (windowManager.Error())
	{
		ShowMessage(windowManager.GetError());
		exit(0);
	}

    ui->setupUi(this);
	
	// Setup UI
	ui->sourceCodeSplitter->setSizes(QList<int>() << 500 << 75);
    ui->debugSplitter->setSizes(QList<int>() << 500 << 100);
	ui->debugToolbar->hide();
	ui->debugSplitter->hide();
	ui->watchList->addItem("");
	ui->watchList->addAction(ui->actionCreate_Edit_watch);
	ui->watchList->addAction(ui->actionDelete_watch);
	ui->stackList->addAction(ui->actionShow_Gosub_Function_call);
	iconGo.addFile(":/icons/Images/Go.png");
	iconStop.addFile(":/icons/Images/Stop.png");
	iconPause.addFile(":/icons/Images/Pause.png");
	iconPlay.addFile(":/icons/Images/Play.png");

	// Set initial directory to programs
	QDir::setCurrent(FolderSettings::GetDefaultProgramsFolder());

	// Initialise compiler and VM
	InitLibraries();

	// Create autocompleter using library function names
	functionNames = GetFunctionNames();
	completer = new QCompleter(functionNames, this);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

	// Setup timers
	connect(&idleTimer, SIGNAL(timeout()), this, SLOT(IdleTimerTimeout()));
	connect(&pollTimer, SIGNAL(timeout()), this, SLOT(PollTimerTimeout()));
	pollTimer.start(200);

	// Create initial empty program
	New();

	// Register self is IAppSettings
	appSettings = this;
}

Basic4GLEditor::~Basic4GLEditor()
{
	assert(runMode == RunMode::STOPPED);
	appSettings = nullptr;
	ShutDownLibraries();
    delete ui;
}

void Basic4GLEditor::SourceIsModified()
{
	UpdateUI();
}

void Basic4GLEditor::FilenameIsModified(bool isMainFile)
{
	// If main file is modified, switch to corresponding directory
	if (isMainFile)
	{
		QString filename = sourceFileForms[0]->GetAbsoluteFilename();
		SwitchToFolder(filename);
	}

	// UI update required to set tab text
	UpdateUI();
}

int Basic4GLEditor::FileCount()
{
	return sourceFileForms.size();
}

QString Basic4GLEditor::GetSourceAbsoluteFilename(int index)
{
	return sourceFileForms[index]->GetAbsoluteFilename();
}

QString Basic4GLEditor::GetSourceText(int index)
{
	return sourceFileForms[index]->GetSourceText();
}

bool Basic4GLEditor::IsBreakpoint(QString absoluteFilename, int lineNumber)
{
	// Often will get a bunch of IsBreakpoint calls for the same absolute filename.
	// Streamline for this case by storing and reusing the corresponding relative filename,
	// rather than recalculating it each time.
	static QString prevAbsolute;
	static std::string prevRelative;

	std::string relativeFilename;
	if (absoluteFilename == prevAbsolute)
	{
		relativeFilename = prevRelative;
	}
	else
	{
		relativeFilename = ProcessPath(CppString(absoluteFilename));
		prevAbsolute = absoluteFilename;
		prevRelative = relativeFilename;
	}

	return debugger.IsUserBreakPt(relativeFilename, lineNumber);
}

void Basic4GLEditor::DoToggleBreakpoint(int lineNumber, std::string relativeFilename)
{
	debugger.ToggleUserBreakPt(relativeFilename, lineNumber);
	if (runMode == RunMode::RUNNING)
		vm.RepatchBreakpts();
}

void Basic4GLEditor::ToggleBreakpoint(QString absoluteFilename, int lineNumber)
{
	std::string relativeFilename = ProcessPath(CppString(absoluteFilename));
	DoToggleBreakpoint(lineNumber, relativeFilename);
}

bool Basic4GLEditor::IsInstructionPointer(QString absoluteFilename, int lineNumber)
{
	if (runMode == RunMode::RUNNING || !vm.IPValid()) return false;

	std::string relativeFilename = ProcessPath(CppString(absoluteFilename));
	int expandedLine, col;
	vm.GetIPInSourceCode(expandedLine, col);
	std::string ipFilename;
	int ipLineNumber;
	preprocessor.LineNumberMap().SourceFromMain(ipFilename, ipLineNumber, expandedLine);

	return ipFilename == relativeFilename && ipLineNumber == lineNumber;
}

QString Basic4GLEditor::EvaluateVariable(QString variable, bool canCallFunctions)
{
	return EvaluateWatch(variable, canCallFunctions);
}

bool Basic4GLEditor::IsProgramRunning()
{
	return runMode == RunMode::RUNNING;
}

void Basic4GLEditor::WriteFileHeader(QTextStream& stream)
{
	for (auto dll : pluginManager.LoadedDLLs())
	{
		stream << "#plugin " << dll->Filename().c_str() << endl;
	}
}

QCompleter* Basic4GLEditor::GetCompleter()
{
	return completer;
}

QString Basic4GLEditor::GetFunctionSignature(QString function, int paramNumber)
{
	std::string functionStr = CppString(function);
	auto& functionIndex = compiler.FunctionIndex();

	// Find function specifications.
	// Copied from compiler code
	compExtFuncSpec functions[TC_MAXOVERLOADEDFUNCTIONS];
	int functionCount = 0;

	// Find builtin functions
	compFuncIndex::iterator it;
	for (it = functionIndex.find(functionStr);
		it != functionIndex.end() && (*it).first == functionStr && functionCount < TC_MAXOVERLOADEDFUNCTIONS;
		it++) {
		compFuncSpec& spec = compiler.Functions()[(*it).second];                        // Get specification
		functions[functionCount].m_spec = &spec;
		functions[functionCount].m_builtin = true;
		functionCount++;
	}

	// Find plugin DLL functions
	pluginManager.FindFunctions(functionStr, functions, functionCount, TC_MAXOVERLOADEDFUNCTIONS);

	if (functionCount == 0) return "";

	// Describe signatures
	QString signature = "";
	for (int i = 0; i < functionCount; i++)
	{
		auto spec = functions[i].m_spec;
		if (signature != "") signature += "<br/>\n";
		signature += "<nobr>";
		signature += QString("<span style=\"color:blue\">") + (spec->m_isFunction ? "function" : "sub") + "</span> ";
		signature += "<span style=\"color:red\">" + QString::fromStdString(functionStr).toHtmlEscaped() + "</span>";
		signature += "(";
		int signatureParamNumber = 0;
		for (auto const& p : spec->m_paramTypes.Params())
		{
			if (signatureParamNumber > 0) signature += ", ";
			if (signatureParamNumber == paramNumber) signature += "<b>";
			signature += DescribeType(p);
			if (signatureParamNumber == paramNumber) signature += "</b>";
			signatureParamNumber++;
		}
		signature += ")";
		if (spec->m_isFunction)
		{
			signature += " <span style=\"color:blue\">as</span> " + DescribeType(spec->m_returnType);
		}

		// Nasty hack to prevent word wrapping.
		// Tooltip width appears to be calculated based on width of the topmost line, so add some
		// extra spaces to ensure subsequent lines aren't longer.
		//if (i == 0)
		//	for (int j = 0; j < 10; j++)
		//		signature += "&nbsp;";

		signature += "</nobr>";
	}

	return signature;
}

bool Basic4GLEditor::IsAutocompleteEnabled()
{
	return settings.text.isAutocompleteEnabled;
}

bool Basic4GLEditor::IsFunctionSignatureEnabled()
{
	return settings.text.isFunctionSignatureEnabled;
}

void Basic4GLEditor::LongRunningFnDone(bool cancelled)
{
	// Restart idle timer after long running function completes
	if (runMode == RunMode::RUNNING && !idleTimer.isActive())
		idleTimer.start();
}

bool Basic4GLEditor::ProcessMessages()
{
	// TODO: How to handle this???
	return true;
}

void Basic4GLEditor::OpenIncludeFile(QString relativeFilename)
{
	// Check if file is already open
	std::string relFilename = ProcessPath(CppString(relativeFilename));
	for (int i = 0; i < sourceFileForms.size(); i++) 
	{
		if (ProcessPath(CppString(sourceFileForms[i]->GetAbsoluteFilename())) == relFilename)
		{
			// Switch to open tab
			ui->sourceTabs->setCurrentIndex(i);
			return;
		}
	}

	// Attempt to load file.
	// If fails, assume file is missing and proceed anyway (will open a tab with no text yet)
	QFileInfo fileInfo(relativeFilename);
	QString absoluteFilename = fileInfo.absoluteFilePath();
	bool succeeded;
	QString text = DoLoad(absoluteFilename, succeeded, false);

	AddTab(absoluteFilename, text);
	ui->sourceTabs->setCurrentIndex(sourceFileForms.size() - 1);			// Switch to new tab
	UpdateUI();
}

void Basic4GLEditor::OpenMainFile(QString filename)
{
	if (!CheckSaveChanges()) return;
	DoLoadMainFile(filename);
}

void Basic4GLEditor::RunAndQuit()
{
	if (!Compile())
	{
		show();								//  Show editor to display compiler error
		return;
	}

	Run();
	if (runMode != RunMode::RUNNING) {
		show();								// Show editor to display run error
		return;
	}

	// Flag that we are in run-and-quit mode
	isRunAndQuitMode = true;
}

void Basic4GLEditor::SetDebugMode(bool debugMode)
{
	// Show/hide debug tools
	ui->actionVirtual_Machine->setVisible(debugMode);
}

void Basic4GLEditor::closeEvent(QCloseEvent* event)
{
	// Ensure program has stopped
	if (runMode != RunMode::STOPPED)
		Stop();

	// Check for any unsaved changes
	if (CheckSaveChanges())
	{
		event->accept();
	}
	else
	{
		// Cancel event if user aborts
		event->ignore();
	}
}

void Basic4GLEditor::on_actionRun_triggered()
{
	switch (runMode)
	{
	case RunMode::STOPPED:
		if (Compile())
			Run();
		break;
	case RunMode::RUNNING:
	case RunMode::PAUSED:
		Stop();
		break;
	}
}

void Basic4GLEditor::on_actionSave_triggered()
{
	SaveChanges();
}

void Basic4GLEditor::on_actionSave_As_triggered()
{
	SaveChangesAs();
}

void Basic4GLEditor::on_actionOpen_Program_triggered()
{
	Open();
}

void Basic4GLEditor::on_actionNew_Program_triggered()
{
	New();
}

void Basic4GLEditor::IdleTimerTimeout()
{
	pluginManager.ProcessMessages();
	DriveVM();
}

void Basic4GLEditor::PollTimerTimeout()
{
	// Perform some OpenGL window maintenance every 200ms
	if (runMode != RunMode::RUNNING) {

		// Allow window to be closed
		if (windowManager.IsCloseRequested())
		{
			if (runMode == RunMode::PAUSED)
			{
				Stop();
			}

			windowManager.DestroyWindow();

			// Run and quit mode?
			if (isRunAndQuitMode)
				qApp->quit();
		}
	}
}

void Basic4GLEditor::InitLibraries()
{
	// Create OpenGL text grid
	QString filename = qApp->applicationDirPath() + "/charset.png";
	glText = new glTextGrid(
		CppString(filename).c_str(),
		&files,
		25,
		40,
		16,
		16);
	if (glText->Error())
	{
		ShowMessage(glText->GetError());
		exit(1);			// Can't use qApp->exit yet. Event loop hasn't started.
	}

	// Plug in constant and function libraries
	InitTomStdBasicLib(compiler);							// Standard library
	InitTomWindowsBasicLib(compiler, &files, &windowManager);		// Windows specific library
	InitTomOpenGLBasicLib(compiler, &files, &windowManager);		// OpenGL
	InitTomTextBasicLib(compiler, glText, &windowManager, &contentManager, &keyboard, &mouse);		// Basic text/sprites
	InitGLBasicLib_gl(compiler);
	InitGLBasicLib_glu(compiler);
	InitTomJoystickBasicLib(compiler, &keyboard);           // Joystick support
	InitTomTrigBasicLib(compiler);							// Trigonometry library
	InitTomFileIOBasicLib(compiler, &files);				// File I/O library
	InitTomNetBasicLib(compiler);							// Networking
	InitTomSoundBasicLib(compiler, &files);					// OpenAL + Ogg Vorbis sound library
	InitTomCompilerBasicLib(compiler, this, &files);

	vm.SetLongRunningFnDoneNotified(this);
}

void Basic4GLEditor::ShutDownLibraries()
{
	vm.Clr();
	pluginManager.Clear();				// Shut down libraries before destroying OpenGL window, just in case.

	if (glText)
	{
		delete glText;
		glText = nullptr;
	}
}

bool Basic4GLEditor::Compile()
{
	isCompiled = false;

	// Load in source code
	compiler.Parser().SourceCode().clear();
	if (!preprocessor.Preprocess(
		new EditorSourceFile(ProcessPath(CppString(GetSourceAbsoluteFilename(0))), GetSourceText(0)),
		compiler.Parser()))
	{
		PlaceCursor(compiler.Parser().SourceCode().size() - 1, 0);
		ui->statusBar->showMessage(preprocessor.GetError().c_str());
		return false;
	}

	// Set syntax
	compiler.DefaultSyntax() = static_cast<compLanguageSyntax>(settings.defaultSyntax);

	// Compile
	compiler.ClearError();
	compiler.Compile();

	if (compiler.Error())
	{
		PlaceCursor(compiler.Line(), compiler.Col());
		ui->statusBar->showMessage(compiler.GetError().c_str());
		return false;
	}	
	
	isCompiled = true;
	return true;
}

void Basic4GLEditor::Run()
{
	vm.Reset();
	if (!SetupForRun()) return;
	runMode = RunMode::RUNNING;

	UpdateUI();
	ui->statusBar->showMessage("Running...");
}

void Basic4GLEditor::Pause()
{
	DeactivateForPause();
	runMode = RunMode::PAUSED;

	activateWindow();
	UpdateUI();
	PlaceCursorAtIP();
	ui->statusBar->showMessage("Paused");
}

void Basic4GLEditor::Continue()
{
	delayScreenSwitch = true;
	ActivateForContinue();
	runMode = RunMode::RUNNING;

	UpdateUI();
	ui->statusBar->showMessage("Running...");
}

void Basic4GLEditor::Stop()
{
	DeactivateForStop();
	runMode = RunMode::STOPPED;
	PlaceCursorAtIP();

	UpdateUI();
	ui->statusBar->showMessage("Stopped");
}

bool Basic4GLEditor::SetupOpenGLWindow()
{
	// Set initial window parameters from settings
	windowManager.pendingParams.isFullscreen = settings.window.isFullScreen;
	windowManager.pendingParams.width = settings.window.GetXResolution();
	windowManager.pendingParams.height = settings.window.GetYResolution();
	windowManager.pendingParams.bpp = settings.window.GetBPP();
	windowManager.pendingParams.isBordered = settings.window.windowBorder;
	windowManager.pendingParams.isResizable = settings.window.allowResizing;
	windowManager.pendingParams.isStencilBufferRequired = settings.window.stencil;
	windowManager.pendingParams.title = "Basic4GL";

	// Create window
	windowManager.RecreateWindow();
	if (windowManager.Error())
	{
		ShowMessage(windowManager.GetError());
		return false;
	}

	return true;

	//bool wasWindowUpdated = IsWindowUpdated();
	//SetWindowVariables(
	//	true,
	//	settings.window.isFullScreen,
	//	settings.window.windowBorder,
	//	settings.window.GetXResolution(),
	//	settings.window.GetYResolution(),
	//	settings.window.GetBPP(),
	//	settings.window.stencil,
	//	"Basic4GL",
	//	settings.window.allowResizing,
	//	settings.window.FitToWorkArea());

	//if (wasWindowUpdated)
	//{
	//	RecreateGLWindow();
	//}
	//else {
	//	glWindow->ResetGL();
	//	glWindow->Activate();
	//	glWindow->OpenGLDefaults();
	//}

	//glWindow->SetClosing(false);

}

bool Basic4GLEditor::SetupForRun()
{
	// Reset OpenGL window
	if (!SetupOpenGLWindow()) return false;

	// Notify libraries
	pluginManager.ProgramStart();

	// Start timers
	idleTimer.start();

	return true;
}

void Basic4GLEditor::ActivateForContinue()
{
	if (delayScreenSwitch) {
		// Don't show OpenGL window initially.
		// Wait to see if program stops before 1000 op codes are executed.
		// This makes single stepping less flickery.

		// Notify libraries
		pluginManager.ProgramResume();
	}
	else
	{		
		// Show OpenGL window
		windowManager.ActivateWindow();
		pluginManager.ProgramDelayedResume();
	}

	idleTimer.start();
}

void Basic4GLEditor::DeactivateForStop()
{
	// Clear stepping breakpoints
	vm.ClearTempBreakPts();

	// Notify libraries
	pluginManager.ProgramEnd();
	StopTomSoundBasicLib();
	glEnd();

	idleTimer.stop();
}

void Basic4GLEditor::DeactivateForPause()
{
	// Clear stepping breakpoints
	vm.ClearTempBreakPts();

	// Hide OpenGL window if in fullscreen
	windowManager.DeactivateWindow();

	idleTimer.stop();
}

void Basic4GLEditor::PlaceCursorAtIP()
{
	if (vm.IPValid()) {
		int expandedLine, col;
		vm.GetIPInSourceCode(expandedLine, col);
		PlaceCursor(expandedLine, col);
	}
}

bool Basic4GLEditor::DoExecuteVMSteps(int numSteps)
{
	__try {
		vm.Continue(numSteps);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		switch (GetExceptionCode()) {

			// Skip mathematics errors (overflows, divide by 0 etc).
			// This is quite important!, as some OpenGL drivers will trigger
			// divide-by-zero and other conditions if geometry happens to
			// be aligned in certain ways. The appropriate behaviour is to
			// ignore these errors, and keep running, and NOT to stop the
			// program!
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
			vm.SkipInstruction();
			break;

			// All other exceptions will stop the program.
		default:
			return false;
		}
	}

	return true;
}

void Basic4GLEditor::ExecuteVMSteps(int numSteps)
{
	if (!DoExecuteVMSteps(numSteps))
	{
		vm.MiscError("An exception occurred");
	}
}

void Basic4GLEditor::DriveVM()
{
	if (runMode != RunMode::RUNNING) return;

	// If VM can wait for an event, do nothing
	if (vm.CanWaitForEvents()) {
		idleTimer.stop();				// Stop polling this method. Otherwise we end up with a 100% CPU burn loop.
		return;
	}

	// Execute a single step before patching in breakpoints.
	// Otherwise if virtual machine is sitting on a breakpoint it will never get past it.
	ExecuteVMSteps(1);

	// Drive the virtual machine
	if (!vm.Error() && !vm.Done()) {
		vm.PatchIn();
		ExecuteVMSteps(VMSTEPS);
	}

	// Check for error/done
	if (vm.Error() || vm.Done())
	{
		// Activate editor window for errors.
		// Leave output window active for regular completion.
		if (vm.Error()) {
			if (isRunAndQuitMode) {
				show();							// Editor is hidden in run-and-quit mode. Make visible so we can show error location.
				isRunAndQuitMode = false;		// Switch out of run-and-quit mode.
			}
			activateWindow();
		}

		// Place cursor
		DeactivateForStop();
		runMode = RunMode::STOPPED;
		PlaceCursorAtIP();
		ui->statusBar->showMessage(vm.Error() ? vm.GetError().c_str() : "Program completed");
		UpdateUI();
	}
	else if (windowManager.IsCloseRequested())
	{
		Stop();
		windowManager.DestroyWindow();

		// Run and quit mode?
		if (isRunAndQuitMode)
			qApp->quit();
	}
	else if (vm.Paused() || keyboard.IsPausePressed())
	{
		// Paused, either manually, for breakpoint or end of step operation
		Pause();		

		if (isRunAndQuitMode)
		{
			show();								// Switch out of run-and-quit mode and show editor
			isRunAndQuitMode = false;
		}
	}
	else if (delayScreenSwitch)
	{
		// 1000 op codes executed without program stopping/pausing
		// Activate OpenGL window
		delayScreenSwitch = false;
		ActivateForContinue();
	}
}

void Basic4GLEditor::SwitchToFolder(QString filename)
{
	// Set current folder to that containing the file
	QFileInfo fileInfo(filename);
	QString path = fileInfo.absolutePath();
	QDir::setCurrent(path);
	settings.folder.saveLoadDefaultFolder = path;
}

bool Basic4GLEditor::IsModified()
{
	// Check each source form
	for (auto* form : sourceFileForms)
		if (form->IsModified())
			return true;
	return false;
}

bool Basic4GLEditor::SaveChangesAs()
{
	int index = ui->sourceTabs->currentIndex();
	return sourceFileForms[index]->SaveAs();
}

bool Basic4GLEditor::SaveChanges()
{
	int index = ui->sourceTabs->currentIndex();
	return sourceFileForms[index]->Save();
}

bool Basic4GLEditor::SaveAllChanges()
{
	for (int i = 0; i < sourceFileForms.size(); i++) {
		if (sourceFileForms[i]->IsModified())
		{
			ui->sourceTabs->setCurrentIndex(i);
			if (!sourceFileForms[i]->Save())
				return false;
		}
	}

	return true;
}

bool Basic4GLEditor::CheckSaveChanges()
{
	// Check for unsaved changes.
	// Return true if user is okay to continue.
	if (!IsModified()) return true;				// No changes to save

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Unsaved changes", "Save your changes first?",
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	switch (reply)
	{
	case QMessageBox::Yes:
		return SaveAllChanges();				// Proceed if changes were saved
	case QMessageBox::No:
		return true;							// Proceed without saving
	default:
		return false;							// Don't proceed
	}
}

void Basic4GLEditor::DoLoadMainFile(QString newFilename)
{
	// Close previous program.
	// Must be done before attempting to load, to ensure plugins are unloaded,
	// as loading process will load plugins for new program.
	CloseProgram();

	// Attempt to load
	bool succeeded;
	QString fileText = DoLoad(newFilename, succeeded, true);
	if (!succeeded)
	{
		// Add a blank tab, otherwise there won't be any
		AddTab("", "");
		QMessageBox::critical(this, "Error", "Could not open file:\n" + newFilename);
	}
	else {
		// Create single tab for loaded program.
		SwitchToFolder(newFilename);
		AddTab(newFilename, fileText);
	}

	UpdateUI();
}

void Basic4GLEditor::Open()
{
	// Save changes to existing program first
	if (!CheckSaveChanges()) return;

	// Prompt for file
	QString newFilename = QFileDialog::getOpenFileName(
		this,
		tr("Open Basic4GL Program"),
		settings.folder.saveLoadDefaultFolder,
		tr("Basic4GL program (*.gb *.inc);;Text file (*.txt);;All files (*.*)"));
	if (newFilename.isEmpty()) return;

	DoLoadMainFile(newFilename);
}

void Basic4GLEditor::New()
{
	if (!CheckSaveChanges()) return;
	
	// Clear source
	CloseProgram();
	AddTab("", "");

	// Set default folder back to programs
	QDir::setCurrent(FolderSettings::GetDefaultProgramsFolder());

	UpdateUI();
}

void Basic4GLEditor::ReadFileHeader(QTextStream& in)
{
	// File header consists of #plugin declarations
	auto pos = in.pos();			// Track start of line in case we have to back-up
	auto line = in.readLine();
	while (line.startsWith("#plugin "))
	{
		// Get plugin filename
		auto filename = CppString(line.mid(8));

		// Attempt to load it
		if (!pluginManager.LoadDLL(filename))
		{
			ShowMessage("Error loading plugin " + filename + ": " + pluginManager.Error());
		}

		// Read next line
		pos = in.pos();
		line = in.readLine();
	}

	// Last line is NOT part of the header.
	// Backup read position
	in.seek(pos);

	// Update autocomplete items in case plugins were loaded
	UpdateCompleter();
}

QString Basic4GLEditor::DoLoad(QString absoluteFilename, bool& succeeded, bool isMainFile)
{
	// Attempt to load
	QFile file(absoluteFilename);
	if (!file.open(QIODevice::ReadOnly)) {
		succeeded = false;
		return "";
	}

	// Load into new source file form
	QTextStream in(&file);
	if (isMainFile)
	{
		ReadFileHeader(in);
	}

	QString fileText = in.read(0x7fffffff);		// Assuming BASIC source text is shorter than 2^32 characters.
	file.close();

	succeeded = true;
	return fileText;
}

void Basic4GLEditor::CloseProgram()
{
	// Close and unload all tabs
	ui->sourceTabs->clear();
	sourceFileForms.clear();

	// Unload all DLLs
	pluginManager.Clear();
	pluginDialog.Invalidate();

	// Clear breakpoints
	debugger.ClearUserBreakPts();
	isCompiled = false;

	// Update completer as plugin functions/constants may no longer be available 
	UpdateCompleter();
}

void Basic4GLEditor::AddTab(QString filename, QString text)
{
	SourceFileForm* form = new SourceFileForm(this, sourceFileForms.empty(), filename, text);
	form->AddTextAction(ui->actionCut);
	form->AddTextAction(ui->actionCopy);
	form->AddTextAction(ui->actionPaste);
	form->AddTextAction(ui->actionFind);
	form->AddTextAction(ui->actionFind_next);
	form->AddTextAction(ui->actionReplace);
	form->AddTextAction(ui->actionToggle_Breakpoint);
	form->AddTextAction(ui->actionAdd_watch_at_cursor);
	form->AddTextAction(ui->actionRun);
	ui->sourceTabs->addTab(form, form->GetTabText());
	sourceFileForms.push_back(form);
}

void Basic4GLEditor::PlaceCursor(int expandedLine, int col)
{
	// Lookup source file and line
	std::string filename;
	int line;
	preprocessor.LineNumberMap().SourceFromMain(filename, line, expandedLine);

	if (line >= 0)
	{
		// Ensure file is open
		OpenIncludeFile(filename.c_str());

		// Place cursor
		sourceFileForms[ui->sourceTabs->currentIndex()]->PlaceCursor(line, col);
	}
}

int Basic4GLEditor::GetWatchIndex(QListWidgetItem* item)
{
	int index = GetListItemIndex(ui->watchList, item);

	// Note: Last watch item is dummy blank one (can be double clicked to create a new item).
	// Treat it as "no item clicked"
	if (index == ui->watchList->count() - 1)
		index = -1;

	return index;
}

QString Basic4GLEditor::EvaluateWatch(QString expression, bool canCallFunctions)
{
	if (!isCompiled || runMode == RunMode::RUNNING) return "???";

	// Save virtual machine state
	vmState state = vm.GetState();
	RunMode saveMode = runMode;

	// Evaluate watch
	QString result = InternalEvaluateWatch(CppString(expression), canCallFunctions).c_str();

	// Restore VM state
	vm.SetState(state);
	runMode = saveMode;
	
	return result;
}

std::string Basic4GLEditor::InternalEvaluateWatch(std::string expression, bool canCallFunctions)
{
	// Setup compiler "in function" state to match the the current VM user
	// stack state.
	int currentFunction;
	if (vm.CurrentUserFrame() < 0 ||
		vm.UserCallStack().back().userFuncIndex < 0)
		currentFunction = -1;
	else
		currentFunction = vm.UserCallStack()[vm.CurrentUserFrame()].userFuncIndex;

	bool inFunction = currentFunction >= 0;

	// Compile watch expression
	// This also gives us the expression result type
	unsigned int codeStart = vm.InstructionCount();
	vmValType valType;
	if (!compiler.TempCompileExpression(expression, valType, inFunction, currentFunction))
		return compiler.GetError();

	if (!canCallFunctions)
		// Expressions aren't allowed to call functions for mouse-over hints.
		// Scan compiled code for OP_CALL_FUNC or OP_CALL_OPERATOR_FUNC
		for (int i = codeStart; i < vm.InstructionCount(); i++)
			if (vm.Instruction(i).m_opCode == OP_CALL_FUNC
				//|| vm.Instruction(i).m_opCode == OP_CALL_OPERATOR_FUNC
				|| vm.Instruction(i).m_opCode == OP_CALL_DLL
				|| vm.Instruction(i).m_opCode == OP_CREATE_USER_FRAME)
				return "Mouse hints can't call functions. Use watch instead.";

	// Run compiled code
	vm.GotoInstruction(codeStart);
	runMode = RunMode::RUNNING;
	int iterations = 0;
	const int maxIterations = 10;
	do {

		// Run the virtual machine for a certain number of steps
		DoExecuteVMSteps(VMSTEPS);
		iterations++;

	} while (runMode == RunMode::RUNNING
		&& !vm.Error()
		&& !vm.Done()
		&& !vm.Paused()
		&& iterations < maxIterations);

	// Error occurred?
	if (vm.Error())
		return vm.GetError();

	// Execution didn't finish?
	if (!vm.Done())
		return "???";

	// Convert expression result to string
	return DisplayVariable(valType);
}

void Basic4GLEditor::InternalValToString(vmValType valType, std::string& result)
{
	int maxChars = VM_DATATOSTRINGMAXCHARS;
	result = vm.ValToString(vm.Reg(), valType, maxChars);
}

int Basic4GLEditor::InternalDisplayVariable(vmValType valType, std::string& result)
{
	// Note: All these nested "internal" methods and magic numbers are to keep std::string
	// objects out of the method containing __try/__except. 
	// Otherwise it will not compile (complains that SEH cannot mix with C++ object unwinding)
	__try {
		InternalValToString(valType, result);
		return 0;							// Value calculated
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

		// Floating point errors can be raised when converting floats to string
		switch (GetExceptionCode()) {
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
			return 1;						// Floating point exception
		default:
			return 2;						// An exception occurred
		}
	}
}

std::string Basic4GLEditor::DisplayVariable(vmValType valType)
{
	if (valType == VTP_STRING)                                  // String is special case.
	{
		return "\"" + vm.RegString() + "\"";                 // Stored in string register.
	}
	else
	{
		std::string result;
		int code = InternalDisplayVariable(valType, result);
		switch (code)
		{
		case 0: return result;
		case 1: return "Floating point exception";
		default: return "An exception occurred";
		}
	}
}

void Basic4GLEditor::UpdateUI()
{
	// Set tab text
	for (int i = 0; i < sourceFileForms.size(); i++) {
		ui->sourceTabs->setTabText(i, sourceFileForms[i]->GetTabText());
		sourceFileForms[i]->RedrawLineNumberArea();
	}

	// Configure actions
	switch (runMode)
	{
	case RunMode::STOPPED:
		for (auto* form : sourceFileForms) form->SetReadonly(false);
		ui->actionRun->setText("Run");
		ui->actionRun->setIcon(iconGo);
		ui->actionRun->setShortcut(Qt::Key_F5);
		ui->actionPause->setText("Pause");
		ui->actionPause->setIcon(iconPause);
		ui->actionPause->setShortcut(QKeySequence());
		ui->actionNew_Program->setEnabled(true);
		ui->actionOpen_Program->setEnabled(true);
		ui->actionPause->setEnabled(false);
		ui->actionStep_Over->setEnabled(false);
		ui->actionStep_Into->setEnabled(false);
		ui->actionStep_Out_Of->setEnabled(false);
		ui->actionPlugin_Libraries->setEnabled(true);
		ui->actionBASIC_runtime_settings->setEnabled(true);
		break;

	case RunMode::RUNNING:
		for (auto* form : sourceFileForms) form->SetReadonly(true);
		ui->actionRun->setText("Stop");
		ui->actionRun->setIcon(iconStop);
		ui->actionRun->setShortcut(QKeySequence());
		ui->actionPause->setText("Pause");
		ui->actionPause->setIcon(iconPause);
		ui->actionPause->setShortcut(QKeySequence());
		ui->actionNew_Program->setEnabled(false);
		ui->actionOpen_Program->setEnabled(false);
		ui->actionPause->setEnabled(true);
		ui->actionStep_Over->setEnabled(false);
		ui->actionStep_Into->setEnabled(false);
		ui->actionStep_Out_Of->setEnabled(false);
		ui->actionPlugin_Libraries->setEnabled(false);
		ui->actionBASIC_runtime_settings->setEnabled(false);
		break;

	case RunMode::PAUSED:
		for (auto* form : sourceFileForms) form->SetReadonly(true);
		ui->actionRun->setText("Stop");
		ui->actionRun->setIcon(iconStop);
		ui->actionRun->setShortcut(QKeySequence());
		ui->actionPause->setText("Continue");
		ui->actionPause->setIcon(iconPlay);
		ui->actionPause->setShortcut(Qt::Key_F5);
		ui->actionNew_Program->setEnabled(false);
		ui->actionOpen_Program->setEnabled(false);
		ui->actionPause->setEnabled(true);
		ui->actionStep_Over->setEnabled(true);
		ui->actionStep_Into->setEnabled(true);
		ui->actionStep_Out_Of->setEnabled(true);
		ui->actionPlugin_Libraries->setEnabled(false);
		ui->actionBASIC_runtime_settings->setEnabled(false);
		break;
	}

	ui->actionSave_All->setEnabled(IsModified());

	// Save action is based on the currently selected source file form
	int index = ui->sourceTabs->currentIndex();
	ui->actionSave->setEnabled(index >= 0 && index < sourceFileForms.size() && sourceFileForms[index]->IsModified());

	// Update debugging UI
	UpdateDebugUI();
}

void Basic4GLEditor::UpdateDebugUI()
{
	if (!ui->actionDebug_Mode->isChecked()) return;

	// Reevaluate watches
	for (int i = 0; i < watches.size(); i++)
	{
		ui->watchList->item(i)->setText(watches[i] + ": " + EvaluateWatch(watches[i], true));
	}

	// Populate callstack listbox
	ui->stackList->clear();
	if (runMode == RunMode::PAUSED) {
		ui->stackList->addItem("IP");
		auto& callstack = vm.UserCallStack();
		for (int i = 0; i < callstack.size(); i++)
		{
			auto& frame = callstack[callstack.size() - i - 1];

			// User functions have positive indices.
			std::string text = frame.userFuncIndex >= 0
				? compiler.GetUserFunctionName(frame.userFuncIndex) + "()"
				: "gosub " + compiler.DescribeStackCall(frame.returnAddr);
			ui->stackList->addItem(text.c_str());
		}
	}
}

void Basic4GLEditor::on_actionSave_All_triggered()
{
	SaveAllChanges();
}

void Basic4GLEditor::on_sourceTabs_currentChanged(int index)
{
	UpdateUI();
}

void Basic4GLEditor::on_sourceTabs_tabCloseRequested(int index)
{
	// Special case, closing tab 0 => New
	if (index == 0)
	{
		New();
	}
	else
	{
		// Close clicked tab
		if (sourceFileForms[index]->CheckSaveChanges()) {
			ui->sourceTabs->removeTab(index);
			sourceFileForms.erase(sourceFileForms.begin() + index);
		}
	}
}

void Basic4GLEditor::on_actionToggle_Breakpoint_triggered()
{
	auto form = sourceFileForms[ui->sourceTabs->currentIndex()];
	std::string filename = ProcessPath(CppString(form->GetAbsoluteFilename()));
	int line = form->CursorLine();
	DoToggleBreakpoint(line, filename);
	form->RedrawLineNumberArea();
}

void Basic4GLEditor::on_actionPause_triggered()
{
	switch (runMode)
	{
	case RunMode::RUNNING:
		Pause();
		break;
	case RunMode::PAUSED:
		Continue();
		break;
	}
}

void Basic4GLEditor::on_actionStep_Into_triggered()
{
	if (runMode != RunMode::PAUSED) return;
	vm.AddStepBreakPts(true);
	Continue();
}

void Basic4GLEditor::on_actionStep_Over_triggered()
{
	if (runMode != RunMode::PAUSED) return;
	vm.AddStepBreakPts(false);
	Continue();
}

void Basic4GLEditor::on_actionStep_Out_Of_triggered()
{
	if (runMode != RunMode::PAUSED) return;
	if (vm.AddStepOutBreakPt())		// Note: Can fail if stack is empty
		Continue();
}

void Basic4GLEditor::on_actionDebug_Mode_toggled(bool arg1)
{
	if (arg1)
	{
		ui->debugToolbar->show();
		ui->debugSplitter->show();
		UpdateDebugUI();
	}
	else
	{
		ui->debugToolbar->hide();
		ui->debugSplitter->hide();
	}
}

void Basic4GLEditor::CreateOrEditWatch(QListWidgetItem* item)
{
	// Lookup corresponding watch
	int index = GetWatchIndex(item);
	QString expression = index >= 0 ? watches[index] : "";

	// Prompt for expression
	bool okClicked;
	expression = QInputDialog::getText(this, index == -1 ? "Add watch" : "Edit watch", "Expression to watch:", QLineEdit::Normal, expression, &okClicked);
	if (okClicked)
	{
		if (index >= 0)
		{
			watches[index] = expression;
		}
		else
		{
			watches.push_back(expression);
			ui->watchList->addItem("");
		}

		UpdateDebugUI();
	}
}

void Basic4GLEditor::on_watchList_itemDoubleClicked(QListWidgetItem *item)
{
	CreateOrEditWatch(item);
}

void Basic4GLEditor::on_actionCreate_Edit_watch_triggered()
{
	CreateOrEditWatch(ui->watchList->currentItem());
}

void Basic4GLEditor::on_actionDelete_watch_triggered()
{
	QListWidgetItem* item = ui->watchList->currentItem();
	int index = GetWatchIndex(item);
	if (index >= 0)
	{
		delete item;
		watches.erase(watches.begin() + index);
	}
}

void Basic4GLEditor::on_actionAdd_watch_at_cursor_triggered()
{
	auto form = sourceFileForms[ui->sourceTabs->currentIndex()];
	QString expression = form->GetExpressionUnderCursor();
	if (!expression.isEmpty())
	{
		watches.push_back(expression);
		ui->watchList->addItem("");
		if (ui->actionDebug_Mode->isChecked())
			UpdateDebugUI();
		else
			ui->actionDebug_Mode->setChecked(true);
	}
}

void Basic4GLEditor::ShowStackCall(QListWidgetItem* item)
{
	int i = GetListItemIndex(ui->stackList, item);
	
	// Top most element is current IP
	if (i == 0)
	{
		PlaceCursorAtIP();		
	}
	else
	{
		i--;
		if (i < vm.UserCallStack().size())
		{
			int index = vm.UserCallStack().size() - i - 1;
			auto& frame = vm.UserCallStack()[index];
			
			unsigned int offset = frame.returnAddr - 1;		// Minus one to get call instruction address
			if (offset < vm.InstructionCount())
			{
				auto& instruction = vm.Instruction(offset);
				PlaceCursor(instruction.m_sourceLine, instruction.m_sourceChar);
			}
		}
	}
}

void Basic4GLEditor::ShowHelp(QString filename)
{
	const QString explorer = "explorer.exe";
	QString param;
	param = QLatin1String("/open,");
	param += QDir::toNativeSeparators(qApp->applicationDirPath() + "/Help/" + filename + ".html");
	QString command = explorer + " " + param;
	QProcess::startDetached(command);
}

void Basic4GLEditor::TextSearch(QString findText, bool isMatchCase, bool isWholeWordsOnly)
{
	int index = ui->sourceTabs->currentIndex();
	sourceFileForms[index]->TextSearch(findText, isMatchCase, isWholeWordsOnly);
}

void Basic4GLEditor::TextReplace(QString findText, QString replaceText, bool isReplaceAll, bool isMatchCase, bool isWholeWordsOnly)
{
	int index = ui->sourceTabs->currentIndex();
	sourceFileForms[index]->TextReplace(findText, replaceText, isReplaceAll, isMatchCase, isWholeWordsOnly);
}

QStringList Basic4GLEditor::GetFunctionNames()
{
	QStringList names;

	// Add function names and constants
	for (auto& fn : compiler.Functions())
		names << QString::fromStdString(compiler.FunctionName(fn.m_index)).toLower();
	for (auto& c : compiler.Constants())
		names << QString::fromStdString(c.first).toUpper();

	// Add function names and constants from plugin DLLs
	for (auto& lib : pluginManager.LoadedLibraries()) {
		for (int i = 0; i < lib->Count(); i++)
			names << QString::fromStdString(lib->FunctionName(i)).toLower();
		for (auto& c : lib->Constants())
			names << QString::fromStdString(c.first).toUpper();
	}

	names.sort(Qt::CaseInsensitive);
	names.removeDuplicates();
	return names;
}

QString Basic4GLEditor::DescribeType(vmValType type)
{
	QString result = "";

	// Pointer level
	for (int i = 0; i < type.m_pointerLevel; i++)
		result += "&amp;";

	// Basic type / structure
	if (type.m_basicType >= 0)
		result += QString::fromStdString(vm.DataTypes().Structures()[type.m_basicType].m_name);
	else
		result += "<span style=\"color:blue\">" + DescribeBasicType(type.m_basicType) + "</span>";

	// Array indices
	for (int i = type.m_arrayLevel - 1; i >= 0; i--) {
		result += "(";
		if (type.VirtualPointerLevel() == 0) {
			int size = type.m_arrayDims[i];
			if (size > 0)
				result += QString::number(size - 1);
		}
		result += ")";
	}

	return result;
}

QString Basic4GLEditor::DescribeBasicType(vmBasicValType type)
{
	switch (type)
	{
	case VTP_INT: return "integer";
	case VTP_REAL: return "single";
	case VTP_STRING: return "string";
	case VTP_NULL: return "null";
	case VTP_FUNC_PTR:
	case VTP_UNTYPED_FUNC_PTR: return "function";
	case VTP_VOID: return "";
	default: return "UNKNOWN TYPE";
	}
}

void Basic4GLEditor::on_stackList_itemDoubleClicked(QListWidgetItem *item)
{
	ShowStackCall(item);
}

void Basic4GLEditor::on_actionShow_Gosub_Function_call_triggered()
{
	if (ui->stackList->count() == 0) return;
	ShowStackCall(ui->stackList->currentItem());
}

void Basic4GLEditor::on_actionPlugin_Libraries_triggered()
{
	pluginDialog.exec();

	if (pluginDialog.WasChangeMade())
	{
		// Main file is effectively modified, as plugin list is saved with it
		sourceFileForms[0]->MarkAsModified();

		// Update auto-complete words
		UpdateCompleter();

		// TODO: Can we force syntax highlighting to redraw?
	}
}

void Basic4GLEditor::on_actionCreate_Standalone_exe_triggered()
{
	// Program must compile
	if (!Compile()) {
		ShowMessage("Program does not compile.\nPlease fix the problem and try again.");
		return;
	}

	if (vm.InstructionCount() <= 1)
	{
		ShowMessage("Program does not produce any executable code.\nPlease fix the problem and try again.");
		return;
	}

	createStandaloneDialog.Execute(sourceFileForms[0]->GetAbsoluteFilename(), compiler, settings.window);
}

void Basic4GLEditor::on_actionBASIC_runtime_settings_triggered()
{
	optionsDialog.Execute(settings);
}

void Basic4GLEditor::on_actionGetting_started_triggered()
{
	ShowHelp("Getting Started");
}

void Basic4GLEditor::on_actionProgrammer_s_guide_triggered()
{
	ShowHelp("Programmers Guide");
}

void Basic4GLEditor::on_actionLanguage_guide_triggered()
{
	ShowHelp("Language Guide");
}

void Basic4GLEditor::on_actionNetwork_engine_guide_triggered()
{
	ShowHelp("Network Guide");
}

void Basic4GLEditor::on_actionOpenGL_guide_triggered()
{
	ShowHelp("OpenGL Guide");
}

void Basic4GLEditor::on_actionSymbolic_debugger_triggered()
{
	ShowHelp("Debugger Guide");
}

void Basic4GLEditor::on_actionSprite_library_guide_triggered()
{
	ShowHelp("Sprite Library Guide");
}

void Basic4GLEditor::on_actionCreating_standalone_exes_triggered()
{
	ShowHelp("Standalone Exes");
}

void Basic4GLEditor::on_actionPlugin_dlls_triggered()
{
	ShowHelp("Plugin DLLs");
}

void Basic4GLEditor::on_actionAbout_triggered()
{
	aboutDialog.exec();
}

void Basic4GLEditor::on_actionArguments_triggered()
{
	argumentsDialog.exec();
}

void Basic4GLEditor::on_actionFind_triggered()
{
	if (findDialog.ShowFindOptions())
	{
		TextSearch(findDialog.GetFindText(), findDialog.IsMatchCase(), findDialog.IsWholeWordsOnly());
	}
}

void Basic4GLEditor::on_actionReplace_triggered()
{
	if (findDialog.ShowReplaceOptions())
	{
		TextReplace(findDialog.GetFindText(), findDialog.GetReplaceText(), findDialog.IsReplaceAll(), findDialog.IsMatchCase(), findDialog.IsWholeWordsOnly());
	}
}

void Basic4GLEditor::on_actionFind_next_triggered()
{
	TextSearch(findDialog.GetFindText(), findDialog.IsMatchCase(), findDialog.IsWholeWordsOnly());
}

void Basic4GLEditor::on_actionCut_triggered()
{
	int index = ui->sourceTabs->currentIndex();
	sourceFileForms[index]->Cut();
}

void Basic4GLEditor::on_actionCopy_triggered()
{
	int index = ui->sourceTabs->currentIndex();
	sourceFileForms[index]->Copy();
}

void Basic4GLEditor::on_actionPaste_triggered()
{
	int index = ui->sourceTabs->currentIndex();
	sourceFileForms[index]->Paste();
}

void Basic4GLEditor::on_actionVirtual_Machine_triggered()
{
	vmViewDialog.Execute(runMode == RunMode::PAUSED);
}

void Basic4GLEditor::UpdateCompleter()
{
	functionNames = GetFunctionNames();
	completer->setModel(new QStringListModel(functionNames));
}
