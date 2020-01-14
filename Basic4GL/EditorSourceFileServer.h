#pragma once
#include "PreprocessorInterfaces.h"
#include "SourceFileInterfaces.h"
#include <QString>

class ISourceFileHost
{
public:
	virtual int FileCount() = 0;
	virtual QString GetSourceAbsoluteFilename(int index) = 0;
	virtual QString GetSourceText(int index) = 0;
};

class EditorSourceFileServer :
	public ISourceFileServer
{
	ISourceFileHost* host;
public:
	EditorSourceFileServer(ISourceFileHost* host);

	// ISourceFileServer
	ISourceFile* OpenSourceFile(std::string filename) override;
};

class EditorSourceFile : 
	public ISourceFile
{
	QStringList sourceLines;
	int lineNumber;
	std::string filename;
public:
	explicit EditorSourceFile(std::string filename, QString const& source);
	
	// ISourceFile
	std::string GetNextLine() override;
	std::string Filename() override;
	int LineNumber() override;
	bool Eof() override;
	void Release() override;
};