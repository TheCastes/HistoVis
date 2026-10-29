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

    void addSeries(const std::array<float, 256> &values, const QColor &color);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Series {
        std::array<float, 256> values;
        QColor color;
    };

    QString title;
    Style style;
    std::vector<Series> series;
};