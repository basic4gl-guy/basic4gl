/*  Created 20-May-2007: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

    General purpose filename routines.
*/

#ifndef FilenameRoutinesH
#define FilenameRoutinesH

#include <string>

// Prepares a file path for comparison with another.
// Converts all characters to lowercase and exchanges any '/' symbols with '\'
void PrepPathForComp (char *string);

// Prepares a file path for comparison with another.
// Converts all characters to lowercase and exchanges any '/' symbols with '\'
std::string PrepPathForComp(std::string str);

// Include trailing slash in path
std::string IncludeSlash(std::string path);

// Process a path (and filename) for string matching
// Returns the path relative to the current directory, cast to lowercase.
std::string ProcessPath (std::string filename); 

// Current folder
std::string GetCurrentFolder();

// My documents folder
std::string GetDocumentsFolder(bool allUsers);

// Application data Windows folder
std::string GetAppDataFolder(bool allUsers);

// Expand possibly relative path to full path
std::string GetFullPath(std::string path);

// Read environment variable
std::string GetEnvVariable(std::string name);

// Check if file exists
bool FileExists(std::string name);

#endif
