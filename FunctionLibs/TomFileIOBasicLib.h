//---------------------------------------------------------------------------
/*  Created 02-Nov-2003: Thomas Mulgrew

    File I/O routines
*/
                                                                               
#ifndef TomFileIOBasicLibH
#define TomFileIOBasicLibH
//---------------------------------------------------------------------------
#include "TomComp.h"
#include "EmbeddedFiles.h"

void InitTomFileIOBasicLib (TomBasicCompiler& comp, FileOpener *_files);

/*  FileStream

    Wrapper for any stream objects that can be accessed via the file I/O
    routines.
*/

class FileStream {
public:
	GenericIStream *in;
	GenericOStream *out;
	FileStream () {
		in		= NULL;
		out		= NULL;
	}
	FileStream (GenericIStream *_in) {
		in	= _in;
		out	= NULL;
	}
	FileStream (GenericOStream *_out) {
		in	= NULL;
		out	= _out;
	}
    virtual ~FileStream ();
};

inline bool operator== (const FileStream& s1, const FileStream& s2) {
	return s1.in == s2.in && s1.out == s2.out;
}

////////////////////////////////////////////////////////////////////////////////
//  FileStreamStore

typedef vmPointerResourceStore<FileStream> FileStreamStore;

extern FileStreamStore fileStreams;

#endif
