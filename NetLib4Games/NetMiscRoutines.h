/*	MiscRoutines.h

	Created 3-Jan-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

*/
#ifndef _NetMiscRoutines_h
#define _NetMiscRoutines_h

#include <string>

#ifndef byte
typedef unsigned char byte;
#endif

namespace NetLib4Games {

// Convert int to string
std::string IntToString (int i);

// Convert string to int
int StringToInt (std::string& s);

/// Convert std::string to lowercase
std::string LowerCase (std::string str);
/// Convert std::string to uppercase
std::string UpperCase (std::string str);

template<typename T> T Min(T a, T b) {
	return a < b ? a : b;
}
template<typename T> T Max(T a, T b) {
	return a > b ? a : b;
}

}

#endif