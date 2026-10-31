#include "PlotWidget.h"

#include <algorithm>
#include <cmath>

#include <QLineF>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QRect>
#include <QRectF>

PlotWidget::PlotWidget(const QString &title, Style style, QWidget *parent)
    : QWidget(parent), title(title), style(style) {
    setMinimumHeight(150);
}

void PlotWidget::addSeries(const std::array<float, 256> &values, const QColor &color, bool original) {
    series.push_back(Series{values, color, original});
    update();
}

void PlotWidget::clear() {
    series.clear();
    update();
}

void PlotWidget::setLogScale(bool enabled) {
    logScale = enabled;
    update();
}

double PlotWidget::fraction(float value, float maxValue) const {
    if (logScale) {
        return std::log10(1.0 + value) / std::log10(1.0 + maxValue);
    }
    return value / maxValue;
}

void PlotWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QColor textColor = palette().color(QPalette::WindowText);
    QColor gridColor = textColor;
    gridColor.setAlpha(40);
    painter.fillRect(rect(), palette().color(QPalette::Base));

    const int left = 60;
    const int right = 25;
    const int top = 22;
    const int bottom = 22;
    QRectF plot(left, top, width() - left - right, height() - top - bottom);
    double binWidth = plot.width() / 256.0;

    painter.setPen(textColor);
    painter.drawText(QRectF(0, 0, width(), top), Qt::AlignCenter, title);

    for (int value : {0, 64, 128, 192, 255}) {
        double x = plot.left() + (value + 0.5) * binWidth;
        painter.drawLine(QLineF(x, plot.bottom(), x, plot.bottom() + 4));
        painter.drawText(QRectF(x - 20, plot.bottom() + 4, 40, bottom - 4), Qt::AlignCenter, QString::number(value));
    }

    float maxValue = 0;
    for (const Series &s : series) {
        maxValue = std::max(maxValue, *std::max_element(s.values.begin(), s.values.end()));
    }
    if (maxValue > 0) {
        std::vector<float> ticks;
        if (logScale) {
            ticks.push_back(0);
            for (float t = 1; t <= maxValue; t *= 10) {
                ticks.push_back(t);
            }
        } else {
            for (int k = 0; k <= 4; k++) {
                ticks.push_back(maxValue * k / 4.0f);
            }
        }
        for (float value : ticks) {
            double y = plot.bottom() - fraction(value, maxValue) * plot.height();
            QString label = maxValue >= 10 ? QString::number(value, 'f', 0) : QString::number(value, 'f', 2);
            painter.setPen(gridColor);
            painter.drawLine(QLineF(plot.left(), y, plot.right(), y));
            painter.setPen(textColor);
            painter.drawLine(QLineF(plot.left() - 4, y, plot.left(), y));
            painter.drawText(QRectF(0, y - 8, left - 8, 16), Qt::AlignRight | Qt::AlignVCenter, label);
        }

        int currentCount = 0;
        for (const Series &s : series) {
            if (!s.original) {
                currentCount++;
            }
        }
        for (bool drawOriginals : {false, true}) {
            for (const Series &s : series) {
                if (s.original != drawOriginals) {
                    continue;
                }
                if (s.original) {
                    QPolygonF line;
                    for (int i = 0; i < 256; i++) {
                        double y = plot.bottom() - fraction(s.values[i], maxValue) * plot.height();
                        if (style == Style::Bars) {
                            line.append(QPointF(plot.left() + i * binWidth, y));
                            line.append(QPointF(plot.left() + (i + 1) * binWidth, y));
                        } else {
                            line.append(QPointF(plot.left() + (i + 0.5) * binWidth, y));
                        }
                    }
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.setPen(QPen(s.color, 1.5, Qt::DashLine));
                    painter.drawPolyline(line);
                    painter.setRenderHint(QPainter::Antialiasing, false);
                } else if (style == Style::Bars) {
                    QColor color = s.color;
                    if (currentCount > 1) {
                        color.setAlpha(110);
                    }
                    for (int i = 0; i < 256; i++) {
                        int x0 = qRound(plot.left() + i * binWidth);
                        int x1 = qRound(plot.left() + (i + 1) * binWidth);
                        int barHeight = qRound(fraction(s.values[i], maxValue) * plot.height());
                        painter.fillRect(QRect(x0, qRound(plot.bottom()) - barHeight, x1 - x0, barHeight), color);
                    }
                } else {
                    QPolygonF line;
                    for (int i = 0; i < 256; i++) {
                        double x = plot.left() + (i + 0.5) * binWidth;
                        double y = plot.bottom() - fraction(s.values[i], maxValue) * plot.height();
                        line.append(QPointF(x, y));
                    }
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.setPen(QPen(s.color, 2));
                    painter.drawPolyline(line);
                    painter.setRenderHint(QPainter::Antialiasing, false);
                }
            }
        }
    }

    painter.setPen(textColor);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(plot);
}