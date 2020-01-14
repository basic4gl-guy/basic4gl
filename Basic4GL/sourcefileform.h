#ifndef SOURCEFILEFORM_H
#define SOURCEFILEFORM_H

#include "Basic4GLSyntaxHighlighter.h"
#include "TomComp.h"
#include "basic4gltextedit.h"
#include "SourceFileInterfaces.h"
#include <QtWidgets/QSplitter>


namespace Ui {
class SourceFileForm;
}

class SourceFileForm : public QWidget, public ITextEditHost
{
    Q_OBJECT

public:
	explicit SourceFileForm(ISourceFileFormHost* host, bool isMainFile, QString absoluteFilename = "", QString text = "", QWidget *parent = 0);
    ~SourceFileForm();

	QString GetSourceText() const;
	QString GetTabText() const;
	void SetReadonly(bool readonly);
	bool IsModified() const;
	QString GetAbsoluteFilename() const { return absoluteFilename; }
	void PlaceCursor(int line, int col);
	int CursorLine();
	void RedrawLineNumberArea();
	void AddTextAction(QAction* action);
	QString GetExpressionUnderCursor();
	void MarkAsModified();
	void TextSearch(QString findText, bool isMatchCase, bool isWholeWordsOnly);
	void TextReplace(QString findText, QString replaceText, bool isReplaceAll, bool isMatchCase, bool isWholeWordsOnly);
	void Cut();
	void Copy();
	void Paste();

	bool CheckSaveChanges();
	bool SaveAs();
	bool Save();

	// ITextEditHost
	void OpenIncludeFile(QString relativeFilename) override;
	bool IsBreakpoint(int lineNumber) override;
	void ToggleBreakpoint(int lineNumber) override;
	bool IsInstructionPointer(int lineNumber) override;
	QString EvaluateVariable(QString variable, bool canCallFunctions) override;
	bool IsProgramRunning() override;
	QString GetFunctionSignature(QString function, int paramNumber) override;
	bool IsAutocompleteEnabled() override;
	bool IsFunctionSignatureEnabled() override;

private slots:
		void on_sourceTextEdit_textChanged();

private:
    Ui::SourceFileForm *ui;

	// Components
	ISourceFileFormHost* host;
	Basic4GLSyntaxHighlighter* highlighter;

	// State
	bool isInitialised;
	bool isMainFile;
//	bool isModified;
	QString absoluteFilename;				// Filename stored as full absolute path and filename

	bool DoSave();
	bool DoFindText(QString findText, QTextDocument::FindFlags flags);
	void SetModified(bool value);
};

#endif // SOURCEFILEFORM_H
