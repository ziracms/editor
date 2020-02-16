#ifndef TABSLIST_H
#define TABSLIST_H

#include <QListWidget>

class TabsList : public QListWidget
{
    Q_OBJECT
public:
    explicit TabsList(QWidget * parent);
    void addItem(QString str, int index);
protected:
    void keyPressEvent(QKeyEvent *e) override;
private slots:
    void onItemClicked(QListWidgetItem * item);
signals:
    void itemClicked(int i);
};

#endif // TABSLIST_H
