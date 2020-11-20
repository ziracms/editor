#include "colordialog.h"
#include <QVBoxLayout>

ColorDialog::ColorDialog(QWidget *parent) : QColorDialog(parent)
{
    setOption(QColorDialog::DontUseNativeDialog);
    setWindowTitle(tr("Pick a color"));

    #if defined(Q_OS_ANDROID)
    setWindowState(windowState() | Qt::WindowMaximized);
    QVBoxLayout * vLayout = findChild<QVBoxLayout *>();
    if (vLayout != nullptr) {
        vLayout->insertStretch(vLayout->count()-1);
    }
    #endif
}

ColorDialog::~ColorDialog()
{

}
