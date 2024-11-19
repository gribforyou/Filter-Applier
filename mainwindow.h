#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImage();
    void saveImageAs();
    void showOriginal();
    void showHighPass();
    void showEqualized();
    void showHystogram();

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QImage originalImage;
    QImage highPassImage;
    QImage equalizedImage;
    QImage hystogramm;
    int curIm;

    void resizeEvent(QResizeEvent *event) override;

    QImage applyHighPassFilter(const QImage& image);
    QImage applyHistogramEqualization(const QImage& image);
};
#endif // MAINWINDOW_H
