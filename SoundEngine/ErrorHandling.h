/*	ErrorHandling

	Created 25-May-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	Simple base class for objects with error state.
*/

#ifndef _ErrorHandling_h
#define _ErrorHandling_h

#include <string>
#include "ThreadUtils.h"

namespace SndEngine {

///////////////////////////////////////////////////////////////////////////////
//	HasErrorState
//
///	Base class for objects which have an error state
class HasErrorState {
private:
	// Error state
	bool		error;
	std::string	errorText;

protected:
	void ClearError() { error = false; errorText = ""; }
	void SetError(std::string text) { error = true; errorText = text; }

public:
	HasErrorState() : error(false), errorText("") { ; }

	// External interface to query error state
	bool Error() const            { return error; }
	std::string ErrorText() const { return errorText; }
};

}
#endif