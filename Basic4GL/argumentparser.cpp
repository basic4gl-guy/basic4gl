#include "argumentparser.h"
#include "basic4glMisc.h"

ArgumentParser::ArgumentParser() : command(Cmd::NORMAL), debugMode(false)
{
	auto const& args = qApp->arguments();

	// No args -> normal
	if (args.length() <= 1) return;

	// Look for options
	int i = 1;
	while (i < args.length() && args[i].startsWith("-"))
	{
		auto option = args[i].toLower();
		if (option == "-e")
		{
			command = Cmd::EDIT;
		}
		else if (option == "-r")
		{
			command = Cmd::RUN_AND_QUIT;
		}
		else if (option == "-d")
		{
			debugMode = true;
		}
		else
		{
			HasErrorState::SetError(CppString("Unknown option: " + option));
			return;
		}

		i++;
	}

	// Look for program name
	if (i >= args.length())
	{
		// Program name is required for edit and run-and-quit modes
		if (command == Cmd::EDIT || command == Cmd::RUN_AND_QUIT) {
			HasErrorState::SetError("Missing program name");
			return;
		}
	}
	else
	{
		filename = args[i];
		i++;

		// If filename supplied and no command, default to run-and-quit
		if (command == Cmd::NORMAL)
			command = Cmd::RUN_AND_QUIT;
	}

	// Remainder are BASIC program arguments
	while (i < args.length())
	{
		arguments.push_back(CppString(args[i]));
		i++;
	}
}
