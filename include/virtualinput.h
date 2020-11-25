#ifndef VIRTUALINPUT_H
#define VIRTUALINPUT_H

#include <QObject>
#include <QDialog>
#include <QEvent>
#include <QTimer>

class VirtualInput : public QObject
{
    Q_OBJECT
public:
    static VirtualInput& instance();
    static void registerDialog(QDialog * dialog);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void _registerDialog(QDialog * dialog);
    void updateGeometry(QDialog * dialog, QWidget * widget);
    QDialog * findParentDialog(QObject * widget);
    void showWidget(QWidget * widget);
    void hideWidget(QWidget * widget);
private:
    VirtualInput();
    QTimer timer;
    QObject * activeObject;
    QObject * activeInput;
signals:

private slots:
    void updateState();
    void onDialogDestroy(QObject*);
};

#endif // VIRTUALINPUT_H
