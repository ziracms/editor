#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include "ui_shortcutsdialog.h"
#include <QDialog>
#include <unordered_map>

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);
    ~ShortcutsDialog();
    std::unordered_map<std::string, std::string> getData();
protected:
    std::vector<std::pair<std::string, std::string>> getShortcutNames();
private:
    Ui::ShortcutsDialog *ui;
};

#endif // SHORTCUTSDIALOG_H
