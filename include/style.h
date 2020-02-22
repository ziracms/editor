#ifndef STYLE_H
#define STYLE_H

#include <QProxyStyle>

class Style : public QProxyStyle
{
public:
    Style(bool light);
    QPalette standardPalette() const override;
protected:
    QPalette lightPalette() const;
    QPalette darkPalette() const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
private:
    bool light;
};

#endif // STYLE_H
