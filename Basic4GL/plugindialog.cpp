#include "plugindialog.h"
#include "ui_plugindialog.h"
#include "basic4glMisc.h"

PluginDialog::PluginDialog(PluginDLLManager& manager, QWidget *parent) :
    QDialog(parent),
	manager(manager),
	isLoaded(false),
	wasChangeMade(false),
    ui(new Ui::PluginDialog)
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

PluginDialog::~PluginDialog()
{
    delete ui;
}

void PluginDialog::Load()
{
	// Get currently loaded DLLs
	auto plugins = manager.DLLFiles();
	ui->pluginList->clear();
	filenames.clear();
	isFileLoaded.clear();
	for (auto& plugin : plugins)
	{
		// Add list item
		std::string text = plugin.filename + " - " + plugin.description + " v" + plugin.version.VersionString();
		QListWidgetItem* item = new QListWidgetItem(text.c_str());
		item->setCheckState(plugin.loaded ? Qt::Checked : Qt::Unchecked);
		ui->pluginList->addItem(item);

		// Track filename and loaded state
		filenames.push_back(plugin.filename);
		isFileLoaded.push_back(plugin.loaded);;
	}

	isLoaded = true;
}

void PluginDialog::Invalidate()
{
	// Force reload on next exec
	isLoaded = false;
}

int PluginDialog::exec()
{
	if (!isLoaded) 
		Load();

	// Track whether a change is made during this "exec"
	wasChangeMade = false;
	return QDialog::exec();
}

void PluginDialog::on_pluginList_itemChanged(QListWidgetItem *item)
{
	// Find clicked item
	int i = GetListItemIndex(ui->pluginList, item);
	if (i < 0 || i >= filenames.size()) return;

	// Get checked state. Check if changed.
	bool isChecked = item->checkState() == Qt::Checked;
	if (isChecked == isFileLoaded[i])  return;

	// Load/unload plugin 
	bool succeeded = isChecked
		? manager.LoadDLL(filenames[i])
		: manager.UnloadDLL(filenames[i]);

	if (succeeded)
	{
		isFileLoaded[i] = isChecked;
		wasChangeMade = true;

		// Update details view
		if (ui->pluginList->currentItem() != item)
		{
			ui->pluginList->setCurrentItem(item);
		}
		else {
			ShowPluginDetails(ui->pluginList->currentItem());
		}
	}
	else
	{
		// Load/unload failed.
		// Revert checked state.
		item->setCheckState(isFileLoaded[i] ? Qt::Checked : Qt::Unchecked);

		// Show error message
		ShowMessage(manager.Error());
	}
}

void PluginDialog::on_pluginList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	// Show plugin details for current item
	ShowPluginDetails(current);
}

void PluginDialog::ShowPluginDetails(QListWidgetItem* item)
{
	// Display details for corresponding plugins
	ui->functionsList->clear();
	ui->constantsList->clear();

	// Find clicked item
	int i = GetListItemIndex(ui->pluginList, item);
	if (i < 0 || i >= filenames.size()) return;

	// Lookup DLL
	auto* dll = manager.Find(filenames[i]);

	if (dll != nullptr)
	{
		// Populate function list
		int index = 0;
		for (auto& spec : dll->FunctionSpecs())
		{
			std::string text = DescribeFunctionSpec(spec, dll, index);
			ui->functionsList->addItem(text.c_str());
			index++;
		}

		// Populate constant list
		for (auto& constant : dll->Constants())
		{
			std::string text = DescribeConstant(constant.first, constant.second);
			ui->constantsList->addItem(text.c_str());
		}

		// Populate structures list
		{
			std::string text = manager.StructureManager().DescribeOwnedStructures(dll);
			ui->structuresTextEdit->setPlainText(text.c_str());
		}
	}
	else
	{
		ui->functionsList->addItem("Plugin is not loaded");
		ui->constantsList->addItem("Plugin is not loaded");
		ui->structuresTextEdit->setPlainText("Plugin is not loaded");
	}
}

std::string PluginDialog::DescribeValType(vmValType valType)
{
	std::string result = "";
	for (int i = 0; i < valType.VirtualPointerLevel(); i++)
		result += "&";

	if (valType.m_basicType >= 0) {
		// Structure type
		PluginStructure *structure = manager.StructureManager().GetStructure(valType.m_basicType);
		if (structure != nullptr)
			result += structure->Name();
		else
			result += "???";
	}
	else
		result += vmBasicValTypeName(valType.m_basicType);

	for (int i = 0; i < valType.m_arrayLevel; i++)
		result += "()";

	if (valType.m_basicType < 0 && valType.m_arrayLevel == 0 && valType.m_byRef)
		result += "{by reference}";

	return result;
}

std::string PluginDialog::DescribeFunctionSpec(compFuncSpec const& spec, PluginDLL* dll, int index)
{
	std::string text = "";

	// Return type
	if (spec.m_isFunction)
	{
		auto temp = spec.m_returnType;
		text += DescribeValType(temp) + " ";
	}

	// Function name
	text += dll->FunctionName(index);

	// Parameter types
	text += spec.m_brackets ? "(" : " ";
	bool needComma = false;
	for (auto& valType : spec.m_paramTypes.Params())
	{
		if (needComma)
			text += ", ";
		needComma = true;
		text += DescribeValType(valType);
	}
	text += spec.m_brackets ? ")" : "";

	return text;
}

std::string PluginDialog::DescribeConstant(std::string name, const compConstant& constant)
{
	std::string text = "";
	text += vmBasicValTypeName(constant.m_valType) + " ";
	text += name + " = ";
	switch (constant.m_valType)
	{
	case VTP_INT:
		text += IntToString(constant.m_intVal);
		break;
	case VTP_REAL:
		text += RealToString(constant.m_realVal);
		break;
	case VTP_STRING:
		text += "\"" + constant.m_stringVal.substr(0, 999) + "\"";
		break;
	default:
		text += "???";
	}

	return text;
}
