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

    QColor base_color = QColor("#e9e9e9");
    QColor text_color = QColor("#111111");
    QColor bg_color = QColor("#e9e9e9");
    QColor fg_color = QColor("#111111");
    QColor selected_bg_color = QColor("#3584e4");
    QColor selected_fg_color = QColor("white");
    QColor shadow = QColor(10, 10, 10, 200);
    QColor backdrop_fg_color = QColor("#333333");
    QColor backdrop_base_color = QColor("#e9e9e9");
    QColor backdrop_selected_fg_color = QColor("#ffffff");
    QColor button_base_color = bg_color.lighter(110);
    QColor link_color = selected_bg_color.darker(160);
    QColor link_visited_color = selected_bg_color.darker(110);
    QColor insensitive_fg_color = QColor("#555555");
    QColor insensitive_bg_color = QColor("#e9e9e9");

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
    case CE_ToolBar:
        break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
}

int Style::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize Style::sizeFromContents(ContentsType type, const QStyleOption *option,
                                        const QSize &size, const QWidget *widget) const
{
    QSize newSize = QProxyStyle::sizeFromContents(type, option, size, widget);

    switch (type) {
#ifndef QT_NO_SPINBOX
    case CT_SpinBox:
        newSize.rheight() += 1; // bigger box
        break;
#endif
#ifndef QT_NO_COMBOBOX
    case CT_ComboBox:
        newSize = sizeFromContents(CT_PushButton, option, size, widget);
        newSize.rheight() += 6; // bigger box
        break;
#endif
    case CT_LineEdit:
        newSize.rheight() += 6; // bigger box
        break;
    case CT_TabBarTab:
        newSize.rwidth() += 6; // bigger box
        newSize.rheight() += 6; // bigger box
        break;
    case CT_PushButton:
        newSize.rheight() += 6; // bigger box
        break;
    case CT_ItemViewItem:
        // bad idea
        //newSize.rheight() += 10; // bigger box
        break;
    default:
        break;
    }

    return newSize;
}
