/*	Routines.cpp
	Copyright (C) Tom Mulgrew 2008 (tmulgrew@slingshot.co.nz)
*/

#include "Routines.h"

using namespace std;

vector<string> GetArguments(string s) {
    vector<string> result;
    int p = 0;
    while (p < s.length()) {

        string arg = "";

        // Skip whitespace
        while (p < s.length() && s[p] <= ' ')
            p++;

        // Quoted?
        if (p < s.length() && s[p] == '"') {

            // Read quoted string
            p++;
            while (p < s.length() && s[p] != '"') {
                arg += s[p];
                p++;
            }

            // Skip end quote
            if (p < s.length())
                p++;
        }
        // Non-quoted
        else {
            // Read up to next whitespace
            while (p < s.length() && s[p] != '"' && s[p] > ' ') {
                arg += s[p];
                p++;
            }
        }

        if (arg != "")
            result.push_back(arg);
    }

    return result;
}
