#include "EditorSourceFileServer.h"
#include "basic4glMisc.h"
#include <EmbeddedFiles.h>

///////////////////////////////////////////////////////////////////////////////
// EditorSourceFileServer

EditorSourceFileServer::EditorSourceFileServer(ISourceFileHost* host)
	: host(host)
{
}

ISourceFile* EditorSourceFileServer::OpenSourceFile(std::string filename)
{
	for (int i = 0; i < host->FileCount(); i++)
	{
		std::string sourceFilename = ProcessPath(CppString(host->GetSourceAbsoluteFilename(i)));
		if (sourceFilename == filename)
		{
			return new EditorSourceFile(sourceFilename, host->GetSourceText(i));
		}
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// EditorSourceFile

EditorSourceFile::EditorSourceFile(std::string filename, QString const& source)
	: sourceLines(source.split(QRegExp("\n|\r\n|\r"))),
	lineNumber(0),
	filename(filename)
{	
}

std::string EditorSourceFile::GetNextLine()
{
	return !Eof()
		? CppString(sourceLines[lineNumber++])
		: "";
}

std::string EditorSourceFile::Filename()
{
	return filename;
}

int EditorSourceFile::LineNumber()
{
	return lineNumber;
}

bool EditorSourceFile::Eof()
{
	return lineNumber >= sourceLines.size();
}

void EditorSourceFile::Release()
{
	// Assume we are only ever referenced by one object.
	delete this;
}

