/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "servers.h"
#include <QStringList>

const QString SERVERS_START_CMD = "start";
const QString SERVERS_STOP_CMD = "stop";
const QString SERVERS_STATUS_CMD = "status";
const QString SERVERS_APACHE2_SERVICE_CMD = " echo %1 | sudo -S service apache2 %2";
const QString SERVERS_MARIADB_SERVICE_CMD = " echo %1 | sudo -S service mariadb %2";

const QString TPL_LINE = "<div>%1</div>";
const QString TPL_CMD_LINE = "<div>%1</div><div>&nbsp;</div>";
const QString TPL_COLOR_TEXT = "<span style=\"color:%2\">%1</span>";

Servers::Servers()
{

}

QString Servers::generateApacheServiceCommand(QString command, QString pwd)
{
    return SERVERS_APACHE2_SERVICE_CMD.arg(pwd).arg(command);
}

QString Servers::generateMariaDBServiceCommand(QString command, QString pwd)
{
    return SERVERS_MARIADB_SERVICE_CMD.arg(pwd).arg(command);
}

QString Servers::highlightServersCommand(QString & text)
{
    QString infoColor = QString::fromStdString(Settings::get("git_output_info_color"));
    return TPL_CMD_LINE.arg(TPL_COLOR_TEXT.arg(text).arg(infoColor));
}

QString Servers::highlightServersCommandOutput(QString & output)
{
    QString errorColor = QString::fromStdString(Settings::get("git_output_error_color"));
    QString msgColor = QString::fromStdString(Settings::get("git_output_message_color"));
    QString infoColor = QString::fromStdString(Settings::get("git_output_info_color"));
    QString result = "";
    QStringList outputList = output.split("\n");
    QString highlightedOutput = "";
    for (int i=0; i<outputList.size(); i++) {
        QString lineText = outputList.at(i);
        lineText.replace("\t", "    ");
        lineText.replace("  ", "&nbsp;&nbsp;");
        lineText.replace("<", "&lt;");
        lineText.replace(">", "&gt;");
        if (lineText.isEmpty()) lineText = "&nbsp;";
        QString highlightedText = "";
        if (lineText.indexOf("Sorry, try again.") >= 0) {
            lineText = TPL_COLOR_TEXT.arg(lineText, errorColor);
        } else if (lineText.indexOf("Active: inactive") >= 0) {
            highlightedText = TPL_COLOR_TEXT.arg(lineText, errorColor);
        } else if (lineText.indexOf("Active: active") >= 0) {
            highlightedText = TPL_COLOR_TEXT.arg(lineText, msgColor);
        } else if (lineText.indexOf("*") == 0) {
            highlightedText = TPL_COLOR_TEXT.arg(lineText, infoColor);
        }
        result += TPL_LINE.arg(lineText);
        if (highlightedText.size() > 0) highlightedOutput += TPL_LINE.arg(highlightedText);
    }
    if (highlightedOutput.size() > 0) {
        result = highlightedOutput + TPL_LINE.arg("&nbsp;") + TPL_LINE.arg("...") + TPL_LINE.arg("&nbsp;") + result;
    }
    return result;
}
