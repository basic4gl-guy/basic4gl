#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

	bool ShowFindOptions();
	bool ShowReplaceOptions();

	QString GetFindText() const;
	QString GetReplaceText() const;
	bool IsReplaceAll() const { return isReplaceAll; }
	bool IsMatchCase() const;
	bool IsWholeWordsOnly() const;

private slots:
	void ReplaceClicked();
	void ReplaceAllClicked();

private:
    Ui::FindDialog *ui;

	bool isReplaceAll;
};

#endif // FINDDIALOG_H
