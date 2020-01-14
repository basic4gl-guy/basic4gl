#pragma once

#include <QApplication>

#include "HasErrorState.h"

class ArgumentParser : public HasErrorState
{
public:
	enum class Cmd
	{
		NORMAL,
		EDIT,
		RUN_AND_QUIT
	};
private:
	Cmd command;
	QString filename;
	std::vector<std::string> arguments;
	bool debugMode;
public:
	ArgumentParser();	
	Cmd Command() const { return command; };
	QString Filename() const { return filename; }
	std::vector<std::string> const& Arguments() const { return arguments; }
	bool DebugMode() const { return debugMode; }
};

