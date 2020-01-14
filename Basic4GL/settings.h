#pragma once

#include <QString>
#include <QStringList>
#include <QComboBox>
#include <vector>

extern std::vector<std::tuple<int, int>> resolutions;

struct OpenGLWindowSettings
{
	int screenMode;
	int colourDepth;
	bool isFullScreen;
	bool windowBorder;
	bool allowResizing;
	bool stencil;

	OpenGLWindowSettings();

	bool IsDesktopResolution() const { return screenMode == 0; }
	bool FitToWorkArea() const { return IsDesktopResolution() && !isFullScreen; }
	int GetXResolution() const;
	int GetYResolution() const;
	int GetBPP() const;
	static void PopulateResolutionCombo(QComboBox* combo);
};

struct FolderSettings
{
	QString saveLoadDefaultFolder;
	QStringList recentFileList;

	FolderSettings();
	static QString GetDefaultProgramsFolder();
};

struct TextEditorSettings
{
	bool isAutocompleteEnabled;
	bool isFunctionSignatureEnabled;

	TextEditorSettings();
};

struct Basic4GLSettings
{
	OpenGLWindowSettings window;
	FolderSettings folder;
	TextEditorSettings text;
	int defaultSyntax;
	bool isSandboxMode;

	Basic4GLSettings() : 
		defaultSyntax(1),
		isSandboxMode(true)
	{
		Load();
	}

	~Basic4GLSettings()
	{
		Save();
	}

	void Load();
	void Save();
};
