#include "FilenameRoutines.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <fstream>

void PrepPathForComp (char *string) {

	// Prepare file path for comparison by converting to lowercase, and swapping '/' with '\'
	for (char *ptr = string; *ptr != 0; ptr++)
		if (*ptr >= 'A' && *ptr <= 'Z')
			*ptr = *ptr - 'A' + 'a';
		else if (*ptr == '/')
			*ptr = '\\';
}

std::string PrepPathForComp(std::string str)
{
	if (str == "") return str;

	// Copy to character buffer
	char* buf = new char[str.length() + 1];
	strncpy(buf, str.c_str(), str.length());
	buf[str.length()] = 0;

	// Run string buffer version
	PrepPathForComp(buf);

	// Copy back to string
	std::string result = std::string(buf);
	delete[] buf;
	return result;
}

std::string IncludeSlash(std::string path) {
	if (path != "" && path[path.size() - 1] != '\\' && path[path.size() - 1] != '/')
		path += "\\";
	return path;
}

std::string ProcessPath (std::string filename) {

	if (filename == "") return filename;

	//// Find full pathname
	//char fullPath[1024];
	//char *fileBit;
	//int r = GetFullPathName (filename.c_str (), 1024, fullPath, &fileBit);
	//if (r == 0 || r > 1024)
	//	return filename;
	//PrepPathForComp (fullPath);

	//// Find current directory, convert to lowercase
	//char currentDir[1024];
	//r = GetCurrentDirectory (1024, currentDir);
	//if (r == 0 || r > 1024)
	//	return (std::string) fullPath;
	//PrepPathForComp (currentDir);
	//int curDirLen = strlen (currentDir);
	//if (currentDir [curDirLen - 1] != '\\') {
	//	currentDir [curDirLen] = '\\';
	//	currentDir [curDirLen + 1] = 0;
	//}

	// Find full path and current directory
	std::string fullPathStr = PrepPathForComp(GetFullPath(filename));
	auto fullPath = fullPathStr.c_str();
	std::string currentDirStr = PrepPathForComp(IncludeSlash(GetCurrentFolder()));
	auto currentDir = currentDirStr.c_str();

	// Compare strings for matching directories
	bool relative = false, matches = true;
	int offset = 0;
	while (matches) {
		const char	*p1 = strchr(fullPath + offset, '\\'),
					*p2 = strchr(currentDir + offset, '\\');
		matches =	p1 != nullptr && p2 != nullptr													// Found dir separators
				&&	p1 - (fullPath + offset) == p2 - (currentDir + offset)							// Same # of characters
				&&	memcmp (fullPath + offset, currentDir + offset, p1 - (fullPath + offset)) == 0;	// Directories match
		if (matches) {
			relative = true;
			offset = p1 - fullPath + 1;
		}
	}

	if (!relative)
		return fullPathStr;
	else {
		std::string result = "";
		
		// Look for remaining directories in currentDir that didn't match 
		// For each of these, we need to back out one directory.
		const char *cursor = currentDir + offset;
		const char *p;
		while ((p = strchr (cursor, '\\')) != NULL) {
			result += "..\\";
			cursor = p + 1;
		}
		
		// Append remaining bit of full path
		result += (fullPath + offset);

		return result;
	}
}

std::string GetCurrentFolder()
{
	char buf[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buf);
	return std::string(buf);
}

std::string GetFolderPath(int folder)
{
	char buf[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, folder, NULL, 0, buf)))
		return std::string(buf);
	return "";

}

std::string GetDocumentsFolder(bool allUsers)
{
	return GetFolderPath(allUsers ? CSIDL_COMMON_DOCUMENTS : CSIDL_MYDOCUMENTS);
}

std::string GetAppDataFolder(bool allUsers)
{
	return GetFolderPath(allUsers ? CSIDL_COMMON_APPDATA : CSIDL_APPDATA);
}

std::string GetFullPath(std::string path)
{
	char buf[MAX_PATH];
	char* fileBit;
	GetFullPathName(path.c_str(), MAX_PATH, buf, &fileBit);
	return std::string(buf);
}

std::string GetEnvVariable(std::string name)
{
	char buf[32767];
	int size = 32767;
	int result = GetEnvironmentVariable(name.c_str(), buf, size);
	return result != 0 ? std::string(buf) : "";
}

bool FileExists(std::string name)
{
	std::ifstream file(name.c_str());
	return !file.fail();
}
