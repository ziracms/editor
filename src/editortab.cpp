/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "editortab.h"
#include <QVBoxLayout>
#include "helper.h"

EditorTab::EditorTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout * layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

EditorTab::~EditorTab()
{

}

void EditorTab::setEditor(Editor * editor)
{
    if (layout() == 0) return;
    layout()->addWidget(editor);
}

Editor * EditorTab::getEditor()
{
    if (layout() == 0 || layout()->count() != 1) return nullptr;
    QLayoutItem * textEditItem = layout()->itemAt(0);
    if (textEditItem == 0) return nullptr;
    QWidget * textEdit = textEditItem->widget();
    if (textEdit == nullptr) return nullptr;
    Editor * textEditor = static_cast<Editor *>(textEdit);
    return textEditor;
}
