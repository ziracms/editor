#ifndef DOCKTITLEBAR_H
#define DOCKTITLEBAR_H

#include <QWidget>

class DockTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit DockTitleBar(QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:

public slots:
};

#endif // DOCKTITLEBAR_H
