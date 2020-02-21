#include "style.h"
#include <QStyleFactory>

Style::Style(bool light) : light(light)
{
    QStyle * wStyle = QStyleFactory::create("Fusion");
    if (wStyle != nullptr) {
        setBaseStyle(wStyle);
    }
}

QPalette Style::standardPalette() const
{
    if (light) return lightPalette();
    else return darkPalette();
}

QPalette Style::lightPalette() const
{
    QPalette palette;

    palette.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(QRgb(0xff808080)));
    palette.setBrush(QPalette::Disabled, QPalette::Button, QColor(QRgb(0xffdddfe4)));
    palette.setBrush(QPalette::Disabled, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Disabled, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Disabled, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(QRgb(0xff808080)));
    palette.setBrush(QPalette::Disabled, QPalette::Base, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, palette.color(QPalette::Disabled, QPalette::Base).darker(110));
    palette.setBrush(QPalette::Disabled, QPalette::Window, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Disabled, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(QRgb(0xff567594)));
    palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Disabled, QPalette::Link, QColor(QRgb(0xff0000ee)));
    palette.setBrush(QPalette::Disabled, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    palette.setBrush(QPalette::Active, QPalette::WindowText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Button, QColor(QRgb(0xffdddfe4)));
    palette.setBrush(QPalette::Active, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Active, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Active, QPalette::Text, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::ButtonText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::AlternateBase, palette.color(QPalette::Active, QPalette::Base).darker(110));
    palette.setBrush(QPalette::Active, QPalette::Window, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Active, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Active, QPalette::Highlight, QColor(QRgb(0xff678db2)));
    palette.setBrush(QPalette::Active, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Active, QPalette::Link, QColor(QRgb(0xff0000ee)));
    palette.setBrush(QPalette::Active, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Button, QColor(QRgb(0xffdddfe4)));
    palette.setBrush(QPalette::Inactive, QPalette::Light, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Midlight, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Dark, QColor(QRgb(0xff555555)));
    palette.setBrush(QPalette::Inactive, QPalette::Mid, QColor(QRgb(0xffc7c7c7)));
    palette.setBrush(QPalette::Inactive, QPalette::Text, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::BrightText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::ButtonText, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Base, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, palette.color(QPalette::Inactive, QPalette::Base).darker(110));
    palette.setBrush(QPalette::Inactive, QPalette::Window, QColor(QRgb(0xffefefef)));
    palette.setBrush(QPalette::Inactive, QPalette::Shadow, QColor(QRgb(0xff000000)));
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, QColor(QRgb(0xff678db2)));
    palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, QColor(QRgb(0xffffffff)));
    palette.setBrush(QPalette::Inactive, QPalette::Link, QColor(QRgb(0xff0000ee)));
    palette.setBrush(QPalette::Inactive, QPalette::LinkVisited, QColor(QRgb(0xff52188b)));
    return palette;
}

QPalette Style::darkPalette() const
{
    QPalette palette;

    QColor base_color = QColor("#393a3d");
    QColor text_color = QColor("#f1f1f1");
    QColor bg_color = QColor("#393a3d");
    QColor fg_color = QColor("#eeeeec");
    QColor selected_bg_color = QColor("#3584e4");
    QColor selected_fg_color = QColor("white");
    QColor shadow = QColor(100, 100, 100, 200);
    QColor backdrop_fg_color = QColor("#dddddd");
    QColor backdrop_base_color = QColor("#393a3d");
    QColor backdrop_selected_fg_color = QColor("#ffffff");
    QColor button_base_color = bg_color.darker(110);
    QColor link_color = selected_bg_color.darker(160);
    QColor link_visited_color = selected_bg_color.darker(110);
    QColor insensitive_fg_color = QColor("#999999");
    QColor insensitive_bg_color = QColor("#393a3d");

    palette.setBrush(QPalette::Disabled, QPalette::WindowText, insensitive_fg_color);
    palette.setBrush(QPalette::Disabled, QPalette::Button, insensitive_bg_color);
    palette.setBrush(QPalette::Disabled, QPalette::Light, insensitive_bg_color.lighter(160));
    palette.setBrush(QPalette::Disabled, QPalette::Midlight, insensitive_bg_color.lighter(110));
    palette.setBrush(QPalette::Disabled, QPalette::Dark, insensitive_bg_color.darker(160));
    palette.setBrush(QPalette::Disabled, QPalette::Mid, insensitive_bg_color.darker(110));
    palette.setBrush(QPalette::Disabled, QPalette::Text, insensitive_fg_color);
    palette.setBrush(QPalette::Disabled, QPalette::BrightText, text_color);
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, insensitive_fg_color);
    palette.setBrush(QPalette::Disabled, QPalette::Base, base_color);
    palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, base_color.darker(110));
    palette.setBrush(QPalette::Disabled, QPalette::Window, insensitive_bg_color);
    palette.setBrush(QPalette::Disabled, QPalette::Shadow, shadow);
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, selected_bg_color);
    palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, selected_fg_color);
    palette.setBrush(QPalette::Disabled, QPalette::Link, link_color);
    palette.setBrush(QPalette::Disabled, QPalette::LinkVisited, link_visited_color);
    palette.setBrush(QPalette::Active, QPalette::WindowText, fg_color);
    palette.setBrush(QPalette::Active, QPalette::Button, button_base_color);
    palette.setBrush(QPalette::Active, QPalette::Light, button_base_color.lighter(160));
    palette.setBrush(QPalette::Active, QPalette::Midlight, button_base_color.lighter(110));
    palette.setBrush(QPalette::Active, QPalette::Dark, button_base_color.darker(160));
    palette.setBrush(QPalette::Active, QPalette::Mid, button_base_color.darker(110));
    palette.setBrush(QPalette::Active, QPalette::Text, fg_color);
    palette.setBrush(QPalette::Active, QPalette::BrightText, text_color);
    palette.setBrush(QPalette::Active, QPalette::ButtonText, fg_color);
    palette.setBrush(QPalette::Active, QPalette::Base, base_color);
    palette.setBrush(QPalette::Active, QPalette::AlternateBase, base_color.darker(110));
    palette.setBrush(QPalette::Active, QPalette::Window, bg_color);
    palette.setBrush(QPalette::Active, QPalette::Shadow, shadow);
    palette.setBrush(QPalette::Active, QPalette::Highlight, selected_bg_color);
    palette.setBrush(QPalette::Active, QPalette::HighlightedText, selected_fg_color);
    palette.setBrush(QPalette::Active, QPalette::Link, link_color);
    palette.setBrush(QPalette::Active, QPalette::LinkVisited, link_visited_color);
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, backdrop_fg_color);
    palette.setBrush(QPalette::Inactive, QPalette::Button, button_base_color);
    palette.setBrush(QPalette::Inactive, QPalette::Light, insensitive_bg_color.lighter(160));
    palette.setBrush(QPalette::Inactive, QPalette::Midlight, insensitive_bg_color.lighter(110));
    palette.setBrush(QPalette::Inactive, QPalette::Dark, insensitive_bg_color.darker(160));
    palette.setBrush(QPalette::Inactive, QPalette::Mid, insensitive_bg_color.darker(110));
    palette.setBrush(QPalette::Inactive, QPalette::Text, backdrop_fg_color);
    palette.setBrush(QPalette::Inactive, QPalette::BrightText, text_color);
    palette.setBrush(QPalette::Inactive, QPalette::ButtonText, backdrop_fg_color);
    palette.setBrush(QPalette::Inactive, QPalette::Base, backdrop_base_color);
    palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, backdrop_base_color.darker(110));
    palette.setBrush(QPalette::Inactive, QPalette::Window, bg_color);
    palette.setBrush(QPalette::Inactive, QPalette::Shadow, shadow);
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, selected_bg_color);
    palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, backdrop_selected_fg_color);
    palette.setBrush(QPalette::Inactive, QPalette::Link, link_color);
    palette.setBrush(QPalette::Inactive, QPalette::LinkVisited, link_visited_color);
    palette.setColor(QPalette::All, QPalette::ToolTipBase, base_color);
    palette.setColor(QPalette::All, QPalette::ToolTipText, fg_color);
    return palette;
}

void Style::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
#ifndef QT_NO_DOCKWIDGET
    case CE_DockWidgetTitle:
        break;
#endif // QT_NO_DOCKWIDGET
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
}

int Style::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case QStyle::PM_ComboBoxFrameWidth:
        return 5;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}
