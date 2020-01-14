#include "DiskFile.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  DiskFile

DiskFile::DiskFile(string _filename)
        : filename(_filename), file(_filename.c_str()), lineNo(0) {
}

bool DiskFile::Fail() {
    return file.fail();
}

string DiskFile::GetNextLine() {
    if (!Eof()) {
        lineNo++;
        char lineBuffer[65536];
        file.getline(lineBuffer, 65536);
        return (string) lineBuffer;
    }
    else
        return "";
}

bool DiskFile::Eof() {
    return file.fail() || file.eof();
}

void DiskFile::Release() {
    delete this;
}

string DiskFile::Filename() {
    return filename;
}

int DiskFile::LineNumber() {
    return lineNo;
}

////////////////////////////////////////////////////////////////////////////////
//  DiskFileServer
ISourceFile* DiskFileServer::OpenSourceFile(string filename) {
    DiskFile *file = new DiskFile(filename);
    if (file->Fail()) {
        delete file;
        return NULL;
    }
    return file;
}

