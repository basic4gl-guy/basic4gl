#pragma once

#include "TomComp.h"
#include <QSyntaxHighlighter>

class Basic4GLSyntaxHighlighter : public QSyntaxHighlighter
{
public:
	Basic4GLSyntaxHighlighter(QTextDocument* parent, TomBasicCompiler& compiler);

protected:
	void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
	TomBasicCompiler& compiler;

	QTextCharFormat defaultFormat;
	QTextCharFormat keywordFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat constantFormat;
	QTextCharFormat numberFormat;
	QTextCharFormat stringFormat;
	QTextCharFormat commentFormat;	
	QTextCharFormat includeFormat;
	QTextCharFormat hyperlinkFormat;
};

