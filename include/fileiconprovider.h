#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include <QFileIconProvider>

class FileIconProvider : public QFileIconProvider
{
public:
    FileIconProvider();
    virtual QIcon icon(IconType type) const override;
    virtual QIcon icon(const QFileInfo &info) const override;
};

#endif // FILEICONPROVIDER_H
