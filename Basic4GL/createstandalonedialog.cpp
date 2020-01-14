#include "createstandalonedialog.h"
#include "ui_createstandalonedialog.h"
#include "../Standalone/Standalone.h"
#include "../Standalone/ResourceIDs.h"
#include "EmbeddedFiles.h"
#include "Basic4glMisc.h"
#include <QFileInfo>
#include <QFileDialog>
#include <assert.h>
#include <windows.h>
#include <fstream>
#include <QMessageBox>

using namespace std;

string GetLastErrorStr()
{
	char buf[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, nullptr);
	return string(buf);
}

createstandalonedialog::createstandalonedialog(std::string basic4glExePath, QWidget *parent) :
    QDialog(parent),
	ui(new Ui::createstandalonedialog),
    basic4glExePath(basic4glExePath)
{
	// Setup UI
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	OpenGLWindowSettings::PopulateResolutionCombo(ui->resCombo);

	UpdateUI();
}

createstandalonedialog::~createstandalonedialog()
{
    delete ui;
}

bool createstandalonedialog::Validate()
{
	if (ui->filenameEdit->text().isEmpty())
	{
		ShowMessage("Please enter a filename");
	}

	return true;
}

int createstandalonedialog::GetFinishedOption()
{
	return ui->finishCloseBtn->isChecked() ? 0
		       : ui->finishWaitKeyBtn->isChecked() ? 1
		       : 2;
}

void createstandalonedialog::SetFinishedOption(int value)
{
	ui->finishCloseBtn->setChecked(value == 0);
	ui->finishWaitKeyBtn->setChecked(value == 1);
	ui->finishWaitCloseBtn->setChecked(value == 2);
}

int createstandalonedialog::GetErrorOption()
{
	return ui->errorCloseBtn->isChecked() ? 0
		       : ui->errorGeneralBtn->isChecked() ? 1
		       : 2;
}

void createstandalonedialog::SetErrorOption(int value)
{
	ui->errorCloseBtn->setChecked(value == 0);
	ui->errorGeneralBtn->setChecked(value == 1);
	ui->errorSpecificBtn->setChecked(value == 2);
}

std::string createstandalonedialog::ReadLine(std::istream& src)
{
	char line[1000];
	src.getline(line, 1000);
	return string(line);
}

int createstandalonedialog::GetColourDepth()
{
	return ui->depthDesktopBtn->isChecked() ? 0
		       : ui->depth16Btn->isChecked() ? 1
		       : 2;
}

void createstandalonedialog::SetColourDepth(int value)
{
	ui->depthDesktopBtn->setChecked(value == 0);
	ui->depth16Btn->setChecked(value == 1);
	ui->depth32Btn->setChecked(value == 2);
}

int createstandalonedialog::GetStartupWindowOption()
{
	return ui->createWindowBtn->isChecked() ? 0 : 1;
}

void createstandalonedialog::SetStartupWindowOption(int value)
{
	ui->createWindowBtn->setChecked(value == 0);
	ui->dontCreateWindowBtn->setChecked(value == 1);
}

bool createstandalonedialog::CreateStandaloneExe(TomBasicCompiler comp)
{
	// Check if file exists
	string outputFilename = CppString(ui->filenameEdit->text());
	{
		// Check whether output file exists by opening (briefly) for read.
		ifstream prevFileStream(outputFilename);
		if (!prevFileStream.fail()) {
		
			// File exists. Confirm overwrite.
			if (QMessageBox::question(
					this, 
					"Confirm overwrite", 
					"File already exists. Overwrite it?", 
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				return false;
		}
	}

	// Setup patch data.
	// Write it to a stream first, so that we can calculate the size
	stringstream patchData;

	// Header first
	StandaloneHeader header;
	header.version = STANDALONE_VERSION;
	memset(header.title, 0, 256);
	string title = CppString(ui->titleEdit->text());
	assert(title.size() < 255);
	memcpy(header.title, title.c_str(), title.size());
	header.finishedOption = GetFinishedOption();
	header.errorOption = GetErrorOption();
	header.useDesktopRes = ui->resCombo->currentIndex() == 0;
	if (header.useDesktopRes)
	{
		header.screenWidth = 0;
		header.screenHeight = 0;
	}
	else
	{
		// Lookup corresponding resolution
		auto res = resolutions[ui->resCombo->currentIndex() - 1];
		header.screenWidth = get<0>(res);
		header.screenHeight = get<1>(res);
	}
	header.colourDepth = GetColourDepth();
	header.stencil = ui->stencilCheck->isChecked();
	header.fullScreen = ui->fullScreenCheck->isChecked();
	header.escKeyQuits = ui->escQuitCheck->isChecked();
	header.border = ui->borderCheck->isChecked();
	header.allowResizing = ui->resizeCheck->isChecked();
	header.startupWindowOption = GetStartupWindowOption();

	patchData.write((char*)&header, sizeof(header));

	// Write compiler and vm state to stream
	stringstream stateStream;
	comp.StreamOut(stateStream);

	// Append length to patch
	int32_t streamLength = stateStream.tellp();
	patchData.write((char*)&streamLength, sizeof(streamLength));

	// ...Followed by state
	stateStream.seekg(0, ios::beg);
	CopyStream(stateStream, patchData, streamLength);

	// ...Then embedded files

	int32_t fileCount = ui->embedList->count() + 1;
	// We always embed charset.png (although this may be made optional in a
	// later version) so that the program can display text.
	patchData.write((char*)&fileCount, sizeof(fileCount));

	// We need to do a bit of current directory swapping to make charset.png
	// appear as if it's in the same directory as the program.
	char filePath[1024];
	GetCurrentDirectory(1024, filePath);
	SetCurrentDirectory(basic4glExePath.c_str());
	if (!EmbedFile("charset.png", patchData)) {
		SetCurrentDirectory(filePath);
		ShowMessage("Couldn't embed charset.png");
		return false;
	}
	SetCurrentDirectory(filePath);

	// Embedd regular files
	for (int j = 0; j < ui->embedList->count(); j++) {
		string filename = CppString(ui->embedList->item(j)->text());
		if (!EmbedFile(filename.c_str(), patchData)) {
			ShowMessage("Couldn't embed " + filename);
			return false;
		}
	}

	// Calculate new data length, and length of data section (padded length)
	int dataLength = patchData.tellp();

	// Scan source file.
	// Extract information and verify file integrity.
	//
	// Info:
	//  * Current size of Basic4GL section
	//
	// Integrity:
	//  * File is windows executable
	//  * Last section is Basic4GL section

	// Source filename
	string stub = basic4glExePath + "\\Stub.bin";

	// Copy stub to output file
	if (CopyFile(
		stub.c_str(),
		outputFilename.c_str(),
		false) == 0)
	{
		ShowMessage("Error creating output file: " + GetLastErrorStr());
		return false;
	}

	// Write patch data into resource section.
	HANDLE res = BeginUpdateResource(
		outputFilename.c_str(),
		true);                              // Delete existing resources.

	if (res == NULL) {
		ShowMessage("Error patching Basic4GL program into exe (BeginUpdateResource): " + GetLastErrorStr());
		return false;
	}

	// Convert stream data to a big fat array, as UpdateResource wants a pointer.
	char *data = new char[dataLength];
	patchData.seekg(0, ios::beg);
	patchData.read(data, dataLength);
	if (!UpdateResource(
		res,
		"BIN",
		MAKEINTRESOURCE(IDR_BASIC4GLDATA),
		0,
		data,
		dataLength)) {
		delete[] data;
		EndUpdateResource(res, true);
		ShowMessage("Error patching Basic4GL program into exe (UpdateResource): " + GetLastErrorStr());
		return false;
	}
	delete[] data;

	if (!EndUpdateResource(res, false)) {
		ShowMessage("Error patching Basic4GL program into exe (EndUpdateResource): " + GetLastErrorStr());
		return false;
	}

	ShowFileInExplorer(this, ui->filenameEdit->text());

	return true;
}

void createstandalonedialog::AddEmbeddedFile(std::string path)
{
	// Convert to relative path
	string relPath = ProcessPath(path);
	QString qPath = relPath.c_str();

	// Check if already in list
	for (int i = 0; i < ui->embedList->count(); i++)
		if (ui->embedList->item(i)->text() == qPath)
			return;

	// Add new item
	ui->embedList->addItem(qPath);
}

void createstandalonedialog::LoadProgramSettings(QString filename)
{
	if (filename.isEmpty()) return;

	// Attempt to open settings file
	QString settingsFilename = GetSettingsFilename(filename);
	ifstream src(CppString(settingsFilename).c_str(), ios::in);
	if (src.fail()) return;				// Assume not present

	// Load embedded file names
	int count = ReadInt(src);
	for (int i = 0; i < count; i++)
		AddEmbeddedFile(ReadLine(src));

	// Read title and output filename
	string title = ReadLine(src);
	if (title != "")
		ui->titleEdit->setText(title.c_str());
	string outputFilename = ReadLine(src);
	if (outputFilename != "")
		ui->filenameEdit->setText(outputFilename.c_str());

	// Read Options tab settings
	SetFinishedOption(ReadInt(src));
	SetErrorOption(ReadInt(src));
	ui->escQuitCheck->setChecked(ReadBool(src));

	// Load display options settings
	ui->resCombo->setCurrentIndex(ReadInt(src));
	SetColourDepth(ReadInt(src));
	SetStartupWindowOption(ReadInt(src));
	ui->stencilCheck->setChecked(ReadBool(src));
	ui->fullScreenCheck->setChecked(ReadBool(src));
	ui->borderCheck->setChecked(ReadBool(src));
	ui->resizeCheck->setChecked(ReadBool(src));
}

void createstandalonedialog::SaveProgramSettings(QString filename)
{
	if (filename.isEmpty()) return;

	// Open settings file
	QString settingsFilename = GetSettingsFilename(filename);
	ofstream dst(CppString(settingsFilename).c_str(), ios::out);

	// Save embedded file names
	dst << ui->embedList->count() << endl;
	for (int i = 0; i < ui->embedList->count(); i++)
		dst << CppString(ui->embedList->item(i)->text()) << endl;

	// Save title and output filename
	dst << CppString(ui->titleEdit->text()) << endl;
	dst << CppString(ui->filenameEdit->text()) << endl;

	// Save Options tab settings
	dst << GetFinishedOption() << endl;
	dst << GetErrorOption() << endl;
	WriteBool(dst, ui->escQuitCheck->isChecked());

	// Save display options settings
	dst << ui->resCombo->currentIndex() << endl;
	dst << GetColourDepth() << endl;
	dst << GetStartupWindowOption() << endl;
	WriteBool(dst, ui->stencilCheck->isChecked());
	WriteBool(dst, ui->fullScreenCheck->isChecked());
	WriteBool(dst, ui->borderCheck->isChecked());
	WriteBool(dst, ui->resizeCheck->isChecked());
}

QString createstandalonedialog::GetSettingsFilename(QString programFilename)
{
	QFileInfo fileInfo(programFilename);
	return fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + ".standalone";
}

bool createstandalonedialog::Execute(QString mainFilePath, TomBasicCompiler& compiler, OpenGLWindowSettings const& windowSettings)
{
	// Setup UI
	if (!mainFilePath.isEmpty()) {
		QFileInfo fileInfo(mainFilePath);
		ui->filenameEdit->setText(fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + ".exe");
		ui->titleEdit->setText(fileInfo.completeBaseName());
	}
	else
	{
		ui->filenameEdit->setText("MyProgram.exe");
		ui->titleEdit->setText("My program");
	}

	// Setup UI from settings
	ui->resCombo->setCurrentIndex(windowSettings.screenMode);
	ui->depthDesktopBtn->setChecked(windowSettings.colourDepth == 0);
	ui->depth16Btn->setChecked(windowSettings.colourDepth == 1);
	ui->depth32Btn->setChecked(windowSettings.colourDepth == 2);
	ui->stencilCheck->setChecked(windowSettings.stencil);
	ui->fullScreenCheck->setChecked(windowSettings.isFullScreen);
	ui->borderCheck->setChecked(windowSettings.windowBorder);
	ui->resizeCheck->setChecked(windowSettings.allowResizing);

	// Look for strings in the BASIC program that correspond to files, and add them as embedded resources.
	ui->embedList->clear();
	for (auto str : compiler.VM().StringConstants())
	{
		// Does string refer to an existing file?
		QFileInfo fileInfo(str.c_str());
		if (fileInfo.exists() && fileInfo.isFile())
		{
			// Add to list
			AddEmbeddedFile(str);
		}
	}

	// Load saved program settings
	LoadProgramSettings(mainFilePath);

	UpdateUI();

	// Execute dialog
	do
	{
		if (exec() == 0) return false;
	} while (!Validate());

	// Create standalone exe
	SaveProgramSettings(mainFilePath);
	return CreateStandaloneExe(compiler);
}

void createstandalonedialog::UpdateUI()
{
	if (ui->fullScreenCheck->isChecked())
	{
		ui->depthDesktopBtn->setEnabled(true);
		ui->depth16Btn->setEnabled(true);
		ui->depth32Btn->setEnabled(true);
		ui->borderCheck->setChecked(false);
		ui->borderCheck->setEnabled(false);
	}
	else
	{
		ui->depthDesktopBtn->setChecked(true);
		ui->depthDesktopBtn->setEnabled(false);
		ui->depth16Btn->setEnabled(false);
		ui->depth32Btn->setEnabled(false);
		ui->borderCheck->setEnabled(true);
	}

	if (ui->borderCheck->isChecked())
	{
		ui->resizeCheck->setEnabled(true);
	}
	else
	{
		ui->resizeCheck->setChecked(false);
		ui->resizeCheck->setEnabled(false);
	}
}

void createstandalonedialog::on_fullScreenCheck_clicked()
{
	UpdateUI();
}

void createstandalonedialog::on_borderCheck_clicked()
{
	UpdateUI();
}

void createstandalonedialog::on_filenameButton_clicked()
{
	QString newFilename = QFileDialog::getSaveFileName(
		this,
		"Standalone exe filename",
		ui->filenameEdit->text(),
		tr("Executable file (*.exe)"));

	if (!newFilename.isEmpty())
	{
		ui->filenameEdit->setText(newFilename);
	}
}

void createstandalonedialog::on_embedAddBtn_clicked()
{
	QStringList filenames = QFileDialog::getOpenFileNames(
		this,
		"Select file(s) to add",
		"",
		"All files (*.*)");
	for (auto filename : filenames)
	{
		AddEmbeddedFile(CppString(filename));
	}	
}

void createstandalonedialog::on_embedRemoveBtn_clicked()
{
	auto items = ui->embedList->selectedItems();
	for (auto item : items)
	{
		delete item;
	}
}
