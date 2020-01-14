#include "vmCodeBlock.h"
#include "streaming.h"
using namespace std;

string PrepCodeBlockFilename(string filename)
{
	// Convert to lowercase
	string result = LowerCase(filename);

	// Replace \ with /
	for (int i = 0; i < result.length(); i++)
		if (result[i] == '\\')
			result[i] = '/';

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//  vmCodeBlock

void vmCodeBlock::SetLengthAtLeast(int length) {
    if (runtimeFunctions.size() < (unsigned)length)
        runtimeFunctions.resize(length);
}

vmRuntimeFunction& vmCodeBlock::GetRuntimeFunction(int index) {
    assert(index >= 0);
    SetLengthAtLeast(index + 1);
    return runtimeFunctions[index];
}

void vmCodeBlock::SetFilename(string _filename)
{
	filename = PrepCodeBlockFilename(_filename);
}

bool vmCodeBlock::FilenameEquals(string _filename)
{
	return filename == PrepCodeBlockFilename(_filename);
}

#ifdef VM_STATE_STREAMING
void vmCodeBlock::StreamOut(ostream& stream) const {
	WriteString(stream, filename);
    WriteLong(stream, programOffset);
    WriteLong(stream, runtimeFunctions.size());
/*#ifdef __ANDROID__
    // (No cbegin/cend on android)
    for (vector<vmRuntimeFunction>::const_iterator i = runtimeFunctions.begin();
         i != runtimeFunctions.end();
         i++)
        i->StreamOut(stream);
#else*/
    for (vector<vmRuntimeFunction>::const_iterator i = runtimeFunctions.cbegin();
            i != runtimeFunctions.cend();
            i++)
        i->StreamOut(stream);

	// User functions
	WriteLong(stream, userFunctions.size());
	for (auto fn : userFunctions)
	{
		WriteString(stream, fn.first);
		WriteLong(stream, fn.second);
	}
//#endif
}

void vmCodeBlock::StreamIn(istream& stream) {
	filename = ReadString(stream);
    programOffset = ReadLong(stream);
    int count = ReadLong(stream);
    runtimeFunctions.resize(count);
    for (vector<vmRuntimeFunction>::iterator i = runtimeFunctions.begin();
            i != runtimeFunctions.end();
            i++)
        i->StreamIn(stream);

	// User functions
	userFunctions.clear();
	count = ReadLong(stream);
	for (int i = 0; i < count; i++)
	{
		std::string name = ReadString(stream);
		int index = ReadLong(stream);
		userFunctions[name] = index;
	}
}
#endif

