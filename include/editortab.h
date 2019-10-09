/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef EDITORTAB_H
#define EDITORTAB_H

#include <QWidget>
#include "editor.h"

class EditorTab : public QWidget
{
    Q_OBJECT
public:
    explicit EditorTab(QWidget *parent = nullptr);
    ~EditorTab();
    void setEditor(Editor * editor);
    Editor * getEditor();
signals:

public slots:
};

#endif // EDITORTAB_H
