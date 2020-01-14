#include "compPreprocessor.h"
#include "FilenameRoutines.h"
#include <cstdarg>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//  compPreprocessor

compPreprocessor::compPreprocessor(int serverCount, ISourceFileServer *server, ...) : inCodeBlock(false) {
    // Register source file servers
	if (serverCount > 0)
	{
		va_list vl;
		va_start(vl, server);
		fileServers.push_back(server);
		for (int i = 1; i < serverCount; i++)
			fileServers.push_back(va_arg(vl, ISourceFileServer*));
		va_end(vl);
	}
    //for (int i = 0; i < serverCount; i++)
    //    fileServers.push_back((&server)[i]);
}

compPreprocessor::~compPreprocessor() {

    // Ensure no source files are still open
    CloseAll();

    // Delete source file servers
    for (   list<ISourceFileServer*>::iterator i = fileServers.begin();
            i != fileServers.end();
            i++)
        delete *i;
}

bool compPreprocessor::Preprocess(ISourceFile *mainFile, compParser& parser) {
    assert(mainFile != NULL);

    // Reset
    CloseAll();
    visitedFiles.clear();
    lineNumberMap.Clear();
    ClearError();
	inCodeBlock = false;

    // Clear the parser
    parser.SourceCode().clear();

    // Load the main file
    openFiles.push_back(mainFile);

    // Process files
	int lineNo = 0;
    while (!openFiles.empty() && !Error()) {
        // Check for Eof
        if (openFiles.back()->Eof()) {

			// Emit 'endcodeblock' if leaving an includeblock file
			if (inCodeBlock)
			{
				lineNumberMap.AddLine(openFiles.back()->Filename(), lineNo);
				parser.SourceCode().push_back("endcodeblock");
				inCodeBlock = false;
			}

            // Close innermost file
            openFiles.back()->Release();
            openFiles.pop_back();
        }
        else {

            // Read a line from the source file
            lineNo = openFiles.back()->LineNumber();
            string line = openFiles.back()->GetNextLine();

            // Check for #include
	        bool isInclude = LowerCase(line.substr(0, 8)) == "include ";
	        bool isIncludeBlock = LowerCase(line.substr(0, 13)) == "includeblock ";
	        if (isInclude || isIncludeBlock) {

				// Code block rules
				if (inCodeBlock)
				{
					SetError("'include' or 'includeblock' not allowed inside an existing 'includeblock'");
					break;
				}

                // Get filename
                string filename = ProcessPath(line.substr(isInclude ? 8 : 13, line.length()));

                // Check this file hasn't been included already
                if (visitedFiles.find(filename) == visitedFiles.end()) {

                    // Open next file
                    ISourceFile *file = OpenFile(filename);
                    if (file == NULL)
                        SetError("Unable to open file: " + line.substr(8, line.length()));
                    else {
                        // This becomes the new innermost file
                        openFiles.push_back(file);

                        // Add to visited files list
                        visitedFiles.insert(filename);

						// Emit 'begincodeblock' if entering code block
						if (isIncludeBlock)
						{
							lineNumberMap.AddLine(openFiles.back()->Filename(), lineNo);
							parser.SourceCode().push_back("begincodeblock \"" + filename + "\"");
							inCodeBlock = true;
						}
                    }
                }
            }
            else {
                // Not an #include line
                // Add to parser, and line number map
                lineNumberMap.AddLine(openFiles.back()->Filename(), lineNo);
                parser.SourceCode().push_back(line);
            }
        }
    }

    // Return true if no error encountered
    return !Error();     
}

void compPreprocessor::CloseAll() {

    // Close all open files
    for (int i = 0; i < openFiles.size(); i++)
        openFiles[i]->Release();
    openFiles.clear();
}

ISourceFile* compPreprocessor::OpenFile(string filename) {

    // Query file servers in order until one returns an open file.
    for (   list<ISourceFileServer*>::iterator i = fileServers.begin();
            i != fileServers.end();
            i++) {
        ISourceFile *file = (*i)->OpenSourceFile(filename);
        if (file != NULL)
            return file;
    }

    // Unable to open file
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//  compLineNumberMapping
void compLineNumberMapping::Clear() {
    filenames.clear();
    filenameLookup.clear();
    mapping.clear();
    reverseMapping.clear();
}

void compLineNumberMapping::AddLine(std::string filename, int fileLineNo) {

    // Append mapping entry
    int fileIndex = GetFileIndex(filename);
    int mainLineNo = mapping.size();
    mapping.push_back(compSourcePos(fileIndex, fileLineNo));

    // Append reverse-mapping entry
    while (reverseMapping[fileIndex].size() <= fileLineNo)
        reverseMapping[fileIndex].push_back(-1);
    reverseMapping[fileIndex][fileLineNo] = mainLineNo;
}

int compLineNumberMapping::GetFileIndex(string filename) {

    // Is file already in list?
    map<string,int>::iterator i = filenameLookup.find(filename);
    if (i != filenameLookup.end())
        return i->second;
    else {
        // Otherwise, add to list
        int index = filenames.size();
        filenameLookup[filename] = index;
        filenames.push_back(filename);
        reverseMapping.push_back(IntVector());
        return index;
    }
}

void compLineNumberMapping::SourceFromMain(string& filename, int& fileLineNo, int lineNo) {

	// Special case: Line immediately after last: Move to end of previous line
	if (lineNo == mapping.size() && !mapping.empty())
	{
		lineNo--;
	}	

    // Is source line valid
    if (lineNo >= 0 && lineNo < mapping.size()) {

        // Return filename and line number
        filename = filenames[mapping[lineNo].fileIndex];
        fileLineNo = mapping[lineNo].fileLineNo;
    }
    else {

        // Invalid source line
        filename = "?";
        fileLineNo = -1;
    }
}

int compLineNumberMapping::SourceFromMain(string filename, int lineNo) {
    return SourceFromMain(GetFileIndex(filename), lineNo);
}

int compLineNumberMapping::SourceFromMain(int fileIndex, int lineNo) {
    // Is source line valid and does it correspond to a line inside the
    // specified file?
    if (lineNo >= 0 && lineNo < mapping.size() &&
            mapping[lineNo].fileIndex == fileIndex)
        return mapping[lineNo].fileLineNo;
    else
        return -1;
}

int compLineNumberMapping::MainFromSource(string filename, int fileLineNo) {
    return MainFromSource(GetFileIndex(filename), fileLineNo);
}

int compLineNumberMapping::MainFromSource(int fileIndex, int fileLineNo) {
    // Check line is valid
    if (fileLineNo >= 0 && fileLineNo < reverseMapping[fileIndex].size())
        return reverseMapping[fileIndex][fileLineNo];
    else
        return -1;
}

