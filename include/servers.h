/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#ifndef SERVERS_H
#define SERVERS_H

#include <QString>
#include "settings.h"

extern const QString SERVERS_START_CMD;
extern const QString SERVERS_STOP_CMD;
extern const QString SERVERS_STATUS_CMD;

class Servers
{
public:
    Servers();
    static QString generateApacheServiceCommand(QString command, QString pwd);
    static QString generateMariaDBServiceCommand(QString command, QString pwd);
    static QString highlightServersCommand(QString & text, Settings * settings);
    static QString highlightServersCommandOutput(QString & output, Settings * settings);
};

#endif // SERVERS_H
