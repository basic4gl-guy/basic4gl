#pragma once
#include "basic4gltextedit.h"
#include <QIcon>

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(Basic4GLTextEdit* editor) : QWidget(editor)
	{
		codeEditor = editor;
	}

	QSize sizeHint() const Q_DECL_OVERRIDE{
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE{
		codeEditor->lineNumberAreaClicked(event);
		event->accept();
	}

protected:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE{
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	Basic4GLTextEdit* codeEditor;
};

