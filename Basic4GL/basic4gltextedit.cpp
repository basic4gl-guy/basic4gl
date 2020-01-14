#include "basic4gltextedit.h"
#include "LineNumberArea.h"
#include "EditorSourceFileServer.h"
#include <QPainter>
#include <QToolTip>
#include <QTextBlock>
#include <QScrollBar>
#include <QApplication>
#include "basic4glMisc.h"

const int NumberAreaBuffer = 20;
const int IconSize = 16;

Basic4GLTextEdit::Basic4GLTextEdit(QWidget* parent) : QPlainTextEdit(parent), host(nullptr), c(nullptr)
{
	iconBreakPt.addFile(":/icons/Images/BreakPt.png", QSize(IconSize, IconSize));
	iconHand.addFile(":/icons/Images/Hand.png", QSize(IconSize, IconSize));

	// Create and wire up line number area
	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(HighlightLines()));

	updateLineNumberAreaWidth(0);
	HighlightLines();

	viewport()->setMouseTracking(true);				// Required for mouse-move events
}

void Basic4GLTextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	// Paint gray background
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), QColor(0xf0f0f0));
	painter.fillRect(QRect(event->rect().right() - 1, event->rect().top(), 1, event->rect().height()), QColor(0x808080));

	// Find top of first visible rectangle
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();

	// Walk down visible blocks, drawing line numbers for those that intersect the painted rectangle
	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			// Draw number on every 5th line
			if (((blockNumber + 1) % 5) == 0) {
				QString number = QString::number(blockNumber + 1);
				painter.setPen(QColor(0x99c8e6));
				painter.drawText(2, top, lineNumberArea->width() - NumberAreaBuffer - 2, fontMetrics().height(), Qt::AlignLeft, number);
			}

			// Draw breakpoint
			if (host->IsBreakpoint(blockNumber))
			{
				iconBreakPt.paint(&painter, lineNumberArea->width() - 3 - IconSize, (top + bottom - IconSize) / 2, IconSize, IconSize);
				// TODO: Highlight line in red
			}

			if (host->IsInstructionPointer(blockNumber))
			{
				iconHand.paint(&painter, lineNumberArea->width() - 3 - IconSize, (top + bottom - IconSize) / 2, IconSize, IconSize);
			}
		}

		// Find next block and vertical position
		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();

		blockNumber++;
	}
}

int Basic4GLTextEdit::lineNumberAreaWidth()
{
	// Count maximum # of digits
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10)
	{
		max /= 10;
		digits++;
	}

	// Allocate space accordingly
	int space = 3 + fontMetrics().width(QLatin1Char('9')) * qMax(digits, 3) + NumberAreaBuffer;

	return space;
}

void Basic4GLTextEdit::RedrawLineNumberArea()
{
	lineNumberArea->update();			// Request repaint
}

void Basic4GLTextEdit::lineNumberAreaClicked(QMouseEvent* event)
{
	// Find line number for corresponding Y position
	QPoint adjustedPos(0, event->pos().y());
	QTextCursor cursor = cursorForPosition(adjustedPos);
	
	// Toggle breakpoint
	host->ToggleBreakpoint(cursor.blockNumber());

	// Repaint
	lineNumberArea->update();
	HighlightLines();
}

bool IsMouseOverIncludeFile(QString lineText, QString includePrefix, QString::size_type col, QString& includeFile)
{
	includePrefix += " ";
	QString::size_type prefixSize = includePrefix.size();
	if (lineText.left(prefixSize).toLower() == includePrefix && col >= prefixSize && col < lineText.size())
	{
		includeFile = lineText.right(lineText.size() - prefixSize);
		return true;
	}

	return false;
}

void Basic4GLTextEdit::mousePressEvent(QMouseEvent* e)
{
	QPlainTextEdit::mousePressEvent(e);

	// Clicked on "include" line?
	QPoint pos = e->pos();
	QTextCursor cursor = cursorForPosition(pos);
	QString lineText = cursor.block().text();
	QString::size_type col = cursor.columnNumber();
	QString includeFile;
	if (IsMouseOverIncludeFile(lineText, "include", col, includeFile)
		|| IsMouseOverIncludeFile(lineText, "includeblock", col, includeFile))
	{
		host->OpenIncludeFile(includeFile);
	}
}

void Basic4GLTextEdit::mouseMoveEvent(QMouseEvent* e)
{
	QPoint pos = e->pos();
	QTextCursor cursor = cursorForPosition(pos);
	QString lineText = cursor.block().text();
	QString::size_type col = cursor.columnNumber();
	QString includeFile;
	if (IsMouseOverIncludeFile(lineText, "include", col, includeFile)
		|| IsMouseOverIncludeFile(lineText, "includeblock", col, includeFile))
	{
		viewport()->setCursor(Qt::PointingHandCursor);
	}
	else
	{
		viewport()->setCursor(Qt::IBeamCursor);
	}

	QPlainTextEdit::mouseMoveEvent(e);
}

void Basic4GLTextEdit::resizeEvent(QResizeEvent* event)
{
	QPlainTextEdit::resizeEvent(event);

	// Layout the line number area on the left
	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

bool Basic4GLTextEdit::event(QEvent* e)
{
	// Detect tooltip event
	if (e->type() == QEvent::ToolTip)
	{
		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);

		// Find text under mouse
		if (!host->IsProgramRunning()) {
			QPoint texRelPos(helpEvent->pos().x() - lineNumberAreaWidth(), helpEvent->pos().y());
			QTextCursor cursor = cursorForPosition(texRelPos);
			QString variable = GetVariableAt(cursor.block().text(), cursor.columnNumber());
			if (!variable.isEmpty())
			{
				// Evaluate variable and display as tool tip
				QString value = host->EvaluateVariable(variable, false);
				QToolTip::showText(helpEvent->globalPos(), "<p>" + variable + ": " + value.toHtmlEscaped() + "</p>");
				return true;
			}
		}

		// No tooltip
		QToolTip::hideText();
		e->ignore();
		return true;
	}

	return QPlainTextEdit::event(e);
}

void Basic4GLTextEdit::keyPressEvent(QKeyEvent* e)
{
	if (c && c->popup()->isVisible())
	{
		// Let completer handle certain keypresses
		switch (e->key())
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return;
		default:
			break;
		}
	}

	// Detect autocomplete shortcut key.
	bool isAutocompleteShortcut = ((e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::ShiftModifier) && e->key() == Qt::Key_Space);
	bool isTooltipShortcut = ((e->modifiers() & Qt::ControlModifier) && (e->modifiers() & Qt::ShiftModifier) && e->key() == Qt::Key_Space);

	if (isAutocompleteShortcut) 
		KeypressAutocompleteLogic(e, isAutocompleteShortcut);
	else if (isTooltipShortcut)
		KeypressTooltipLogic(e, isTooltipShortcut);
	else
	{
		QPlainTextEdit::keyPressEvent(e);
		KeypressAutocompleteLogic(e, isAutocompleteShortcut);
		KeypressTooltipLogic(e, isTooltipShortcut);
	}
}

void Basic4GLTextEdit::focusInEvent(QFocusEvent* e)
{
	if (c)
		c->setWidget(this);
	QPlainTextEdit::focusInEvent(e);
}

void Basic4GLTextEdit::updateLineNumberAreaWidth(int newBlockCount)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Basic4GLTextEdit::updateLineNumberArea(const QRect& rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void Basic4GLTextEdit::HighlightLines()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	// Breakpoint lines
	if (host != nullptr) {
		for (auto block = document()->begin(); block.isValid(); block = block.next())
		{
			if (host->IsBreakpoint(block.blockNumber()))
			{
				QColor lineColor = QColor(255, 215, 215);
				QTextEdit::ExtraSelection selection;
				selection.format.setBackground(lineColor);
				selection.format.setProperty(QTextFormat::FullWidthSelection, true);
				selection.cursor = QTextCursor(block);
				selection.cursor.clearSelection();
				extraSelections.append(selection);
			}
		}
	}

	// Current line
	{
		QColor lineColor = QColor(255, 255, 215);
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void Basic4GLTextEdit::insertCompletion(const QString& completion)
{
	if (c->widget() != this) return;

	// Use text cursor to insert remaining characters
	QTextCursor tc = textCursor();
	tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);			// Select end of current word

	// Replace with end of selected entry
	int numRemaining = completion.length() - c->completionPrefix().length();
	tc.insertText(completion.right(numRemaining));

	setTextCursor(tc);
}

void Basic4GLTextEdit::KeypressAutocompleteLogic(QKeyEvent* e, bool isShortcut)
{
	QTextCursor cursor = textCursor();
	QString completionPrefix = GetCompletionPrefix(cursor.block().text(), cursor.columnNumber());
	QString text = e->text();

	// Determine whether to popup should display
	static QString eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-="); // end of word
	bool canShow = completionPrefix.length() >= 1;
	bool isLong = completionPrefix.length() < 3;
	bool autoShow = host->IsAutocompleteEnabled()
		&& (e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::ShiftModifier)
		&& !text.isEmpty()
		&& !eow.contains(text.right(1));
	bool isShowing = c->popup()->isVisible();
	bool show = canShow && (isShortcut || ((autoShow || isShowing) && !isLong));

	if (show)
	{
		// Set completion text
		if (completionPrefix != c->completionPrefix())
		{
			c->setCompletionPrefix(completionPrefix);
			c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
		}

		// Size window
		QRect cr = cursorRect();
		cr.setWidth(c->popup()->sizeHintForColumn(0) + c->popup()->verticalScrollBar()->sizeHint().width());

		// Pop it up
		c->complete(cr);
	}
	else
		c->popup()->hide();
}

void Basic4GLTextEdit::KeypressTooltipLogic(QKeyEvent* e, bool isShortcut)
{
	QString text = e->text();

	// Determine whether to show tooltip
	bool autoShow = host->IsFunctionSignatureEnabled()
		&& (e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::ShiftModifier)
		&& !text.isEmpty();
	bool isShowing = QToolTip::isVisible();
	bool hide = e->key() == Qt::Key_Escape;
	bool show = (isShortcut || autoShow || isShowing) && !hide;

	// Get signature
	QString signature = "";
	if (show)
	{
		// Find cursor position
		QTextCursor cursor = textCursor();
		QString function;
		int paramNumber;
		if (GetFunctionSignatureAt(cursor.block().text(), cursor.columnNumber(), function, paramNumber))
		{
			signature = host->GetFunctionSignature(function, paramNumber);
		}
	}
	
	// Display signature
	if (signature == "") 
		QToolTip::hideText();
	else
	{
		// Attempt to position above cursor
		int rowCount = signature.count('\n') + 1;
		QPoint pos = cursorRect().topLeft();
		pos += QPoint(0, -25);
		pos += QPoint(0, rowCount * -30);				// Allow room for 2 rows as may word wrap.
		QPoint globalPos = viewport()->mapToGlobal(pos);
		if (globalPos.y() < 0)
		{
			// No room above. Place underneath.
			pos = cursorRect().bottomLeft();
			globalPos = viewport()->mapToGlobal(pos);
		}

		// Show tooltip
		QToolTip::showText(globalPos, signature, this, QRect(), 60000);
	}
}

bool Basic4GLTextEdit::isIdentifierCharacter(QChar c)
{
	return c.isLetterOrNumber() || c == '#' || c == '%' || c == '$' || c == '_';
}

QString Basic4GLTextEdit::GetVariableAt(QString line, int x)
{
	// Find character
	if (x < 0 || x >= line.size() || line[x].isSpace())
		return "";

	// Scan to right of word
	int right = x + 1;
	while (right < line.size()
		&& ((line[right].isLetterOrNumber())
		|| line[right] == '_'
		|| line[right] == '#'
		|| line[right] == '$'))
		right++;

	// Scan left
	int left = x;
	while (left >= 0
		&& ((line[left].isLetterOrNumber())
		|| line[left] == '.'
		|| line[left] == '_'
		|| line[left] == '#'
		|| line[left] == '$'
		|| line[left] == ')')) {

		// Skip over brackets
		if (line[left] == ')') {
			int level = 1;
			left--;
			while (level > 0 && left > 0) {
				if (line[left] == ')')         level++;
				else if (line[left] == '(')    level--;
				left--;
			}
			while (left >= 0 && line[left].isSpace())
				left--;
		}
		else
			left--;
	}
	left++;

	// Trim whitespace from left
	while (left < right && line[left] <= ' ')
		left++;

	// Return result
	return left < right ? line.mid(left, right - left) : "";
}

bool Basic4GLTextEdit::GetFunctionSignatureAt(QString line, int x, QString& function, int& paramNumber)
{
	function = "";
	paramNumber = 0;
	
	// Search left for opening (.
	// Skip (), with proper handling of nesting
	// Count commas.
	int bracketLevel = 1;
	while (bracketLevel > 0 && x > 0)
	{
		x--;
		if (line[x] == ',' && bracketLevel == 1) paramNumber++;
		if (line[x] == ')') bracketLevel++;
		if (line[x] == '(') bracketLevel--;
	}

	// No opening bracket found? -> Not inside a function
	if (x <= 0) return false;

	// Search for function name before (. Skip any whitespace
	x--;
	while (x >= 0 && line[x].isSpace()) x--;
	if (x <= 0) return false;

	// Find function name
	int end = x + 1;
	while (x >= 0 && isIdentifierCharacter(line[x])) x--;
	int start = x + 1;

	// Get function name
	function = line.mid(start, end - start).toLower();
	return !function.isEmpty();
}

QString Basic4GLTextEdit::GetCompletionPrefix(QString line, int x)
{
	int end = x;
	x--;
	while (x >= 0 && isIdentifierCharacter(line[x])) x--;
	int start = x + 1;
	return line.mid(start, end - start).toLower();
}

void Basic4GLTextEdit::setCompleter(QCompleter* completer)
{
	// Disconnect previous one (if any)
	if (c) QObject::disconnect(c, nullptr, this, nullptr);

	// Set new one
	c = completer;
	if (!c) return;
	
	// Configure and wire up
	c->setWidget(this);
	c->setCompletionMode(QCompleter::PopupCompletion);
	c->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(c, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

QCompleter* Basic4GLTextEdit::completer() const
{
	return c;
}
