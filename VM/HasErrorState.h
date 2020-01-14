//---------------------------------------------------------------------------
/*  Created 27-Jun-2003: Thomas Mulgrew

    Simple base class for object error handling.

*/

#ifndef HasErrorStateH
#define HasErrorStateH
#include <string>
#include <assert.h>
//---------------------------------------------------------------------------

class HasErrorState {
    // Error status
    bool                        m_error;
    std::string                 m_errorString;

protected:
    virtual void SetError (std::string text) {
        m_error         = true;
        m_errorString   = text;
    }

public:
	virtual ~HasErrorState()
	{
	}

	// Error handling
    HasErrorState () : m_error (false) { ; }
    bool Error() const          { return m_error; }
    std::string GetError() const{ assert (m_error); return m_errorString; }
    void ClearError ()          { m_error = false; }
    void CopyErrorState(HasErrorState const& other) {
        m_error = other.m_error;
        m_errorString = other.m_errorString;
    }
    bool CheckCond(bool condition, std::string errorMessage) {
    	if (!condition) SetError(errorMessage);
    	return condition;
    }
};

#endif
