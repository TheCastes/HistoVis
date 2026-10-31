#include "MainWindow.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QRect>
#include <QSpinBox>
#include <QStringList>
#include <QToolBar>
#include <QVBoxLayout>

#include "core/Equalization.h"
#include "core/Matching.h"

namespace {
    QImage toQImage(const Image &image) {
        int width = image.getWidth();
        int height = image.getHeight();
        int channels = image.getChannels();
        if (channels == 2) {
            QImage result(width, height, QImage::Format_RGBA8888);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    Pixel p = image.getPixel(x, y);
                    result.setPixelColor(x, y, QColor(p.R, p.G, p.B, p.A));
                }
            }
            return result;
        }
        QImage::Format format = QImage::Format_RGB888;
        if (channels == 1) {
            format = QImage::Format_Grayscale8;
        } else if (channels == 4) {
            format = QImage::Format_RGBA8888;
        }
        return QImage(image.getData(), width, height, width * channels, format).copy();
    }
} //namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("HistoVis");
    resize(1200, 800);

    QAction *openAction = new QAction(QIcon::fromTheme("document-open"), "&Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    saveAction = new QAction(QIcon::fromTheme("document-save"), "&Save...", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setEnabled(false);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);

    QAction *quitAction = new QAction(QIcon::fromTheme("application-exit"), "&Quit", this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    exportPlotsAction = new QAction("Export &plots...", this);
    exportPlotsAction->setEnabled(false);
    connect(exportPlotsAction, &QAction::triggered, this, &MainWindow::exportPlots);

    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exportPlotsAction);
    exportMenu = fileMenu->addMenu("Export &data");
    exportMenu->addAction("&Histogram...", this, [this]() { exportData("hist"); });
    exportMenu->addAction("&PDF...", this, [this]() { exportData("pdf"); });
    exportMenu->addAction("&CDF...", this, [this]() { exportData("cdf"); });
    exportMenu->addAction("&Statistics...", this, [this]() { exportData("stats"); });
    exportMenu->setEnabled(false);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    undoAction = new QAction(QIcon::fromTheme("edit-undo"), "&Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);

    redoAction = new QAction(QIcon::fromTheme("edit-redo"), "&Redo", this);
    redoAction->setShortcuts({QKeySequence("Ctrl+Y"), QKeySequence("Ctrl+Shift+Z")});
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);

    resetAction = new QAction(QIcon::fromTheme("document-revert"), "Re&set", this);
    resetAction->setShortcut(QKeySequence("Ctrl+R"));
    resetAction->setEnabled(false);
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetImage);

    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(resetAction);

    statsAction = new QAction("&Statistics", this);
    statsAction->setCheckable(true);
    connect(statsAction, &QAction::toggled, this, [this](bool checked) {
        statsTable->setVisible(checked);
    });

    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(statsAction);

    showOriginalAction = new QAction("Show &original", this);
    showOriginalAction->setCheckable(true);
    connect(showOriginalAction, &QAction::toggled, this, &MainWindow::updatePlots);
    viewMenu->addAction(showOriginalAction);

    logScaleAction = new QAction("&Logarithmic scale", this);
    logScaleAction->setCheckable(true);
    connect(logScaleAction, &QAction::toggled, this, [this](bool checked) {
        histogramPlot->setLogScale(checked);
    });
    viewMenu->addAction(logScaleAction);

    QAction *equalizeGlobalAction = new QAction("Equalize (&global)...", this);
    connect(equalizeGlobalAction, &QAction::triggered, this, &MainWindow::equalizeGlobal);

    QAction *equalizeLocalAction = new QAction("Equalize (&local)...", this);
    connect(equalizeLocalAction, &QAction::triggered, this, &MainWindow::equalizeLocal);

    QAction *claheAction = new QAction("&CLAHE...", this);
    connect(claheAction, &QAction::triggered, this, &MainWindow::clahe);

    QAction *matchAction = new QAction("&Match...", this);
    connect(matchAction, &QAction::triggered, this, &MainWindow::matchImage);

    toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction(equalizeGlobalAction);
    toolsMenu->addAction(equalizeLocalAction);
    toolsMenu->addAction(claheAction);
    toolsMenu->addAction(matchAction);
    toolsMenu->setEnabled(false);

    QToolBar *toolBar = addToolBar("Toolbar");
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addSeparator();
    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);
    toolBar->addSeparator();

    spaceBox = new QComboBox;
    spaceBox->addItems({"RGB", "YCbCr", "Lab"});
    connect(spaceBox, &QComboBox::currentIndexChanged, this, &MainWindow::changeSpace);
    toolBar->addWidget(new QLabel(" Space: "));
    toolBar->addWidget(spaceBox);

    plotBox = new QComboBox;
    connect(plotBox, &QComboBox::currentIndexChanged, this, &MainWindow::updatePlots);
    toolBar->addWidget(new QLabel(" Plot: "));
    toolBar->addWidget(plotBox);

    imageBox = new QComboBox;
    connect(imageBox, &QComboBox::currentIndexChanged, this, &MainWindow::showImage);
    toolBar->addWidget(new QLabel(" Components: "));
    toolBar->addWidget(imageBox);

    imageLabel = new QLabel("Open an image (File → Open)");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setMinimumSize(1, 1);

    histogramPlot = new PlotWidget("Histogram", PlotWidget::Style::Bars);
    cdfPlot = new PlotWidget("CDF", PlotWidget::Style::Line);

    plotsPanel = new QWidget;
    QVBoxLayout *plotsLayout = new QVBoxLayout(plotsPanel);
    plotsLayout->setContentsMargins(0, 0, 0, 0);
    plotsLayout->addWidget(histogramPlot);
    plotsLayout->addWidget(cdfPlot);

    statsTable = new QTableWidget(9, 0);
    statsTable->setVerticalHeaderLabels({"Min", "Max", "Range (min–max)", "Range (P1–P99)", "Mean", "Median",
                                         "Mode", "Std dev", "IQR (P25–P75)"});
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setSelectionMode(QAbstractItemView::NoSelection);
    statsTable->setFocusPolicy(Qt::NoFocus);
    statsTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    statsTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    int rowHeight = statsTable->fontMetrics().height() + 6;
    statsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    statsTable->verticalHeader()->setDefaultSectionSize(rowHeight);
    statsTable->setFixedHeight(statsTable->horizontalHeader()->sizeHint().height() + 9 * rowHeight +
                               2 * statsTable->frameWidth());
    statsTable->setToolTip("Values in bin scale 0-255: Cb, Cr, a*, b* are neutral at 128, L* 255 is L* 100");
    statsTable->setVisible(false);
    plotsLayout->addWidget(statsTable);

    QWidget *central = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->addWidget(imageLabel, 2);
    layout->addWidget(plotsPanel, 1);
    setCentralWidget(central);

    changeSpace();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    showImage();
}

void MainWindow::openImage() {
    QString path = QFileDialog::getOpenFileName(this, "Open image", QString(),
                                                "Images (*.png *.jpg *.jpeg *.bmp);;All files (*)");
    if (path.isEmpty()) {
        return;
    }
    loadImage(path);
}

void MainWindow::loadImage(const QString &path) {
    std::unique_ptr<Image> loaded = std::make_unique<Image>(path.toLocal8Bit().constData());
    if (!loaded->isValid()) {
        QMessageBox::warning(this, "Error", "Cannot open image:\n" + path);
        return;
    }
    image = std::move(loaded);
    original = image->clone();
    undoStack.clear();
    redoStack.clear();
    historyTrimmed = false;
    imagePath = path;
    saveAction->setEnabled(true);
    exportPlotsAction->setEnabled(true);
    exportMenu->setEnabled(true);
    toolsMenu->setEnabled(true);
    updateHistory();
    computeOriginalHistogram();
    refreshImage();
}

void MainWindow::saveImage() {
    QString path = QFileDialog::getSaveFileName(this, "Save image", QFileInfo(imagePath).absolutePath(),
                                                "PNG (*.png);;JPEG (*.jpg *.jpeg)");
    if (path.isEmpty()) {
        return;
    }
    QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix.isEmpty()) {
        path += ".png";
        suffix = "png";
    }
    bool saved = false;
    if (suffix == "png") {
        saved = image->saveImagePNG(path.toLocal8Bit().constData());
    } else if (suffix == "jpg" || suffix == "jpeg") {
        saved = image->saveImageJPG(path.toLocal8Bit().constData());
    } else {
        QMessageBox::warning(this, "Error", "Output must be .png, .jpg or .jpeg:\n" + path);
        return;
    }
    if (!saved) {
        QMessageBox::warning(this, "Error", "Cannot save image:\n" + path);
    }
}

void MainWindow::exportPlots() {
    QFileInfo info(imagePath);
    QString suggested = info.absolutePath() + "/" + info.completeBaseName() + "_plots.png";
    QString path = QFileDialog::getSaveFileName(this, "Export plots", suggested, "PNG (*.png)");
    if (path.isEmpty()) {
        return;
    }
    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".png";
    }
    const qreal scale = 2.0;
    QPixmap plotsImage(plotsPanel->size() * scale);
    plotsImage.setDevicePixelRatio(scale);
    plotsImage.fill(palette().color(QPalette::Window));
    plotsPanel->render(&plotsImage);
    if (!plotsImage.save(path, "PNG")) {
        QMessageBox::warning(this, "Error", "Cannot save plots:\n" + path);
    }
}

void MainWindow::exportData(const QString &kind) {
    QFileInfo info(imagePath);
    QString suggested = info.absolutePath() + "/" + info.completeBaseName() + "_" +
                        spaceBox->currentText().toLower() + "_" + kind + ".csv";
    QString path = QFileDialog::getSaveFileName(this, "Export data", suggested, "CSV (*.csv)");
    if (path.isEmpty()) {
        return;
    }
    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".csv";
    }
    std::ofstream file(path.toLocal8Bit().constData());
    if (!file) {
        QMessageBox::warning(this, "Error", "Cannot write file:\n" + path);
        return;
    }
    //RGB view: r,g,b; YCbCr or Lab view: the three channels of the space, like -s in the CLI
    ExportChannels channels = spaceBox->currentIndex() == 0 ? ExportChannels::RGB : ExportChannels::Space;
    if (kind == "hist") {
        histogram->exportHistogram(file, channels);
    } else if (kind == "pdf") {
        histogram->exportPDF(file, channels);
    } else if (kind == "cdf") {
        histogram->exportCDF(file, channels);
    } else {
        histogram->exportStatistics(file, channels);
    }
    file.close();
    if (file.fail()) {
        QMessageBox::warning(this, "Error", "Cannot write file:\n" + path);
    }
}

void MainWindow::resetImage() {
    QMessageBox::StandardButton answer = QMessageBox::warning(
        this, "Reset", "Go back to the original image?\nThe undo history will be lost.",
        QMessageBox::Reset | QMessageBox::Cancel, QMessageBox::Cancel);
    if (answer != QMessageBox::Reset) {
        return;
    }
    image = original->clone();
    undoStack.clear();
    redoStack.clear();
    historyTrimmed = false;
    updateHistory();
    refreshImage();
}

void MainWindow::undo() {
    if (undoStack.empty()) {
        return;
    }
    redoStack.push_back(std::move(image));
    image = std::move(undoStack.back());
    undoStack.pop_back();
    updateHistory();
    refreshImage();
}

void MainWindow::redo() {
    if (redoStack.empty()) {
        return;
    }
    undoStack.push_back(std::move(image));
    image = std::move(redoStack.back());
    redoStack.pop_back();
    updateHistory();
    refreshImage();
}

void MainWindow::pushUndo() {
    const size_t maxUndo = 10;
    undoStack.push_back(image->clone());
    if (undoStack.size() > maxUndo) {
        undoStack.erase(undoStack.begin());
        historyTrimmed = true;
    }
    redoStack.clear();
}

void MainWindow::updateHistory() {
    bool modified = isModified();
    undoAction->setEnabled(!undoStack.empty());
    redoAction->setEnabled(!redoStack.empty());
    resetAction->setEnabled(modified || !redoStack.empty());
    QString title = "HistoVis - " + QFileInfo(imagePath).fileName();
    if (modified) {
        title += " (modified)";
    }
    setWindowTitle(title);
}

void MainWindow::equalizeGlobal() {
    QDialog dialog(this);
    dialog.setWindowTitle("Equalize (global)");
    QComboBox *spaceChoice = new QComboBox;
    spaceChoice->addItems({"YCbCr", "Lab"});
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QFormLayout *form = new QFormLayout(&dialog);
    form->addRow("Space:", spaceChoice);
    form->addRow(buttons);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    LuminanceSpace space = spaceChoice->currentIndex() == 1 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
    pushUndo();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    equalize(*image, space);
    QApplication::restoreOverrideCursor();
    updateHistory();
    refreshImage();
}

void MainWindow::equalizeLocal() {
    int shorter = std::min(image->getWidth(), image->getHeight());
    if (shorter < 3) {
        QMessageBox::warning(this, "Error", "Image too small for local equalization");
        return;
    }
    int maxWindow = shorter % 2 == 0 ? shorter - 1 : shorter;
    int defaultWindow = std::min(std::max(3, (shorter / 8) | 1), maxWindow);

    QDialog dialog(this);
    dialog.setWindowTitle("Equalize (local)");
    QComboBox *spaceChoice = new QComboBox;
    spaceChoice->addItems({"YCbCr", "Lab"});
    QSpinBox *windowChoice = new QSpinBox;
    windowChoice->setRange(3, maxWindow);
    windowChoice->setSingleStep(2);
    windowChoice->setValue(defaultWindow);
    windowChoice->setSuffix(" px");
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QFormLayout *form = new QFormLayout(&dialog);
    form->addRow("Space:", spaceChoice);
    form->addRow("Window (odd):", windowChoice);
    form->addRow(buttons);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    LuminanceSpace space = spaceChoice->currentIndex() == 1 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
    int window = windowChoice->value() | 1;
    pushUndo();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ::equalizeLocal(*image, window, space);
    QApplication::restoreOverrideCursor();
    updateHistory();
    refreshImage();
}

void MainWindow::clahe() {
    int shorter = std::min(image->getWidth(), image->getHeight());
    if (shorter < 3) {
        QMessageBox::warning(this, "Error", "Image too small for CLAHE");
        return;
    }
    int maxWindow = shorter % 2 == 0 ? shorter - 1 : shorter;
    int defaultWindow = std::min(std::max(3, (shorter / 8) | 1), maxWindow);

    QDialog dialog(this);
    dialog.setWindowTitle("CLAHE");
    QComboBox *spaceChoice = new QComboBox;
    spaceChoice->addItems({"YCbCr", "Lab"});
    QSpinBox *windowChoice = new QSpinBox;
    windowChoice->setRange(3, maxWindow);
    windowChoice->setSingleStep(2);
    windowChoice->setValue(defaultWindow);
    windowChoice->setSuffix(" px");
    QDoubleSpinBox *clipChoice = new QDoubleSpinBox;
    clipChoice->setRange(1.0, 20.0);
    clipChoice->setSingleStep(0.5);
    clipChoice->setDecimals(1);
    clipChoice->setValue(2.0);
    QLabel *clipWarning = new QLabel("Clip limits below 1.5 can be very slow with large windows");
    clipWarning->setStyleSheet("color: #c06000;");
    clipWarning->setVisible(false);
    connect(clipChoice, &QDoubleSpinBox::valueChanged, clipWarning, [clipWarning](double value) {
        clipWarning->setVisible(value < 1.5);
    });
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QFormLayout *form = new QFormLayout(&dialog);
    form->addRow("Space:", spaceChoice);
    form->addRow("Window (odd):", windowChoice);
    form->addRow("Clip limit:", clipChoice);
    form->addRow("", clipWarning);
    form->addRow(buttons);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    LuminanceSpace space = spaceChoice->currentIndex() == 1 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
    int window = windowChoice->value() | 1;
    float clip = static_cast<float>(clipChoice->value());
    pushUndo();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ::equalizeLocal(*image, window, space, clip);
    QApplication::restoreOverrideCursor();
    updateHistory();
    refreshImage();
}

void MainWindow::matchImage() {
    QDialog dialog(this);
    dialog.setWindowTitle("Match");
    QComboBox *spaceChoice = new QComboBox;
    spaceChoice->addItems({"YCbCr", "Lab"});
    QLineEdit *referenceEdit = new QLineEdit;
    referenceEdit->setReadOnly(true);
    referenceEdit->setPlaceholderText("No reference image");
    referenceEdit->setMinimumWidth(300);
    QPushButton *browseButton = new QPushButton("Browse...");
    QWidget *referenceRow = new QWidget;
    QHBoxLayout *referenceLayout = new QHBoxLayout(referenceRow);
    referenceLayout->setContentsMargins(0, 0, 0, 0);
    referenceLayout->addWidget(referenceEdit);
    referenceLayout->addWidget(browseButton);
    QLabel *previewLabel = new QLabel("No reference image");
    previewLabel->setFixedSize(240, 180);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setFrameShape(QFrame::StyledPanel);
    PlotWidget *referencePlot = new PlotWidget("Reference", PlotWidget::Style::Bars);
    referencePlot->setFixedSize(320, 180);
    QWidget *previewRow = new QWidget;
    QHBoxLayout *previewLayout = new QHBoxLayout(previewRow);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(referencePlot);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QFormLayout *form = new QFormLayout(&dialog);
    form->addRow("Space:", spaceChoice);
    form->addRow("Reference:", referenceRow);
    form->addRow("", previewRow);
    form->addRow(buttons);

    std::unique_ptr<Image> reference;
    std::function<void()> updateReferencePlot = [&]() {
        referencePlot->clear();
        if (!reference) {
            return;
        }
        LuminanceSpace space = spaceChoice->currentIndex() == 1 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        Histogram histogram(*reference, space);
        QApplication::restoreOverrideCursor();
        Channel channel = histogram.isGreyscale() ? Channel::R : Channel::L;
        const std::array<int, 256> &counts = histogram.getHistogram(channel);
        std::array<float, 256> values{};
        for (int i = 0; i < 256; i++) {
            values[i] = static_cast<float>(counts[i]);
        }
        referencePlot->addSeries(values, palette().color(QPalette::WindowText));
    };
    connect(spaceChoice, &QComboBox::currentIndexChanged, &dialog, [&]() {
        updateReferencePlot();
    });
    connect(browseButton, &QPushButton::clicked, &dialog, [&]() {
        QString path = QFileDialog::getOpenFileName(&dialog, "Open reference image", QFileInfo(imagePath).absolutePath(),
                                                    "Images (*.png *.jpg *.jpeg *.bmp);;All files (*)");
        if (path.isEmpty()) {
            return;
        }
        std::unique_ptr<Image> loaded = std::make_unique<Image>(path.toLocal8Bit().constData());
        if (!loaded->isValid()) {
            QMessageBox::warning(&dialog, "Error", "Cannot open image:\n" + path);
            return;
        }
        reference = std::move(loaded);
        referenceEdit->setText(path);
        previewLabel->setPixmap(QPixmap::fromImage(toQImage(*reference))
                                    .scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        updateReferencePlot();
        buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
    });
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    LuminanceSpace space = spaceChoice->currentIndex() == 1 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
    pushUndo();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    match(*image, *reference, space);
    QApplication::restoreOverrideCursor();
    updateHistory();
    refreshImage();
}

void MainWindow::refreshImage() {
    pixmap = QPixmap::fromImage(toQImage(*image));
    computeHistogram();
    computeComponents();
    showImage();
    updatePlots();
}

void MainWindow::showImage() {
    if (pixmap.isNull()) {
        return;
    }
    int index = imageBox->currentIndex();
    if (index == 0) {
        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }
    if (index <= 3) {
        imageLabel->setPixmap(componentPixmaps[index - 1].scaled(imageLabel->size(), Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation));
        return;
    }

    std::array<const QPixmap *, 4> cells = {&pixmap, &componentPixmaps[0], &componentPixmaps[1], &componentPixmaps[2]};
    int cellWidth = imageLabel->width() / 2;
    int cellHeight = imageLabel->height() / 2;
    QPixmap grid(imageLabel->size());
    grid.fill(Qt::transparent);
    QPainter painter(&grid);
    QFontMetrics metrics(painter.font());
    for (int i = 0; i < 4; i++) {
        QRect cell((i % 2) * cellWidth, (i / 2) * cellHeight, cellWidth, cellHeight);
        cell.adjust(2, 2, -2, -2);
        QPixmap scaled = cells[i]->scaled(cell.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = cell.x() + (cell.width() - scaled.width()) / 2;
        int y = cell.y() + (cell.height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);

        QString name = imageBox->itemText(i);
        QRect textRect(x + 4, y + 4, metrics.horizontalAdvance(name) + 10, metrics.height() + 4);
        painter.fillRect(textRect, QColor(0, 0, 0, 150));
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, name);
    }
    painter.end();
    imageLabel->setPixmap(grid);
}

void MainWindow::changeSpace() {
    QStringList names = {"R", "G", "B"};
    if (spaceBox->currentIndex() == 1) {
        names = {"Y", "Cb", "Cr"};
    } else if (spaceBox->currentIndex() == 2) {
        names = {"L*", "a*", "b*"};
    }

    plotBox->blockSignals(true);
    plotBox->clear();
    plotBox->addItems(names);
    plotBox->addItem("All");
    plotBox->setCurrentIndex(3);
    plotBox->blockSignals(false);

    imageBox->blockSignals(true);
    imageBox->clear();
    imageBox->addItem("Color");
    imageBox->addItems(names);
    imageBox->addItem("All");
    imageBox->setCurrentIndex(0);
    imageBox->blockSignals(false);

    computeOriginalHistogram();
    computeHistogram();
    computeComponents();
    updatePlots();
    showImage();
}

void MainWindow::computeComponents() {
    if (!image) {
        return;
    }
    int width = image->getWidth();
    int height = image->getHeight();
    int space = spaceBox->currentIndex();
    std::array<QImage, 3> components;
    for (int k = 0; k < 3; k++) {
        components[k] = QImage(width, height, QImage::Format_Grayscale8);
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int y = 0; y < height; y++) {
        std::array<uchar *, 3> lines = {components[0].scanLine(y), components[1].scanLine(y), components[2].scanLine(y)};
        for (int x = 0; x < width; x++) {
            Pixel p = image->getPixel(x, y);
            std::array<int, 3> values = {p.R, p.G, p.B};
            if (space == 1) {
                values = Histogram::spaceBins(p, LuminanceSpace::YCbCr);
            } else if (space == 2) {
                values = Histogram::spaceBins(p, LuminanceSpace::Lab);
            }
            for (int k = 0; k < 3; k++) {
                lines[k][x] = static_cast<uchar>(values[k]);
            }
        }
    }
    for (int k = 0; k < 3; k++) {
        componentPixmaps[k] = QPixmap::fromImage(components[k]);
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::computeHistogram() {
    if (!image) {
        return;
    }
    LuminanceSpace space = spaceBox->currentIndex() == 2 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    histogram = std::make_unique<Histogram>(*image, space);
    QApplication::restoreOverrideCursor();
}

void MainWindow::computeOriginalHistogram() {
    if (!original) {
        return;
    }
    LuminanceSpace space = spaceBox->currentIndex() == 2 ? LuminanceSpace::Lab : LuminanceSpace::YCbCr;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    originalHistogram = std::make_unique<Histogram>(*original, space);
    QApplication::restoreOverrideCursor();
}

bool MainWindow::isModified() const {
    return !undoStack.empty() || historyTrimmed;
}

void MainWindow::updatePlots() {
    histogramPlot->clear();
    cdfPlot->clear();
    statsTable->setColumnCount(0);
    if (!histogram) {
        return;
    }
    QColor textColor = palette().color(QPalette::WindowText);
    std::array<Channel, 3> channels = {Channel::R, Channel::G, Channel::B};
    std::array<QColor, 3> colors = {Qt::red, Qt::darkGreen, Qt::blue};
    if (spaceBox->currentIndex() == 1) {
        channels = {Channel::L, Channel::C1, Channel::C2};
        colors = {textColor, Qt::blue, Qt::red};
    } else if (spaceBox->currentIndex() == 2) {
        channels = {Channel::L, Channel::C1, Channel::C2};
        colors = {textColor, Qt::magenta, Qt::darkYellow};
    }
    int index = plotBox->currentIndex();
    for (int i = 0; i < 3; i++) {
        if (index == i || index == 3) {
            addChannelPlots(channels[i], colors[i]);
            addStatisticsColumn(channels[i], plotBox->itemText(i));
        }
    }
}

void MainWindow::addStatisticsColumn(Channel channel, const QString &name) {
    Statistics s = histogram->getStatistics(channel);
    int column = statsTable->columnCount();
    statsTable->insertColumn(column);
    statsTable->setHorizontalHeaderItem(column, new QTableWidgetItem(name));
    QStringList values = {QString::number(s.min), QString::number(s.max), QString::number(s.range),
                          QString::number(s.percentileRange), QString::number(s.mean, 'f', 2),
                          QString::number(s.median), QString::number(s.mode),
                          QString::number(s.stdDev, 'f', 2), QString::number(s.iqr)};
    for (int row = 0; row < values.size(); row++) {
        QTableWidgetItem *item = new QTableWidgetItem(values[row]);
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        statsTable->setItem(row, column, item);
    }
}

void MainWindow::addChannelPlots(Channel channel, const QColor &color) {
    const std::array<int, 256> &counts = histogram->getHistogram(channel);
    std::array<float, 256> countValues{};
    for (int i = 0; i < 256; i++) {
        countValues[i] = static_cast<float>(counts[i]);
    }
    histogramPlot->addSeries(countValues, color);
    cdfPlot->addSeries(histogram->getCDF(channel), color);

    if (showOriginalAction->isChecked() && isModified() && originalHistogram) {
        const std::array<int, 256> &originalCounts = originalHistogram->getHistogram(channel);
        std::array<float, 256> originalValues{};
        for (int i = 0; i < 256; i++) {
            originalValues[i] = static_cast<float>(originalCounts[i]);
        }
        histogramPlot->addSeries(originalValues, color, true);
        cdfPlot->addSeries(originalHistogram->getCDF(channel), color, true);
    }
}