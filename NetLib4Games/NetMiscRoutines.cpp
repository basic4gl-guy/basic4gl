/*	MiscRoutines.cpp

	Created 3-Jan-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

*/
#include "NetMiscRoutines.h"
#include <stdio.h>

namespace NetLib4Games {

#ifndef _MSC_VER
#define tolower std::tolower
#define toupper std::toupper
#endif

std::string IntToString (int i)   {
	char buffer [20];
	sprintf (buffer, "%d", i);
	return (std::string) buffer;
}
int StringToInt (std::string& s)   {
	int i;
	sscanf (s.c_str (), "%d", &i);
	return i;
}
std::string LowerCase (std::string str) {
    for (unsigned int i = 0; i < str.length (); i++)
        str [i] = tolower (str [i]);
    return str;
}
std::string UpperCase (std::string str) {
    for (unsigned int i = 0; i < str.length (); i++)
        str [i] = toupper (str [i]);
    return str;
}

}