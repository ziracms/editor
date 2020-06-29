#include "filedialog.h"
#include <QStandardPaths>
#include <QToolButton>

FileDialog::FileDialog(QWidget *parent) : QFileDialog(parent)
{
    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    setWindowState(windowState() | Qt::WindowMaximized);
    setOption(QFileDialog::DontUseNativeDialog);
    #endif
    setViewMode(QFileDialog::Detail);
    iconProvider = new FileIconProvider();
    setIconProvider(iconProvider);
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    //urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first());
    urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first());
    #if defined(Q_OS_ANDROID)
    urls << QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first());
    #endif
    setSidebarUrls(urls);

    QToolButton * newFolderButton = findChild<QToolButton *>("newFolderButton");
    if (newFolderButton != nullptr) newFolderButton->hide();
}

FileDialog::~FileDialog()
{
    delete iconProvider;
}
