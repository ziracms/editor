/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef SEARCH_H
#define SEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include "editor.h"

class Search : public QWidget
{
    Q_OBJECT
public:
    explicit Search(Editor * codeEditor);
    QSize sizeHint() const override;
    void setFindEditBg(QColor bgColor);
    void setReplaceEditBg(QColor bgColor);
    void setFindEditProp(const char * prop, QString val);
    void setReplaceEditProp(const char * prop, QString val);
    void setFindEditFocus();
    void setFindEditText(QString str);
    bool isFocused();
protected:
    void paintEvent(QPaintEvent *event) override;
    QLineEdit * findEdit;
    QPushButton * findButton;
    QPushButton * findPrevButton;
    QCheckBox * findCaseSensitive;
    QCheckBox * findWholeWords;
    QCheckBox * findRegexp;
    QLineEdit * replaceEdit;
    QPushButton * replaceButton;
    QPushButton * replaceAllButton;
    QPushButton * closeButton;
    QHBoxLayout * hLayoutFind;
    QHBoxLayout * hLayoutReplace;
    QVBoxLayout * vLayout;
private:
    Editor * editor;
    bool CaSe;
    bool Word;
    bool RegE;
    bool Prev;
signals:

private slots:
    void findPressed();
    void findPrevPressed();
    void replacePressed();
    void replaceAllPressed();
    void closePressed();
    void findCaseSensitiveChanged(int state);
    void findWholeWordsChanged(int state);
    void findRegexpChanged(int state);
    void findEnterPressed();
    void replaceEnterPressed();
    void findTextChanged(QString);
};

#endif // SEARCH_H
