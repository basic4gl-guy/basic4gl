/*  Created 5-Jan-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces for communicating between application and virtual machine view
*/
#ifndef VMViewCallbackInterfacesH
#define VMViewCallbackInterfacesH

#include <string>

//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//  ISourceLineFormatter
//
/// Converts line offsets into descriptive source file + line strings.
/// If preprocessing is involved (e.g. combining include files into one large
/// virtual file), the implementor is expected to take this into account.
class ISourceLineFormatter {
public:
    virtual std::string GetSourcePos(int line) = 0;
};

#endif
