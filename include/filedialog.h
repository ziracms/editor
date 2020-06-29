#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QFileDialog>
#include "fileiconprovider.h"

class FileDialog : public QFileDialog
{
public:
    FileDialog(QWidget *parent = nullptr);
    ~FileDialog();
private:
    FileIconProvider * iconProvider;
};

#endif // FILEDIALOG_H
