/*	MainFile.cpp
	Copyright (C) Tom Mulgrew 2008 (tmulgrew@slingshot.co.nz)

*/

#include "MainFile.h"

using namespace std;

MainSourceFile::MainSourceFile(ISourceFile* _file) : file(_file) {

    // Extract first line
    NextLine();
}

void MainSourceFile::NextLine() {
    eof = file->Eof();
    if (!eof) {
        lineNumber = file->LineNumber();
        line = file->GetNextLine();
    }
}

string MainSourceFile::GetNextLine() {
    if (eof)
        return "";
    else {

        // Return line from buffer
        string result = line;

        // Swap next line into buffer
        NextLine();
        return result;
    }
}

string MainSourceFile::Filename() {
    return file->Filename();
}

int MainSourceFile::LineNumber() {
    return lineNumber;
}

bool MainSourceFile::Eof() {
    return eof;
}

void MainSourceFile::Release() {
    file->Release();
    delete this;
}

string MainSourceFile::GetPlugin() {

    // End of file?
    if (eof)
        return "";

    // Not a plugin line?
    if (line.substr(0, 8) != "#plugin ")
        return "";

    // Get plugin name
    string result = line.substr(8, line.length() - 8);

    // Skip line
    NextLine();
    return result;
}


