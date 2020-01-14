#include "vmStateExport.h"
//#include "Common/vmStateTypes.h"
#include "../../PlatformRuntime/Common/vmStateTypes.h"		// TODO: Should this be moved into Basic4GL source?
#include <fstream>

using namespace std;

FunctionTable BuildFunctionTable(multimap<string, unsigned> const& nameIndex, vector<compFuncSpec> const& specifications)
{
	FunctionTable table;

	// Iterate name->index map to get names and indices
	for (auto mapping : nameIndex)
	{
		string name = mapping.first;
		unsigned int index = mapping.second;

		// Ensure signatures array is large enough
		if (table.functions.size() <= index)
			table.functions.resize(index + 1);

		// Write function specification
		auto& spec = specifications[index];
		table.functions[index] = FunctionSignature(index, name, spec);
	}

	return table;
}

///////////////////////////////////////////////////////////////////////////////
//	Public functions

void ExportVMState(ofstream& stream, TomVM const& vm, TomBasicCompiler const& compiler)
{
	// Write main virtual machine state
	vm.StreamOut(stream);

	// Build list of function signatures by index
	FunctionTable functionTable = BuildFunctionTable(compiler.FunctionIndex(), compiler.Functions());

	// Build similar lists for each plugin library
	vector<FunctionTable> libraryFunctionTables;
	for (auto const& library : vm.Plugins().LoadedLibraries())
	{
		libraryFunctionTables.push_back(BuildFunctionTable(library->FunctionLookup(), library->VMFunctionSpecs()));
	}

	// Scan VM op-codes to find referenced functions
	for (unsigned int i = 0; i < vm.InstructionCount(); i++)
	{
		auto const& instruction = vm.Instruction(i);
		switch (instruction.m_opCode)
		{
		case OP_CALL_FUNC: {
			unsigned int index = instruction.m_value.IntVal();
			assert(index < functionTable.functions.size());
			functionTable.functions[index].isReferenced = true;
			break;
		}
		case OP_CALL_DLL: {
			unsigned int index = instruction.m_value.IntVal();
			unsigned int libIndex = index >> 24;
			unsigned int funcIndex = index & 0x00ffffff;
			assert(libIndex < libraryFunctionTables.size());
			assert(funcIndex < libraryFunctionTables[libIndex].functions.size());
			libraryFunctionTables[libIndex].functions[funcIndex].isReferenced = true;
			break;
		}
		}
	}

	// Append function specifications
	functionTable.StreamOut(stream);

	// Append library function specifications
	WriteLong(stream, libraryFunctionTables.size());
	for (auto const& libFuncSpec : libraryFunctionTables)
		libFuncSpec.StreamOut(stream);
}

void ExportVMState(std::string filename, const TomVM & vm, const TomBasicCompiler & compiler)
{
	// Open file
	ofstream stateFile(filename, ios_base::out | ios_base::binary);
	if (stateFile.fail()) throw exception("Error creating VM state file.");

	// Export state
	ExportVMState(stateFile, vm, compiler);

	// Close file
	if (stateFile.fail()) throw exception("Error writing VM state file.");
	stateFile.close();
}

void PluginDLLManagerForStateExport::StreamOut(std::ostream& stream) const
{
	// Do nothing!
	// Runtime platforms don't use the same plugins
}
