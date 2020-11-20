#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QColorDialog>

class ColorDialog : public QColorDialog
{
public:
    ColorDialog(QWidget *parent = nullptr);
    ~ColorDialog();
};

#endif // COLORDIALOG_H
