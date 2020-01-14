#include "finddialog.h"
#include "ui_finddialog.h"
#include <QPushButton>

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
}

FindDialog::~FindDialog()
{
    delete ui;
}

bool FindDialog::ShowFindOptions()
{
	// Configure for find
	ui->replaceLabel->hide();
	ui->replaceEdit->hide();
	ui->buttonBox->clear();
	ui->buttonBox->addButton("Find", QDialogButtonBox::AcceptRole);
	ui->buttonBox->addButton("Cancel", QDialogButtonBox::RejectRole);

	// Show modal
	int result = exec();
	return result == 1;
}

bool FindDialog::ShowReplaceOptions()
{
	// Configure for replace
	ui->replaceLabel->show();
	ui->replaceEdit->show();
	ui->buttonBox->clear();

	auto replaceButton = new QPushButton("Replace");
	replaceButton->setDefault(true);

	auto replaceAllButton = new QPushButton("Replace all");

	ui->buttonBox->addButton(replaceButton, QDialogButtonBox::AcceptRole);
	ui->buttonBox->addButton(replaceAllButton, QDialogButtonBox::AcceptRole);
	ui->buttonBox->addButton("Cancel", QDialogButtonBox::RejectRole);

	connect(replaceButton, SIGNAL(clicked()), this, SLOT(ReplaceClicked()));
	connect(replaceAllButton, SIGNAL(clicked()), this, SLOT(ReplaceAllClicked()));

	// Show modal
	int result = exec();
	return result == 1;
}

QString FindDialog::GetFindText() const
{
	return ui->findEdit->text();
}

QString FindDialog::GetReplaceText() const
{
	return ui->replaceEdit->text();
}

bool FindDialog::IsMatchCase() const
{
	return ui->matchCaseCheckbox->isChecked();
}

bool FindDialog::IsWholeWordsOnly() const
{
	return ui->wholeWordsCheckbox->isChecked();
}

void FindDialog::ReplaceClicked()
{
	isReplaceAll = false;
}

void FindDialog::ReplaceAllClicked()
{
	isReplaceAll = true;
}
