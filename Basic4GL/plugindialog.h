#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include "PluginDLL.h"
#include <QDialog>
#include <QListWidgetItem>
#include <string>
#include <vector>

namespace Ui {
class PluginDialog;
}

class PluginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginDialog(PluginDLLManager& manager, QWidget *parent = 0);
    ~PluginDialog();
	void Load();
	bool WasChangeMade() { return wasChangeMade; }
	void Invalidate();

	int exec() override;

private slots:
    void on_pluginList_itemChanged(QListWidgetItem *item);

	void on_pluginList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::PluginDialog *ui;

	PluginDLLManager& manager;

	// State
	std::vector<std::string> filenames;
	std::vector<bool> isFileLoaded;
	bool wasChangeMade;
	bool isLoaded;

	void ShowPluginDetails(QListWidgetItem *item);
	std::string DescribeValType(vmValType valType);
	std::string DescribeFunctionSpec(compFuncSpec const& spec, PluginDLL* dll, int index);
	std::string DescribeConstant(std::string name, const compConstant& constant);
};

#endif // PLUGINDIALOG_H
