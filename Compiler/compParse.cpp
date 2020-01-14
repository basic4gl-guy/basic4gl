//---------------------------------------------------------------------------


#pragma hdrstop

#include "compParse.h"

using namespace std;

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

char compParser::PeekChar (bool inString) {
    if (Eof ())                                                     // End of file
        return 0;
    else if (m_special)
        return m_specialText [m_specialCol];
    else if (m_col >= Text ().size () || (Text () [m_col] == '\'' && !inString))   // End of line, or start of comment
        return 13;
    else
        return Text () [m_col];                                     // Regular text
}

char compParser::GetChar (bool inString) {
    char c = PeekChar (inString);   // Read character
    if (c == 13) {                  // Advance position
        m_col = 0;
        m_line++;
    }
    else {
        if (m_special)  m_specialCol++;
        else            m_col++;
    }
    return c;
}

bool compParser::IsNumber (char c) {
    return (c >= '0' && c <= '9') || c == '.';
}

bool compParser::IsComparison (char c) {
    return c == '<' || c == '=' || c == '>';
}

compToken compParser::NextToken (bool skipEOL, bool dataMode) {

    ClearError ();

    // Create token, with some defaults
    compToken t;
    t.m_text    = "";
    t.m_valType = VTP_INT;
    t.m_newLine = m_col == 0;

    // Skip leading whitespace.
    // Detect newlines
    unsigned char c = ' ';
    if (m_special) {                            // Special text being parsed.
        t.m_line = m_specialSourceLine;         // Substitute the position in the source that the special text has been
        t.m_col  = m_specialSourceCol;          // associated with.
    }
    else {
        t.m_line = m_line;
        t.m_col = m_col;
    }
    while (!Eof () && c <= ' ' && (c != 13 || skipEOL)) {
        if (m_special) {                            // Special text being parsed.
            t.m_line = m_specialSourceLine;         // Substitute the position in the source that the special text has been
            t.m_col  = m_specialSourceCol;          // associated with.
        }
        else {
            t.m_line = m_line;
            t.m_col = m_col;
        }
        c           = GetChar (false);
        t.m_newLine = t.m_newLine || m_col == 0;
    }

    // Determine token type
    if (c == 0)
        t.m_type = CTT_EOF;
    else if (c == 13)
        t.m_type = CTT_EOL;
    else if (c == '"') {
        t.m_type    = CTT_CONSTANT;
        t.m_valType = VTP_STRING;
    }
    else if (!dataMode) {
        if (IsNumber (c)) {
            t.m_type = CTT_CONSTANT;
            if (c == '.')
                t.m_valType = VTP_REAL;
        }
        else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
            t.m_type    = CTT_TEXT;
        else
            t.m_type    = CTT_SYMBOL;
    }
    else {

        // Data mode. (And not a quoted string or EOF/EOL).
        // Comma and colon are recognised separators (colon also means end of statement).
        // Anything else is considered part of the data element.
        if (c == ',' || c == ':')
            t.m_type    = CTT_SYMBOL;
        else
            t.m_type    = CTT_TEXT;
    }

    // Empty token types
    if (t.m_type == CTT_EOF || t.m_type == CTT_EOL)
        return t;

    // Extract token text
    // Regular case, not dataMode text
    if (!dataMode || t.m_type != CTT_TEXT) {
        if (!(t.m_type == CTT_CONSTANT && t.m_valType == VTP_STRING))       // Don't include leading quote (if string constant)
            t.m_text    = t.m_text + (char) c;
        c                    = PeekChar (t.m_valType == VTP_STRING);
        unsigned char lcase  = tolower (c);
        bool done            = false;
        bool hex             = false;

        while (!done && !Error ()) {

            // Determine whether found end of token
            switch (t.m_type) {
            case CTT_CONSTANT:
                if (t.m_valType == VTP_STRING) {
                    if (c == '"') {
                        done = true;
                        GetChar (false);     // Skip terminating quote
                    }
                    else if (c == 0 || c == 13) {
                        SetError ("Unterminated string");
                        done = true;
                    }
                }
                else {
                    bool validDecimalPt = c == '.' && t.m_valType == VTP_INT;
                    if (validDecimalPt)                                     // Floating point number
                        t.m_valType = VTP_REAL;
                    bool hexSpecifier = lcase == 'x' && (t.m_text == "0" || t.m_text == "-0");
                    if (hexSpecifier)
                        hex = true;
                    done = !(   (c >= '0' && c <= '9')
                                || validDecimalPt
                                || hexSpecifier                             // Hex specifier
                                || (hex && lcase >= 'a' && lcase <= 'f'));  // Or hex digit
                }
                break;
            case CTT_TEXT:
                done = !(       (c >= 'a' && c <= 'z')
                            ||  (c >= 'A' && c <= 'Z')
                            ||  c == '_'
                            ||  (c >= '0' && c <= '9'));
                if (c == '$' || c == '%' || c == '#' || c == '!')           // Variables can be trailed with a $, #, % or !
                    t.m_text = t.m_text + GetChar (false);
                break;
            case CTT_SYMBOL:
                done = !(IsComparison (t.m_text [0]) && IsComparison (c));
                break;
            default:
                SetError ("Bad token");
            }


            // Store character
            if (!done) {
                t.m_text    = t.m_text + GetChar (t.m_valType == VTP_STRING);
                c           = PeekChar (t.m_valType == VTP_STRING);
                lcase       = tolower (c);
            }
            else {
                // Check token is well formed
                if (t.m_type == CTT_CONSTANT && t.m_valType == VTP_INT) {
                    // Check integer number is valid
                    char last = t.m_text[t.m_text.size() - 1];
                    if (last == 'x' || last == 'X')
                        SetError((string)"'" + t.m_text + "' is not a valid number");
                }
            }
        }
    }
    else {

        // Special case: dataMode text
        // Token is actually a text constant
        t.m_type = CTT_CONSTANT;
        bool done            = false;
        bool hex             = false;
        bool firstIteration  = true;
        bool whiteSpaceFound = false;
        unsigned char lcase  = tolower (c);

        while (!done && !Error ()) {

            // Comma, colon, return, newline and eof are separators
            // Double quotes are not allowed (syntactically), so we truncate the
            // token at that point
            done =      c == 13
                    ||  c == 10
                    ||  c == 0
                    ||  c == ','
                    ||  c == ':'
                    ||  c == '"';
            if (!done) {

                // Value type rules.
                // Assume numeric until non-numeric character found.
                // (Also, numeric types can't have spaces).
                // Decimal point means floating point (real) type.
                if (t.m_valType != VTP_STRING) {
                    bool validDecimalPt = c == '.' && t.m_valType == VTP_INT;
                    if (validDecimalPt)                                   // Floating point number
                        t.m_valType = VTP_REAL;
                    bool hexSpecifier = lcase == 'x' && (t.m_text == "0" || t.m_text == "-0");
                    if (hexSpecifier)
                        hex = true;
                    if (!(  (c >= '0' && c <= '9')
                            ||  validDecimalPt                              // Regular decimal number
                            ||  hexSpecifier
                            ||  (hex && lcase >= 'a' && lcase <= 'f')       // Or hex digit
                            ||  (firstIteration && c == '-')                // Negative sign
                            ||  (c <= ' '))                                 // Trailing whitespace
                        ||  (c > ' ' && whiteSpaceFound))                   // Contained whitespace
                        t.m_valType = VTP_STRING;
                }
                if (c <= ' ')
                    whiteSpaceFound = true;
            }
            if (!done) {
                if (firstIteration)
                    t.m_text = c;
                else
                    t.m_text    = t.m_text + GetChar (false);
                c           = PeekChar (false);
                lcase       = tolower (c);
            }
            else {
                // Check token is well formed
                if (t.m_type == CTT_CONSTANT && t.m_valType == VTP_INT) {
                    // Check integer number is valid
                    char last = t.m_text[t.m_text.size() - 1];
                    if (last == 'x' || last == 'X')
                        SetError((string)"'" + t.m_text + "' is not a valid number");
                }
            }
            firstIteration = false;
        }

        // Trim trailing whitespace
        int end = t.m_text.length () - 1;
        while (end >= 0 && t.m_text [end] <= ' ')
            end--;
        t.m_text = t.m_text.substr (0, end + 1);
    }

    return t;
}

compToken compParser::PeekToken (bool skipEOL, bool dataMode) {

    // Save position
    int line = m_line, col = m_col;

    // Read token
    compToken t = NextToken (skipEOL, dataMode);

    // Restore position. (Except on error, when we leave the cursor pointing to
    // the error position.)
    if (!Error ()) {
        m_line  = line;
        m_col   = col;
    }

    return t;
}

