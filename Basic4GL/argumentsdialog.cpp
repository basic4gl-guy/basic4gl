#include "argumentsdialog.h"
#include "ui_argumentsdialog.h"
#include "TomStdBasicLib.h"
#include "Routines.h"
#include "basic4glMisc.h"

ArgumentsDialog::ArgumentsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ArgumentsDialog)
{
    ui->setupUi(this);
	setFixedSize(width(), height());
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

ArgumentsDialog::~ArgumentsDialog()
{
    delete ui;
}

int ArgumentsDialog::exec()
{
	// Setup dialog
	ui->argumentsEdit->setText(GetArguments());

	// Run modally
	int result = QDialog::exec();

	if (result == 1)
	{
		// Write back changes
		SetArguments(ui->argumentsEdit->text());
	}

	return result;
}

QString ArgumentsDialog::GetArguments()
{
	// Get program arguments vector
	auto argList = GetProgramArguments();

	// Convert to string
	QString argString;
	for (auto arg : argList)
	{
		// Quote entries with spaces
		if (arg.find(' ') != std::string::npos)
		{
			arg = "\"" + arg + "\"";
		}

		// Add to list
		if (!argString.isEmpty())
			argString += " ";
		argString += arg.c_str();
	}

	return argString;
}

void ArgumentsDialog::SetArguments(QString str)
{
	auto argList = ::GetArguments(CppString(str));
	SetProgramArguments(argList);
}
