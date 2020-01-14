#include "virtualmachineviewdialog.h"
#include "ui_virtualmachineviewdialog.h"

class DummySourceLineFormatter : public ISourceLineFormatter
{
public:
	std::string GetSourcePos(int line) override { return "L" + IntToString(line); }
};

DummySourceLineFormatter dummySourceLineFormatter;

VirtualMachineViewDialog::VirtualMachineViewDialog(TomBasicCompiler& _compiler, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VirtualMachineViewDialog),
	compiler(_compiler)
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);	

	// Create grid sources
	codeSource = std::make_shared<VMCodeGridSource>(compiler, &dummySourceLineFormatter);
	variableSource = std::make_shared<VMGlobalVariableGridSource>(compiler.VM());
	stackSource = std::make_shared<VMStackValueGridSource>(compiler.VM());	
}

VirtualMachineViewDialog::~VirtualMachineViewDialog()
{
    delete ui;
}

void VirtualMachineViewDialog::Execute(bool canStep)
{
	ui->stepButton->setEnabled(canStep);

	ScrollToIP();
	exec();
}

void VirtualMachineViewDialog::on_stepButton_clicked()
{
	compiler.VM().Continue(1);
	ScrollToIP();
	UpdateUI();
}

void VirtualMachineViewDialog::ScrollToIP()
{
	// Scroll to IP
	ui->codeScroll->setRange(0, codeSource->GetRowCount());
	int pos = compiler.VM().IP() - 5 - codeSource->GetFirstRowIndex();
	if (pos < 0) pos = 0;
	ui->codeScroll->setValue(pos);
}

void VirtualMachineViewDialog::UpdateGrid(QTableWidget* grid, QScrollBar* scrollBar, std::shared_ptr<IGridSource> source, int highlightRowIndex)
{
	// Set default row size
	grid->setRowCount(100);
	QHeaderView *verticalHeader = grid->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(17);

	// Calculate # of visible rows
	grid->setRowCount(100);
	int visibleRowCount = grid->rowAt(grid->height()) - grid->rowAt(0) - 2;
	//grid->setRowCount(100);
	//int height = grid->height();
	//int visibleRowCount = (height - 32) / 17;
	if (visibleRowCount < 2) return;

	// Compare to # of source rows to display
	int firstRowIndex = source->GetFirstRowIndex();
	int	sourceRowCount = source->GetRowCount();
	int rowCount;
	if (sourceRowCount <= visibleRowCount)
	{
		// No scrolling required
		scrollBar->setVisible(false);
		rowCount = sourceRowCount;
	}
	else
	{
		// Show scrollbar.
		scrollBar->setVisible(true);
		int pos = scrollBar->value();
		scrollBar->setRange(0, sourceRowCount - visibleRowCount);
		if (pos > scrollBar->maximum()) pos = scrollBar->maximum();
		scrollBar->setValue(pos);
		rowCount = visibleRowCount;
	}

	// Populate grid rows
	grid->setRowCount(rowCount);
	for (int r = 0; r < rowCount; r++)
	{
		// Left column is row index
		int rowIndex = firstRowIndex + scrollBar->value() + r;
		QString text = QString::number(rowIndex);
		if (rowIndex == highlightRowIndex) text += " >>>";
		grid->setItem(r, 0, new QTableWidgetItem(text));

		// Populate remaining columns from source
		for (int c = 1; c < grid->columnCount(); c++)
		{
			std::string val = source->GetValue(scrollBar->value() + r, c - 1);
			grid->setItem(r, c, new QTableWidgetItem(QString::fromStdString(val)));
		}
	}
}

void VirtualMachineViewDialog::UpdateCodeGrid()
{
	UpdateGrid(ui->codeGrid, ui->codeScroll, codeSource, compiler.VM().IP());
}

void VirtualMachineViewDialog::UpdateRegisters()
{
	// Register 1
	ui->reg1Int->setText(QString::number(compiler.VM().Reg().IntVal()));
	ui->reg1Float->setText(QString::number(compiler.VM().Reg().RealVal()));
	ui->reg1String->setText(QString::fromStdString(compiler.VM().RegString()));

	// Register 2
	ui->reg2Int->setText(QString::number(compiler.VM().Reg2().IntVal()));
	ui->reg2Float->setText(QString::number(compiler.VM().Reg2().RealVal()));
	ui->reg2String->setText(QString::fromStdString(compiler.VM().Reg2String()));
}

void VirtualMachineViewDialog::UpdateVariableGrid()
{
	UpdateGrid(ui->variableGrid, ui->variableScroll, variableSource);
}

void VirtualMachineViewDialog::UpdateHeapGrid()
{
	UpdateGrid(ui->heapGrid, ui->heapScroll, std::make_shared<VMHeapDataGridSource>(compiler.VM()));
}

void VirtualMachineViewDialog::UpdateStackGrid()
{
	UpdateGrid(ui->stackGrid, ui->stackScroll, stackSource);
}

void VirtualMachineViewDialog::UpdateCallStackGrid()
{
	// Add call stack entries
	auto& callStack = compiler.VM().UserCallStack();
	auto& functions = compiler.VM().UserFunctions();
	auto& prototypes = compiler.VM().UserFunctionPrototypes();
	ui->callGrid->setRowCount(callStack.size() + 1);
	for (int i = 0; i < callStack.size(); i++) {
		vmUserFuncStackFrame& frame = callStack[callStack.size() - i - 1];

		QString text;

		// Special case. Gosubs have -1 index
		if (frame.userFuncIndex == -1)
			text = "GOSUB";
		else {

			vmUserFunc& func = functions[frame.userFuncIndex];
			vmUserFuncPrototype& prototype = prototypes[func.prototypeIndex];
			text = prototype.hasReturnVal ? "function " : "sub ";
			text += QString::fromStdString(compiler.GetUserFunctionName(frame.userFuncIndex));

			// Indicate the current active frame with an asterisk
			if (i == compiler.VM().CurrentUserFrame())
				text += "*";
		}
		ui->callGrid->setItem(i, 0, new QTableWidgetItem(text));
	}

	ui->callGrid->setItem(callStack.size(), 0, new QTableWidgetItem("[GLOBAL]"));
}

void VirtualMachineViewDialog::UpdateUI()
{
	// Update registers
	UpdateRegisters();

	// Refresh grids
	UpdateCodeGrid();
	UpdateVariableGrid();
	UpdateHeapGrid();
	UpdateStackGrid();
	UpdateCallStackGrid();
}

void VirtualMachineViewDialog::on_codeScroll_valueChanged(int value)
{
	UpdateCodeGrid();
}

void VirtualMachineViewDialog::on_variableScroll_valueChanged(int value)
{
	UpdateVariableGrid();
}

void VirtualMachineViewDialog::on_heapScroll_valueChanged(int value)
{
	UpdateHeapGrid();
}

void VirtualMachineViewDialog::on_stackScroll_valueChanged(int value)
{
	UpdateStackGrid();
}

void VirtualMachineViewDialog::resizeEvent(QResizeEvent* ev)
{
	QDialog::resizeEvent(ev);
	UpdateUI();
}

void VirtualMachineViewDialog::showEvent(QShowEvent* ev)
{
	QDialog::showEvent(ev);
	UpdateUI();
}
