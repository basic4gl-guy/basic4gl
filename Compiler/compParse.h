//---------------------------------------------------------------------------
/*  Created 8-Jun-2003: Thomas Mulgrew

    Parse text into tokens.
*/
                                              
#ifndef compParseH
#define compParseH
//---------------------------------------------------------------------------
#include "vmTypes.h"
#include "HasErrorState.h"
//#include "TomVM.h"

enum compTokenType {
        CTT_CONSTANT,
        CTT_TEXT,               // Could be a variable, function name, keyword e.t.c...
        CTT_KEYWORD,            // Note: Parser will never return CTT_KEYWORD,
                                // as it doesnt know what the reserved keywords are.
                                // It is up to the calling code to detect CTT_KEYWORD,
                                // compare it against its known keywords and update
                                // the token type.
        CTT_FUNCTION,           // A builtin function (Same deal as CTT_KEYWORD)
        CTT_USER_FUNCTION,      // A user defined function (Same deal as CTT_KEYWORD)
        CTT_RUNTIME_FUNCTION,   // A special function that will be implemented by runtime compiled code
        CTT_SYMBOL,
        CTT_EOL,
        CTT_EOF};

struct compToken {
    std::string     m_text;
    compTokenType   m_type;
    vmBasicValType  m_valType;              // For constants. Defines the value type.
    bool            m_newLine;              // True if immediately preceeded by newline
    unsigned int m_line, m_col;
};

#ifndef StringVector
typedef std::vector<std::string> StringVector;
#endif

class compParser : public HasErrorState {

    // Text
    StringVector m_sourceCode;

    // Special mode
    bool            m_special;
    std::string     m_specialText;
    unsigned int    m_specialCol;
    int             m_specialSourceLine, m_specialSourceCol;

    // State
    unsigned int m_line, m_col;

    // Reading
    char GetChar (bool inString);
    char PeekChar (bool inString);
    std::string Text () { assert (!Eof ()); return m_sourceCode [m_line]; }
    bool IsNumber (char c);
    bool IsComparison (char c);
public:
    compParser ()               { Reset (); }
    StringVector& SourceCode () { return m_sourceCode; }
    void SetPos (int line, int col) { SetNormal (); m_line = line; m_col = col; ClearError (); }
    void Reset ()               { SetPos (0, 0); }
    int Line ()                 { return m_line; }
    int Col ()                  { return m_col; }
    bool Eof () {
        if (m_special)  return m_specialCol >= m_specialText.length ();
        else            return m_line >= m_sourceCode.size ();
    }

    compToken NextToken (bool skipEOL = false, bool dataMode = false);
    compToken PeekToken (bool skipEOL = false, bool dataMode = false);
        // "dataMode" is set to true when reading elements of a "DATA" statement.
        // (Slightly different parsing rules apply.)

    // Special mode
    bool Special ()                     { return m_special; }
    void SetSpecial (std::string& text, int line = -1, int col = -1) {
        m_special = true;
        m_specialText = text;
        m_specialCol = 0;
        m_specialSourceCol  = col;
        if (line >= 0)  m_specialSourceLine = line;
        else            m_specialSourceLine = m_line;
        if (col >= 0)   m_specialSourceCol = col;
        else            m_specialSourceCol = m_col;
    }
    void SetNormal ()                   { m_special = false; }

};

#endif
