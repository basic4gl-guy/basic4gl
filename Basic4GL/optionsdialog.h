#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "settings.h"
#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

	bool Execute(Basic4GLSettings& settings);

private slots:
    void on_fullScreenCheck_clicked();

    void on_borderCheck_clicked();

private:
    Ui::OptionsDialog *ui;

	bool Validate();
	void UpdateUI();
};

#endif // OPTIONSDIALOG_H
