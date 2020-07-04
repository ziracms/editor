#include "fileiconprovider.h"
#include "project.h"
#include "icon.h"

FileIconProvider::FileIconProvider()
{

}

QIcon FileIconProvider::icon(IconType type) const
{
    if (type == QFileIconProvider::Folder) {
        return Icon::get("folder", QIcon(":/icons/folder.png"));
    } else if (type == QFileIconProvider::Computer) {
        return Icon::get("folder", QIcon(":/icons/folder.png"));
    }
    return QFileIconProvider::icon(type);
}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isDir()) {
        if (Project::exists(info.absoluteFilePath())) {
            return Icon::get("actionHelpZiraCMS", QIcon(":/icons/zira.png"));
        }
        return Icon::get("folder", QIcon(":/icons/folder.png"));
    }
    return QFileIconProvider::icon(info);
}
