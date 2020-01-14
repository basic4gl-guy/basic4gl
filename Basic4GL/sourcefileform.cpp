#include "sourcefileform.h"
#include "ui_sourcefileform.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <EmbeddedFiles.h>
#include "basic4glMisc.h"

SourceFileForm::SourceFileForm(ISourceFileFormHost* host, bool isMainFile, QString absoluteFilename, QString text, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SourceFileForm),
	host(host),
	highlighter(nullptr),
	isInitialised(false),
	isMainFile(isMainFile),
	//isModified(false),
	absoluteFilename(absoluteFilename)
{
    ui->setupUi(this);	
	QFontMetrics metrics(ui->sourceTextEdit->font());
	ui->sourceTextEdit->setTabStopWidth(4 * metrics.width(' '));
	ui->sourceTextEdit->SetHost(this);
	highlighter = new Basic4GLSyntaxHighlighter(ui->sourceTextEdit->document(), host->GetCompiler());	// Note: Must set BEFORE setting text, otherwise get a bogus textchanged notification later on.
	ui->sourceTextEdit->setPlainText(text);
	ui->sourceTextEdit->setCompleter(host->GetCompleter());
	SetModified(false);

	isInitialised = true;
}

SourceFileForm::~SourceFileForm()
{
	isInitialised = false;
	delete highlighter;
    delete ui;
}

QString SourceFileForm::GetSourceText() const
{
	return ui->sourceTextEdit->toPlainText();
}

//ui->sourceTextEdit->viewport()->setCursor(Qt::PointingHandCursor)


QString SourceFileForm::GetTabText() const
{
	QString result = "unnamed";
	if (!absoluteFilename.isEmpty())
	{
		// Display relative path and filename
		std::string relative = ProcessPath(CppString(absoluteFilename));
		result = relative.c_str();
	}

	if (IsModified()) result += "*";
	return result;
}

void SourceFileForm::SetReadonly(bool readonly)
{
	ui->sourceTextEdit->setReadOnly(readonly);

	if (readonly)
	{
		// Ensure the cursor is visible even when readonly
		ui->sourceTextEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	}
}

bool SourceFileForm::IsModified() const
{
	return ui->sourceTextEdit->document()->isModified();
}

void SourceFileForm::PlaceCursor(int line, int col)
{
	ui->sourceTextEdit->setFocus();
	QTextCursor cursor = ui->sourceTextEdit->textCursor();

	// Move to start, then down and across
	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	if (line > 0)
		cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
	if (col > 0)
		cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, col);

	ui->sourceTextEdit->setTextCursor(cursor);
}

int SourceFileForm::CursorLine()
{
	return ui->sourceTextEdit->textCursor().blockNumber();
}

void SourceFileForm::RedrawLineNumberArea()
{
	ui->sourceTextEdit->RedrawLineNumberArea();
}

void SourceFileForm::AddTextAction(QAction* action)
{
	ui->sourceTextEdit->addAction(action);
}

QString SourceFileForm::GetExpressionUnderCursor()
{
	QTextCursor cursor = ui->sourceTextEdit->textCursor();
	return ui->sourceTextEdit->GetVariableAt(cursor.block().text(), cursor.columnNumber());
}

void SourceFileForm::MarkAsModified()
{
	if (isInitialised && !IsModified())
	{
		SetModified(true);
		host->SourceIsModified();
	}
}

bool SourceFileForm::DoFindText(QString findText, QTextDocument::FindFlags flags)
{
	if (ui->sourceTextEdit->find(findText, flags))
		return true;

	// Search again from start of document
	auto saveCursor = ui->sourceTextEdit->textCursor();
	ui->sourceTextEdit->moveCursor(QTextCursor::Start);
	if (ui->sourceTextEdit->find(findText, flags))
		return true;
	
	// Not found: Restore original cursor position
	ui->sourceTextEdit->setTextCursor(saveCursor);
	return false;
}

void SourceFileForm::SetModified(bool value)
{
	ui->sourceTextEdit->document()->setModified(value);
}

void SourceFileForm::TextSearch(QString findText, bool isMatchCase, bool isWholeWordsOnly)
{
	QTextDocument::FindFlags flags;
	if (isMatchCase)
		flags |= QTextDocument::FindCaseSensitively;
	if (isWholeWordsOnly)
		flags |= QTextDocument::FindWholeWords;

	DoFindText(findText, flags);
}

void SourceFileForm::TextReplace(QString findText, QString replaceText, bool isReplaceAll, bool isMatchCase, bool isWholeWordsOnly)
{
	QTextDocument::FindFlags flags;
	if (isMatchCase)
		flags |= QTextDocument::FindCaseSensitively;
	if (isWholeWordsOnly)
		flags |= QTextDocument::FindWholeWords;

	// Perform edit inside single edit block, so it can be undone in a single operation
	auto cursor = ui->sourceTextEdit->textCursor();
	cursor.beginEditBlock();

	do
	{
		if (!DoFindText(findText, flags))
		{
			break;
		}
		
		ui->sourceTextEdit->textCursor().insertText(replaceText);
	}
	while (isReplaceAll);

	cursor.endEditBlock();
}

void SourceFileForm::Cut()
{
	ui->sourceTextEdit->cut();
}

void SourceFileForm::Copy()
{
	ui->sourceTextEdit->copy();
}

void SourceFileForm::Paste()
{
	ui->sourceTextEdit->paste();
}

bool SourceFileForm::CheckSaveChanges()
{
	if (!IsModified()) return true;

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Unsaved changes", "Save your changes first?",
		QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	switch (reply)
	{
	case QMessageBox::Yes:
		return Save();
	case QMessageBox::No:
		return true;
	default:
		return false;
	}
}

void SourceFileForm::on_sourceTextEdit_textChanged()
{
	if (isInitialised)
		host->SourceIsModified();
}

bool SourceFileForm::DoSave()
{
	assert(!absoluteFilename.isEmpty());
	QFile file(absoluteFilename);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
		return false;
	}
	QTextStream stream(&file);
	if (isMainFile)
		host->WriteFileHeader(stream);
	stream << ui->sourceTextEdit->toPlainText();
	stream.flush();
	file.close();

	SetModified(false);
	host->SourceIsModified();
	return true;
}

bool SourceFileForm::SaveAs()
{
	QString newFilename = QFileDialog::getSaveFileName(
		this,
		tr("Save Basic4GL Program"),
		absoluteFilename,
		tr("Basic4GL program (*.gb *.inc);;Text file (*.txt);;All files (*.*)"));

	if (newFilename.isEmpty()) return false;			// No file specified. Nothing saved.

	// Store new filename
	absoluteFilename = newFilename;
	host->FilenameIsModified(isMainFile);

	// Save the file
	return DoSave();
}

bool SourceFileForm::Save()
{
	// Do save as instead if no filename set
	if (absoluteFilename.isEmpty()) 
		return SaveAs();

	return DoSave();
}

void SourceFileForm::OpenIncludeFile(QString relativeFilename)
{
	host->OpenIncludeFile(relativeFilename);
}

bool SourceFileForm::IsBreakpoint(int lineNumber)
{
	return host->IsBreakpoint(absoluteFilename, lineNumber);
}

void SourceFileForm::ToggleBreakpoint(int lineNumber)
{
	host->ToggleBreakpoint(absoluteFilename, lineNumber);
}

bool SourceFileForm::IsInstructionPointer(int lineNumber)
{
	return host->IsInstructionPointer(absoluteFilename, lineNumber);
}

QString SourceFileForm::EvaluateVariable(QString variable, bool canCallFunctions)
{
	return host->EvaluateVariable(variable, canCallFunctions);
}

bool SourceFileForm::IsProgramRunning()
{
	return host->IsProgramRunning();
}

QString SourceFileForm::GetFunctionSignature(QString function, int paramNumber)
{
	return host->GetFunctionSignature(function, paramNumber);
}

bool SourceFileForm::IsAutocompleteEnabled()
{
	return host->IsAutocompleteEnabled();
}

bool SourceFileForm::IsFunctionSignatureEnabled()
{
	return host->IsFunctionSignatureEnabled();
}
