#include <string>
#include <QListWidget>

extern void ShowMessage(std::string text);

extern std::string CppString(QString str);

extern int GetListItemIndex(QListWidget* list, QListWidgetItem* item);

extern void ShowFileInExplorer(QWidget *parent, const QString pathIn);