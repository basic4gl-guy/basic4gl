#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	OpenGLWindowSettings::PopulateResolutionCombo(ui->resCombo);
	ui->languageCombo->addItem("Traditional");
	ui->languageCombo->addItem("Basic4GL");
	ui->languageCombo->addItem("Traditional PRINT only");
	ui->languageCombo->addItem("Traditional including variable suffixes");
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

bool OptionsDialog::Execute(Basic4GLSettings& settings)
{
	// Setup UI from settings
	ui->resCombo->setCurrentIndex(settings.window.screenMode);
	ui->depthDesktopBtn->setChecked(settings.window.colourDepth == 0);
	ui->depth16Btn->setChecked(settings.window.colourDepth == 1);
	ui->depth32Btn->setChecked(settings.window.colourDepth == 2);
	ui->stencilCheck->setChecked(settings.window.stencil);
	ui->fullScreenCheck->setChecked(settings.window.isFullScreen);
	ui->borderCheck->setChecked(settings.window.windowBorder);
	ui->resizeCheck->setChecked(settings.window.allowResizing);
	ui->languageCombo->setCurrentIndex(settings.defaultSyntax);
	ui->safeModeCheck->setChecked(settings.isSandboxMode);
	ui->autoCompleteCheck->setChecked(settings.text.isAutocompleteEnabled);
	ui->functionSignatureCheck->setChecked(settings.text.isFunctionSignatureEnabled);
	UpdateUI();

	// Run dialog
	do
	{
		if (exec() == 0) return false;
	} while (!Validate());

	// Update settings from UI
	settings.window.screenMode = ui->resCombo->currentIndex();
	settings.window.colourDepth =
		  ui->depthDesktopBtn->isChecked() ? 0
		: ui->depth16Btn->isChecked() ? 1
		: 2;
	settings.window.stencil = ui->stencilCheck->isChecked();
	settings.window.isFullScreen = ui->fullScreenCheck->isChecked();
	settings.window.windowBorder = ui->borderCheck->isChecked();
	settings.window.allowResizing = ui->resizeCheck->isChecked();
	settings.defaultSyntax = ui->languageCombo->currentIndex();
	settings.isSandboxMode = ui->safeModeCheck->isChecked();
	settings.text.isAutocompleteEnabled = ui->autoCompleteCheck->isChecked();
	settings.text.isFunctionSignatureEnabled = ui->functionSignatureCheck->isChecked();

	return true;
}

bool OptionsDialog::Validate()
{
	return true;
}

void OptionsDialog::UpdateUI()
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

void OptionsDialog::on_fullScreenCheck_clicked()
{
    UpdateUI();
}

void OptionsDialog::on_borderCheck_clicked()
{
    UpdateUI();
}
