#ifndef VIRTUALMACHINEVIEWDIALOG_H
#define VIRTUALMACHINEVIEWDIALOG_H

#include <QDialog>
#include <TomVM.h>
#include <TomComp.h>
#include "../VMView/VMViewGridSources.h"
#include <qtablewidget.h>
#include <memory>

namespace Ui {
class VirtualMachineViewDialog;
}

class VirtualMachineViewDialog : public QDialog
{
    Q_OBJECT


public:
	explicit VirtualMachineViewDialog(TomBasicCompiler& _compiler, QWidget *parent = 0);
    ~VirtualMachineViewDialog();
	void Execute(bool canStep);

private slots:
    void on_stepButton_clicked();

    void on_codeScroll_valueChanged(int value);

    void on_variableScroll_valueChanged(int value);

    void on_heapScroll_valueChanged(int value);

    void on_stackScroll_valueChanged(int value);

protected:
	void resizeEvent(QResizeEvent*ev) override;
	void showEvent(QShowEvent*) override;

private:
    Ui::VirtualMachineViewDialog *ui;
	std::shared_ptr<IGridSource> codeSource;
	std::shared_ptr<IGridSource> variableSource;
	std::shared_ptr<IGridSource> stackSource;
	TomBasicCompiler& compiler;

	void ScrollToIP();
	void UpdateGrid(QTableWidget* grid, QScrollBar* scrollBar, std::shared_ptr<IGridSource> source, int highlightRowIndex = -1);
	void UpdateCodeGrid();
	void UpdateRegisters();
	void UpdateVariableGrid();
	void UpdateHeapGrid();
	void UpdateStackGrid();
	void UpdateCallStackGrid();
	void UpdateUI();
};

#endif // VIRTUALMACHINEVIEWDIALOG_H
