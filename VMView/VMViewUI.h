/*  Created 21-Dec-07: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    Interfaces for communicating between application and virtual machine view
*/
#ifndef _VMViewUI_h
#define _VMViewUI_h

#include <string>

////////////////////////////////////////////////////////////////////////////////
//  IGridSource
//
/// Fetches data to populate a grid
class IGridSource {
public:
    virtual int GetRowCount() = 0;
    virtual int GetFirstRowIndex() = 0;
    virtual std::string GetValue(int row, int col) = 0;
};

#endif


