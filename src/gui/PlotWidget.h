#pragma once

#include <array>
#include <vector>

#include <QColor>
#include <QString>
#include <QWidget>

class PlotWidget : public QWidget {
    Q_OBJECT

public:
    enum class Style { Bars, Line };

    PlotWidget(const QString &title, Style style, QWidget *parent = nullptr);

    void addSeries(const std::array<float, 256> &values, const QColor &color, bool original = false);
    void clear();
    void setLogScale(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Series {
        std::array<float, 256> values;
        QColor color;
        bool original;
    };

    double fraction(float value, float maxValue) const;

    QString title;
    Style style;
    std::vector<Series> series;
    bool logScale = false;
};