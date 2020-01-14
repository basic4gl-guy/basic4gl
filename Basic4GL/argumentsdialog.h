#ifndef ARGUMENTSDIALOG_H
#define ARGUMENTSDIALOG_H

#include <QDialog>

namespace Ui {
class ArgumentsDialog;
}

class ArgumentsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArgumentsDialog(QWidget *parent = 0);
    ~ArgumentsDialog();

	int exec() override;

private:
    Ui::ArgumentsDialog *ui;

	QString GetArguments();
	void SetArguments(QString str);
};

#endif // ARGUMENTSDIALOG_H
