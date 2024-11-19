#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Apply Filters");

    QAction *openAction = new QAction("&Open Image", this);
    QAction *saveAction = new QAction("&Save current Image as...", this);
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImageAs);

    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);

    imageLabel = new QLabel(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    QPushButton *originalButton = new QPushButton("Show original", this);
    QPushButton *highPassButton = new QPushButton("Show High pass filter", this);
    QPushButton *equalizedButton = new QPushButton("Show equalized", this);
    QPushButton *hystogrammButton = new QPushButton("Show hystogramm", this);

    connect(originalButton, &QPushButton::clicked, this, &MainWindow::showOriginal);
    connect(highPassButton, &QPushButton::clicked, this, &MainWindow::showHighPass);
    connect(equalizedButton, &QPushButton::clicked, this, &MainWindow::showEqualized);
    connect(hystogrammButton, &QPushButton::clicked, this, &MainWindow::showHystogram);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(originalButton);
    buttonLayout->addWidget(highPassButton);
    buttonLayout->addWidget(equalizedButton);
    buttonLayout->addWidget(hystogrammButton);

    mainLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    mainLayout->addLayout(buttonLayout);

    QWidget *container = new QWidget();
    container->setLayout(mainLayout);
    setCentralWidget(container);

    originalImage = QImage(":/Pictures/image.png");
    equalizedImage = applyHistogramEqualization(originalImage);
    highPassImage = applyHighPassFilter(originalImage);
    showOriginal();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.xpm *.jpg)");
    if (!fileName.isEmpty()) {
        originalImage = QImage(fileName);
        if (originalImage.isNull()) {
            qDebug() << "Ошибка: Не удалось загрузить оригинальное изображение!";
            return;
        }

        highPassImage = applyHighPassFilter(originalImage);
        if (highPassImage.isNull()) {
            qDebug() << "Ошибка: Не удалось применить фильтр high-pass!";
        }

        equalizedImage = applyHistogramEqualization(originalImage);
        if (equalizedImage.isNull()) {
            qDebug() << "Ошибка: Не удалось применить эквализацию гистограммы!";
        }

        showOriginal();
    }
}

void MainWindow::saveImageAs()
{
    QImage image = imageLabel->pixmap().toImage();
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Сохранить изображение", "", "Images (*.png *.xpm *.jpg)");
    if (!fileName.isEmpty()) {
        image.save(fileName);
    }
}

void MainWindow::showOriginal() {
    if (originalImage.isNull()) {
        qDebug() << "Ошибка: Нет загруженного оригинального изображения!";
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(originalImage).scaled(this->width() * 0.8, this->height() * 0.8, Qt::KeepAspectRatio);
    imageLabel->setPixmap(pixmap);
    curIm = 0;
}

void MainWindow::showHighPass() {
    if (highPassImage.isNull()) {
        qDebug() << "Ошибка: Нет загруженного изображения с примененным high-pass фильтром!";
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(highPassImage).scaled(this->width() * 0.8, this->height() * 0.8, Qt::KeepAspectRatio);
    if(pixmap.isNull()) qDebug() << "highpass pixmap is null";
    imageLabel->setPixmap(pixmap);
    curIm = 1;
  }

void MainWindow::showEqualized() {
    if (equalizedImage.isNull()) {
        qDebug() << "Ошибка: Нет загруженного изображения с примененной эквализацией гистограммы!";
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(equalizedImage).scaled(this->width() * 0.8, this->height() * 0.8, Qt::KeepAspectRatio);
    if(pixmap.isNull()) qDebug() << "equalized pixmap is null";
    imageLabel->setPixmap(pixmap);
    curIm = 2;
}

void MainWindow::showHystogram()
{
    if(hystogramm.isNull()){
        qDebug() << "Ошибка: Нет загруженного изображения гистограммы!";
    }
    QPixmap pixmap = QPixmap::fromImage(hystogramm).scaled(this->width() * 0.8, this->height() * 0.8, Qt::KeepAspectRatio);
    if(pixmap.isNull()) qDebug() << "hystogramm pixmap is null";
    imageLabel->setPixmap(pixmap);
    curIm = 3;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    switch(curIm){
    case 0:
        showOriginal();
        break;
    case 1:
        showHighPass();
        break;
    case 2:
        showEqualized();
        break;
    case 3:
        showHystogram();
        break;
    }
}

QImage MainWindow::applyHighPassFilter(const QImage &image)
{
    cv::Mat matImage = cv::Mat(image.height(), image.width(), CV_8UC4,
                               const_cast<uchar*>(image.bits()), image.bytesPerLine());


    cv::Mat bgrImage;
    cv::cvtColor(matImage, bgrImage, cv::COLOR_RGBA2BGR);

    cv::Mat grayImage;
    cv::cvtColor(bgrImage, grayImage, cv::COLOR_BGR2GRAY);

    cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
                          0, -1, 0,
                      -1, 5, -1,
                      0, -1, 0);

    cv::Mat filteredImage;
    cv::filter2D(grayImage, filteredImage, CV_8U, kernel);

    cv::Mat filteredBGR;
    cv::cvtColor(filteredImage, filteredBGR, cv::COLOR_GRAY2BGR);

    QImage outputImage(filteredBGR.data, filteredBGR.cols, filteredBGR.rows, filteredBGR.step[0], QImage::Format_BGR888);

    return outputImage.copy();
}

QImage MainWindow::applyHistogramEqualization(const QImage &image) {
    cv::Mat matImage = cv::Mat(image.height(), image.width(), CV_8UC4,
                               const_cast<uchar*>(image.bits()), image.bytesPerLine());


    cv::Mat bgrImage;
    cv::cvtColor(matImage, bgrImage, cv::COLOR_RGBA2BGR);

    cv::Mat grayImage;
    cv::cvtColor(bgrImage, grayImage, cv::COLOR_BGR2GRAY);

    int histogram[256] = {0};
    for (int i = 0; i < grayImage.rows; ++i) {
        for (int j = 0; j < grayImage.cols; ++j) {
            histogram[grayImage.at<uchar>(i, j)]++;
        }
    }
    cv::Mat histImage(400, 512, CV_8UC1, cv::Scalar(255));
    int maxVal = *std::max_element(histogram, histogram + 256);
    for (int i = 0; i < 256; i++) {
        float binValue = static_cast<float>(histogram[i]) / maxVal * histImage.rows;
        cv::line(histImage, cv::Point(i, histImage.rows), cv::Point(i, histImage.rows - static_cast<int>(binValue)), cv::Scalar(0), 1, 8, 0);
    }
    QImage qImage(histImage.data, histImage.cols, histImage.rows, histImage.step[0], QImage::Format_Grayscale8);
    hystogramm = qImage.copy();

    cv::Mat equalizedImage;
    cv::equalizeHist(grayImage, equalizedImage);

    cv::Mat equalizedBGR;
    cv::cvtColor(equalizedImage, equalizedBGR, cv::COLOR_GRAY2BGR);

    QImage outputImage(equalizedBGR.data, equalizedBGR.cols, equalizedBGR.rows, equalizedBGR.step[0], QImage::Format_BGR888);

    return outputImage.copy();
}


