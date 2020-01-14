#ifndef CREATESTANDALONEDIALOG_H
#define CREATESTANDALONEDIALOG_H

#include "TomComp.h"
#include "settings.h"
#include <QDialog>

namespace Ui {
class createstandalonedialog;
}

class createstandalonedialog : public QDialog
{
    Q_OBJECT

public:
	explicit createstandalonedialog(std::string basic4glExePath, QWidget *parent = 0);
    ~createstandalonedialog();

	bool Validate();
	bool Execute(QString mainFilePath, TomBasicCompiler& compiler, OpenGLWindowSettings const& windowSettings);

private slots:
    void on_fullScreenCheck_clicked();

    void on_borderCheck_clicked();

    void on_filenameButton_clicked();

	void on_embedAddBtn_clicked();

    void on_embedRemoveBtn_clicked();

private:
    Ui::createstandalonedialog *ui;
	std::string basic4glExePath;

	void UpdateUI();
	bool CreateStandaloneExe(TomBasicCompiler compiler);
	void AddEmbeddedFile(std::string path);
	void LoadProgramSettings(QString filename);
	void SaveProgramSettings(QString filename);
	QString GetSettingsFilename(QString programFilename);
	int GetFinishedOption();
	void SetFinishedOption(int value);
	int GetErrorOption();
	void SetErrorOption(int value);
	int GetColourDepth();
	void SetColourDepth(int value);
	int GetStartupWindowOption();
	void SetStartupWindowOption(int value);

	std::string ReadLine(std::istream& src);
	int ReadInt(std::istream& src) { std::string line = ReadLine(src); return StringToInt(line); }
	int ReadBool(std::istream& src) { std::string line = ReadLine(src); return line == "1"; }
	void WriteBool(std::ostream& dst, bool val) { dst << (val ? "1" : "0") << std::endl; }
};

#endif // CREATESTANDALONEDIALOG_H
