/*	Routines.h
	Copyright (C) Tom Mulgrew 2008 (tmulgrew@slingshot.co.nz)
	
	General purpose helper routines.
*/

#ifndef _routines_h
#define _routines_h
#include <vector>
#include <string>

/// Break down program argument string into separate arguments.
std::vector<std::string> GetArguments(std::string s);

#endif