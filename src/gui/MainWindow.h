#pragma once

#include <array>
#include <memory>
#include <vector>

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPixmap>
#include <QString>
#include <QTableWidget>

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
    void exportPlots();
    void exportData(const QString &kind);
    void resetImage();
    void undo();
    void redo();
    void pushUndo();
    void updateHistory();
    void equalizeGlobal();
    void equalizeLocal();
    void clahe();
    void matchImage();
    void refreshImage();
    void showImage();
    void changeSpace();
    void computeHistogram();
    void computeOriginalHistogram();
    bool isModified() const;
    void computeComponents();
    void updatePlots();
    void addChannelPlots(Channel channel, const QColor &color);
    void addStatisticsColumn(Channel channel, const QString &name);

    QAction *saveAction = nullptr;
    QAction *exportPlotsAction = nullptr;
    QMenu *exportMenu = nullptr;
    QWidget *plotsPanel = nullptr;
    QAction *undoAction = nullptr;
    QAction *redoAction = nullptr;
    QAction *resetAction = nullptr;
    QAction *statsAction = nullptr;
    QAction *showOriginalAction = nullptr;
    QAction *logScaleAction = nullptr;
    QTableWidget *statsTable = nullptr;
    QMenu *toolsMenu = nullptr;
    QLabel *imageLabel = nullptr;
    QComboBox *spaceBox = nullptr;
    QComboBox *plotBox = nullptr;
    QComboBox *imageBox = nullptr;
    PlotWidget *histogramPlot = nullptr;
    PlotWidget *cdfPlot = nullptr;

    std::unique_ptr<Image> image;
    std::unique_ptr<Image> original;
    std::vector<std::unique_ptr<Image>> undoStack;
    std::vector<std::unique_ptr<Image>> redoStack;
    bool historyTrimmed = false;
    std::unique_ptr<Histogram> histogram;
    std::unique_ptr<Histogram> originalHistogram;
    QString imagePath;
    QPixmap pixmap;
    std::array<QPixmap, 3> componentPixmaps;
};