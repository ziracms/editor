#include "fileiconprovider.h"
#include "project.h"

FileIconProvider::FileIconProvider()
{

}

QIcon FileIconProvider::icon(IconType type) const
{
    if (type == QFileIconProvider::Folder) {
        return QIcon(":/icons/folder.png");
    } else if (type == QFileIconProvider::Computer) {
        return QIcon(":/icons/levelup.png");
    }
    return QFileIconProvider::icon(type);
}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isDir()) {
        if (Project::exists(info.absoluteFilePath())) {
            return QIcon(":/icons/zira.png");
        }
        return QIcon(":/icons/folder.png");
    }
    return QFileIconProvider::icon(info);
}
