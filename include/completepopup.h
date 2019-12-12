/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef COMPLETEPOPUP_H
#define COMPLETEPOPUP_H

#include <QListWidget>

class CompletePopup : public QListWidget
{
    Q_OBJECT
public:
    explicit CompletePopup(QWidget * parent);
    void clearItems();
    void addItem(QString str, QString data, QString delimiter = "");
    void showPopup(int cursLeft, int cursTop, int viewLeft, int viewTop, int viewWidth, int viewHeight, int blockHeight);
    void hidePopup();
    void selectNextItem();
    void selectPreviousItem();
    void chooseCurrentItem();
    void setTextStartPos(int pos);
    int getTextStartPos();
    int limit();
private:
    int textStartPos;
private slots:
    void onItemClicked(QListWidgetItem * item);
signals:
    void itemDataClicked(QString text, QString data);
};

#endif // COMPLETEPOPUP_H
