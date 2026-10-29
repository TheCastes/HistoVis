#pragma once

#include <array>
#include <memory>

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPixmap>
#include <QString>

#include "core/Histogram.h"
#include "core/Image.h"
#include "gui/PlotWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void openImage();
    void loadImage(const QString &path);
    void saveImage();
    void revertImage();
    void equalizeGlobal();
    void equalizeLocal();
    void matchImage();
    void refreshImage();
    void showImage();
    void changeSpace();
    void computeHistogram();
    void computeComponents();
    void updatePlots();
    void addChannelPlots(Channel channel, const QColor &color);

    QAction *saveAction = nullptr;
    QAction *revertAction = nullptr;
    QMenu *toolsMenu = nullptr;
    QLabel *imageLabel = nullptr;
    QComboBox *spaceBox = nullptr;
    QComboBox *plotBox = nullptr;
    QComboBox *imageBox = nullptr;
    PlotWidget *histogramPlot = nullptr;
    PlotWidget *cdfPlot = nullptr;

    std::unique_ptr<Image> image;
    std::unique_ptr<Histogram> histogram;
    QString imagePath;
    QPixmap pixmap;
    std::array<QPixmap, 3> componentPixmaps;
};