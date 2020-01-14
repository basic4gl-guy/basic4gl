#include "Basic4glMisc.h"
#include <QMessageBox>
#include <QProcess>
#include <QDir>

// Helper routines

void ShowMessage(std::string text)
{
	QMessageBox msgBox;
	msgBox.setText(text.c_str());
	msgBox.exec();
}

std::string CppString(QString str)
{
	// Weirdly str.toLatin1().data() does not work. (An intermediate object is deleted or something)
	QByteArray bytes = str.toLatin1();
	const char* chars = bytes.data();
	return std::string(chars);
}

int GetListItemIndex(QListWidget* list, QListWidgetItem* item)
{
	for (int i = 0; i < list->count(); i++)
		if (item == list->item(i))
			return i;

	return -1;
}

void ShowFileInExplorer(QWidget *parent, const QString pathIn)
{
	// Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
	const QString explorer = "explorer.exe";
	//if (explorer.isEmpty()) {
	//	QMessageBox::warning(parent,
	//		"Launching Windows Explorer failed",
	//		"Could not find explorer.exe in path to launch Windows Explorer.");
	//	return;
	//}
	QString param;
	if (!QFileInfo(pathIn).isDir())
		param = QLatin1String("/select,");
	param += QDir::toNativeSeparators(pathIn);
	QString command = explorer + " " + param;
	QProcess::startDetached(command);
#elif defined(Q_OS_MAC)
	Q_UNUSED(parent)
		QStringList scriptArgs;
	scriptArgs << QLatin1String("-e")
		<< QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
		.arg(pathIn);
	QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
	scriptArgs.clear();
	scriptArgs << QLatin1String("-e")
		<< QLatin1String("tell application \"Finder\" to activate");
	QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
	// we cannot select a file here, because no file browser really supports it...
	const QFileInfo fileInfo(pathIn);
	const QString folder = fileInfo.absoluteFilePath();
	const QString app = Utils::UnixUtils::fileBrowser(Core::ICore::instance()->settings());
	QProcess browserProc;
	const QString browserArgs = Utils::UnixUtils::substituteFileBrowserParameters(app, folder);
	if (debug)
		qDebug() << browserArgs;
	bool success = browserProc.startDetached(browserArgs);
	const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
	success = success && error.isEmpty();
	if (!success)
		showGraphicalShellError(parent, app, error);
#endif
}
