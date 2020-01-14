#include "settings.h"
#include "FilenameRoutines.h"
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

using namespace std;

std::vector<std::tuple<int, int>> resolutions = {
	tuple<int, int>(640, 480),
	tuple<int, int>(800, 600),
	tuple<int, int>(1024, 768),
	tuple<int, int>(1152, 864),
	tuple<int, int>(1600, 1200),
	tuple<int, int>(1280, 720),
	tuple<int, int>(1366, 768),
	tuple<int, int>(1600, 900),
	tuple<int, int>(1920, 1080),
	tuple<int, int>(2560, 1440),
	tuple<int, int>(3840, 2160),
	tuple<int, int>(1280, 800),
	tuple<int, int>(1440, 900),
	tuple<int, int>(1680, 1050),
	tuple<int, int>(1920, 1200),
	tuple<int, int>(2560, 1600)
};

OpenGLWindowSettings::OpenGLWindowSettings() :
	screenMode(3),
	colourDepth(0),
	isFullScreen(false),
	windowBorder(true),
	allowResizing(false),
	stencil(false)
{
}

int OpenGLWindowSettings::GetXResolution() const
{
	if (screenMode == 0 || screenMode > resolutions.size()) return 0;
	return get<0>(resolutions[screenMode - 1]);
}

int OpenGLWindowSettings::GetYResolution() const
{
	if (screenMode == 0 || screenMode > resolutions.size()) return 0;
	return get<1>(resolutions[screenMode - 1]);
}

int OpenGLWindowSettings::GetBPP() const
{
	switch (colourDepth)
	{
	case 1: return 16;
	case 2: return 32;
	default: return 0;
	}
}

void OpenGLWindowSettings::PopulateResolutionCombo(QComboBox* combo)
{
	combo->addItem("Use desktop");
	for (auto res : resolutions)
	{
		// Calculate ratio
		int w = get<0>(res);
		int h = get<1>(res);
		for (int i = 2; i <= w / 2; i++)
		{
			while ((w % i) == 0 && (h % i) == 0)
			{
				w /= i;
				h /= i;
			}
		}

		if (h == 5)
		{
			// For some reason ratios are expressed as 16:10 instead of 8:5
			w *= 2;
			h *= 2;
		}

		combo->addItem(QString::number(get<0>(res)) + "x" + QString::number(get<1>(res)) + " (" + QString::number(w) + ":" + QString::number(h) + ")");
	}

	combo->setCurrentIndex(3);
}

FolderSettings::FolderSettings() :
	saveLoadDefaultFolder(GetDefaultProgramsFolder())
{
}

QString FolderSettings::GetDefaultProgramsFolder()
{
	// Default programs folder is "Basic4GL" in the all users documents folder.
	std::string folder = IncludeSlash(GetDocumentsFolder(true)) + "Basic4GL\\";
	return QString(folder.c_str());
}

void Basic4GLSettings::Load()
{
	QSettings s("Basic4GL.com", "Basic4GL");

	window.screenMode = s.value("window.screenMode", window.screenMode).toInt();
	window.colourDepth = s.value("window.colourDepth", window.colourDepth).toInt();
	window.isFullScreen = s.value("window.isFullScreen", window.isFullScreen).toBool();
	window.windowBorder = s.value("window.windowBorder", window.windowBorder).toBool();
	window.allowResizing = s.value("window.allowResizing", window.allowResizing).toBool();
	window.stencil = s.value("window.stencil", window.stencil).toBool();

	folder.saveLoadDefaultFolder = s.value("folder.saveLoadDefaultFolder", folder.saveLoadDefaultFolder).toString();
	folder.recentFileList = s.value("folder.recentFileList", folder.recentFileList).toStringList();

	text.isAutocompleteEnabled = s.value("editor.isAutocompleteEnabled", true).toBool();
	text.isFunctionSignatureEnabled = s.value("editor.isFunctionSignatureEnabled", true).toBool();

	isSandboxMode = s.value("isSandboxMode", isSandboxMode).toBool();
	defaultSyntax = s.value("defaultSyntax", defaultSyntax).toInt();
}

void Basic4GLSettings::Save()
{
	QSettings s("Basic4GL.com", "Basic4GL");

	s.setValue("window.screenMode", window.screenMode);
	s.setValue("window.colourDepth", window.colourDepth);
	s.setValue("window.isFullScreen", window.isFullScreen);
	s.setValue("window.windowBorder", window.windowBorder);
	s.setValue("window.allowResizing", window.allowResizing);
	s.setValue("window.stencil", window.stencil);

	s.setValue("folder.saveLoadDefaultFolder", folder.saveLoadDefaultFolder);
	s.setValue("folder.recentFileList", folder.recentFileList);

	s.setValue("editor.isAutocompleteEnabled", text.isAutocompleteEnabled);
	s.setValue("editor.isFunctionSignatureEnabled", text.isFunctionSignatureEnabled);

	s.setValue("isSandboxMode", isSandboxMode);
	s.setValue("defaultSyntax", defaultSyntax);
}

TextEditorSettings::TextEditorSettings() : isAutocompleteEnabled(true), isFunctionSignatureEnabled(true)
{
}
