#include "basic4gleditor.h"
#include "argumentparser.h"
#include "basic4glMisc.h"
#include "TomStdBasicLib.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// Parse arguments
	ArgumentParser args;
	if (args.Error())
	{
		ShowMessage(args.GetError());
		return 1;
	}

	// Otherwise run application normally
	Basic4GLEditor w;
	w.SetDebugMode(args.DebugMode());
	switch (args.Command())
	{
	case ArgumentParser::Cmd::RUN_AND_QUIT:
		w.OpenMainFile(args.Filename());
		SetProgramArguments(args.Arguments());
		w.RunAndQuit();
		break;

	case ArgumentParser::Cmd::EDIT:
		w.OpenMainFile(args.Filename());
		SetProgramArguments(args.Arguments());
		w.show();
		break;

	case ArgumentParser::Cmd::NORMAL:
		w.show();
		break;

	default:
		ShowMessage("Unknown command enum!");
		return 1;
	}

	return a.exec();
}

