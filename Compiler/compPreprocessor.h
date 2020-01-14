/*  Created 2-Jun-2007: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Basic4GL compiler pre-processor.
*/
#ifndef compPreprocessorH
#define compPreprocessorH

#include "PreprocessorInterfaces.h"
#include "HasErrorState.h"
#include "compParse.h"
#include <list>
#include <string>
#include <vector>
#include <set>
#include <map>

struct compSourcePos {
    // Source file and line number before pre-processing
    int fileIndex;
    int fileLineNo;

    compSourcePos(int _fileIndex, int _fileLineNo)
            : fileIndex(_fileIndex), fileLineNo(_fileLineNo) {
    }
    compSourcePos(const compSourcePos& i) 
            : fileIndex(i.fileIndex), fileLineNo(i.fileLineNo) {
    }
};

typedef std::vector<int> IntVector;

////////////////////////////////////////////////////////////////////////////////
//  compLineNumberMapping
//
/// Maps line numbers of the expanded file back to their corresponding source file.
/// Used by debugging code so that lines correctly match up with program addresses.
class compLineNumberMapping : public ILineNumberMapping {
    std::vector<std::string> filenames;
    std::map<std::string,int> filenameLookup;
    std::vector<compSourcePos> mapping;
    std::vector<IntVector> reverseMapping;

    int GetFileIndex(std::string filename);
    int SourceFromMain(int fileIndex, int lineNo);
    int MainFromSource(int fileIndex, int fileLineNo);

public:
    // Mapping building
    void Clear();
    void AddLine(std::string filename, int fileLineNo);

    // ILineNumberMapping methods
    virtual void SourceFromMain(std::string& filename, int& fileLineNo, int lineNo);
    virtual int SourceFromMain(std::string filename, int lineNo);
    virtual int MainFromSource(std::string filename, int fileLineNo);
};

////////////////////////////////////////////////////////////////////////////////
//  compPreprocessor
//
/// Basic4GL compiler preprocessor.
/// Note: Basic4GL doesn't do a lot of preprocessing. But we do implement an
///     #include file mechanism. The preprocessor has the task of transparently
///     expanding #includes into a single large source file.
class compPreprocessor : public HasErrorState {

    // Registered source file servers
    std::list<ISourceFileServer*> fileServers;

    // Stack of currently opened files.
    // openFiles.back() is the current file being parsed
    std::vector<ISourceFile*> openFiles;

    // Filenames of visited source files. (To prevent circular includes)
    std::set<std::string> visitedFiles;

    // Source file <=> Processed file mapping
    compLineNumberMapping lineNumberMap;

    void CloseAll();
    ISourceFile* OpenFile(std::string filename);
	bool inCodeBlock;

public:

    /// Construct the preprocessor. Pass in 0 or more file servers to initialise.
    compPreprocessor(int serverCount, ISourceFileServer *server, ...);
    virtual ~compPreprocessor();

    /// Process source file into one large file.
    /// Parser is initialised with the expanded file.
    bool Preprocess(ISourceFile *mainFile, compParser& parser);

    /// Member access
    compLineNumberMapping& LineNumberMap() { return lineNumberMap; }
};

#endif
