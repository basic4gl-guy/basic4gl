// Routines for exporting VM state for platform runtimes

#pragma once

#include "TomComp.h"
#include "TomVM.h"
#include <string>

/// Export virtual machine state for loading into platform runtimes
void ExportVMState(std::ofstream& stream, TomVM const& vm, TomBasicCompiler const& compiler);

/// Export virtual machine state for loading into platform runtimes
void ExportVMState(std::string filename, const TomVM & vm, const TomBasicCompiler & compiler);

///////////////////////////////////////////////////////////////////////////////
//	PluginDLLManagerForStateExport
//
/// Custom plugin DLL manager for exporting state for platform runtimes.
/// Removes the plugin information from the export as the platform runtimes 
/// don't use it
class PluginDLLManagerForStateExport : public PluginDLLManager
{
public:
	PluginDLLManagerForStateExport(const std::string& _directory, const bool _isStandaloneExe)
		: PluginDLLManager(_directory, _isStandaloneExe)
	{
	}

	void StreamOut(std::ostream& stream) const override;
};