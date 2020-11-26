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
    static void registerDialog(QDialog * dialog, bool withPlainTextEdits = false);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void _registerDialog(QDialog * dialog, bool withPlainTextEdits = false);
    void updateGeometry(QDialog * dialog, QWidget * widget);
    QDialog * findParentDialog(QObject * widget);
    void showWidget(QWidget * widget);
    void hideWidget(QWidget * widget);
    void filterDialogEvent(QObject *watched, QEvent * event);
    void filterLineEditEvent(QObject *watched, QEvent * event);
    void filterPlainTextEditEvent(QObject *watched, QEvent * event);
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
