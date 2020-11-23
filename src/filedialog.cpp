#include "filedialog.h"
#include <QStandardPaths>
#include <QToolButton>
#include <QListView>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include "scroller.h"

FileDialog::FileDialog(QWidget *parent) : QFileDialog(parent)
{
    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    setWindowState(windowState() | Qt::WindowMaximized);
    #endif
    setOption(QFileDialog::DontUseNativeDialog);
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

    QTreeView * treeView = findChild<QTreeView *>("treeView");
    if (treeView != nullptr) {
        treeView->setDragEnabled(false);
        #if defined(Q_OS_ANDROID)
        // scrolling by gesture
        Scroller::enableGestures(treeView, false);
        #endif
    }
    QListView * listView = findChild<QListView *>("listView");
    if (listView != nullptr) {
        listView->setDragEnabled(false);
    }

    QDialogButtonBox * buttonBox = findChild<QDialogButtonBox *>("buttonBox");
    if (buttonBox != nullptr && layout() != nullptr) {
        QGridLayout * gridLayout = qobject_cast<QGridLayout *>(layout());
        if (gridLayout != nullptr) {
            buttonBox->setOrientation(Qt::Horizontal);
            QHBoxLayout * childLayout = new QHBoxLayout();
            childLayout->addWidget(buttonBox);
            gridLayout->addLayout(childLayout, gridLayout->rowCount(), 1);
        }
    }
}

FileDialog::~FileDialog()
{
    delete iconProvider;
}
