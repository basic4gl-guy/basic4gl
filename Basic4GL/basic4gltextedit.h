#pragma once
#include <QPlainTextEdit>
#include <QIcon>
#include <QCompleter>

class ITextEditHost
{
public:
	virtual void OpenIncludeFile(QString relativeFilename) = 0;
	virtual bool IsBreakpoint(int lineNumber) = 0;
	virtual void ToggleBreakpoint(int lineNumber) = 0;
	virtual bool IsInstructionPointer(int lineNumber) = 0;
	virtual QString EvaluateVariable(QString variable, bool canCallFunctions) = 0;
	virtual bool IsProgramRunning() = 0;
	virtual QString GetFunctionSignature(QString function, int paramNumber) = 0;
	virtual bool IsAutocompleteEnabled() = 0;
	virtual bool IsFunctionSignatureEnabled() = 0;
};

class Basic4GLTextEdit :
	public QPlainTextEdit 
{
	Q_OBJECT
public:
	Basic4GLTextEdit(QWidget* parent = 0);
	void SetHost(ITextEditHost* value) { host = value; }
	void lineNumberAreaPaintEvent(QPaintEvent* event);
	int lineNumberAreaWidth();
	void RedrawLineNumberArea();
	void lineNumberAreaClicked(QMouseEvent *event);
	QString GetVariableAt(QString line, int x);
	bool GetFunctionSignatureAt(QString line, int x, QString& function, int& paramNumber);
	QString GetCompletionPrefix(QString line, int x);

	// Autocompletion
	void setCompleter(QCompleter* completer);
	QCompleter *completer() const;

protected:
	void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
	bool event(QEvent* e) Q_DECL_OVERRIDE;
	void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
	void focusInEvent(QFocusEvent* e) Q_DECL_OVERRIDE;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect&, int);
	void HighlightLines();
	void insertCompletion(const QString& completion);

private:
	ITextEditHost* host;
	QWidget* lineNumberArea;
	QIcon iconBreakPt;
	QIcon iconHand;
	QCompleter* c;
	void KeypressAutocompleteLogic(QKeyEvent* e, bool isShortcut);
	void KeypressTooltipLogic(QKeyEvent* e, bool isShortcut);
	bool isIdentifierCharacter(QChar c);
};

