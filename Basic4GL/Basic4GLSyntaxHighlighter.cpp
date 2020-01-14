#include "Basic4GLSyntaxHighlighter.h"
#include "basic4glMisc.h"


Basic4GLSyntaxHighlighter::Basic4GLSyntaxHighlighter(QTextDocument* parent, TomBasicCompiler& compiler) 
	: QSyntaxHighlighter(parent), compiler(compiler)
{
	// Setup formats
	defaultFormat.setForeground(QBrush(0x000080));
	keywordFormat.setForeground(Qt::blue);
	functionFormat.setForeground(Qt::red);
	constantFormat.setForeground(defaultFormat.foreground());
	constantFormat.setFontWeight(75);			// Bold??
	numberFormat.setForeground(defaultFormat.foreground());
	numberFormat.setFontWeight(75);
	stringFormat.setForeground(Qt::darkGreen);
	commentFormat.setForeground(QBrush(0x657CA7));
	commentFormat.setFontItalic(true);
	includeFormat.setForeground(defaultFormat.foreground());
	includeFormat.setBackground(QBrush(0xffffd0));
	hyperlinkFormat.setBackground(includeFormat.background());
	hyperlinkFormat.setForeground(Qt::blue);
	hyperlinkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
}

void Basic4GLSyntaxHighlighter::highlightBlock(const QString& text)
{
	// Include special case
	if (text.left(8).toLower() == "include ") {
		setFormat(0, 8, includeFormat);
		setFormat(8, text.length() - 8, hyperlinkFormat);
		return;
	}
	if (text.left(13).toLower() == "includeblock ")
	{
		setFormat(0, 13, includeFormat);
		setFormat(13, text.length() - 13, hyperlinkFormat);
		return;
	}

	// Break line into tokens
	int i = 0;
	while (i < text.length()) {

		// Skip whitespace
		while (i < text.length() && text[i] <= ' ')
			i++;

		if (i < text.length()) {

			// Identify basic token type			
			QChar c = text[i].toLower();
			int left = i;
			int style;
			if (c.isLetter() || c == '_')
				style = 1;                              // 1 = keyword
			else if (c.isDigit() || c == '.')
				style = 2;                              // 2 = Number
			else if (c == '"')
				style = 3;                              // 3 = String
			else if (c == '\'')
				style = 4;                              // 4 = Comment
			else
				style = 5;                              // 5 = Symbol

			i++;
			bool done = false;
			while (i < text.length() && !done) {

				// Look for end of token
				c = text[i].toLower();
				switch (style) {
				case 1:
					if (c == '$' || c == '%' || c == '#') {
						i++;
						done = true;
					}
					else
						done = !(c.isLetterOrNumber() || c == '_');
					break;
				case 2:
					done = !(c.isNumber() || c == '.');
					break;
				case 3:
					done = c == '"';
					if (done)
						i++;
					break;
				case 4:
					done = false;
					break;
				case 5:
					done = true;
				};

				if (!done)
					i++;
			}

			QString token = text.mid(left, i - left);
			std::string tokenStr = CppString(token.toLower());

			// Determine text format
			QTextCharFormat* format = &defaultFormat;
			switch (style)
			{
			case 1:
				if (compiler.IsReservedWord(tokenStr) || compiler.IsOperator(tokenStr))
				{
					format = &keywordFormat;
				}
				else if (compiler.IsFunction(tokenStr))
				{
					format = &functionFormat;
				}
				else if (compiler.IsConstant(tokenStr))
				{
					format = &constantFormat;
				}
				break;
			case 2:
				format = &numberFormat;
				break;
			case 3:
				format = &stringFormat;
				break;
			case 4:
				format = &commentFormat;
				break;
			case 5:
				if (compiler.IsOperator(tokenStr))
				{
					format = &keywordFormat;
				}
				break;
			}

			// Apply format
			setFormat(left, i - left, *format);
		}
	}

	// TODO: Breakpoint styling??
}
