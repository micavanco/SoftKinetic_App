#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>

#include <QDockWidget>
#include "videoprocessorthread.h"
#include "depthvideoprocessorthread.h"
#include "projectionprocessorthread.h"
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QFileDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MovementAnalyzer;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openpushButton_pressed();
    void on_closepushButton_2_pressed();
    void on_cameraviewpushButton_pressed();
    void on_cameraviewDock_visibilityChanged(bool visible);
    void on_normalradioButton_pressed();
    void on_normalradioButton_clicked(bool checked);
    void on_depthradioButton_clicked(bool checked);
    void on_depthradioButton_pressed();

    void on_testButton_pressed();

    void on_recordButton_pressed();

    void on_openpushButton_2_pressed();

    void on_closepushButton_3_pressed();

    void on_cameraviewpushButton_2_pressed();
    void on_cameraviewpushButton_8_pressed();
    void on_analysisDock_visibilityChanged(bool visible);
    void onRubberBandChanged(QRect rect,
    QPointF frScn, QPointF toScn);
    void onRubberBandChanged2(QRect rect,
    QPointF frScn, QPointF toScn);
    void onNewFrame(QPixmap newFrm);
    void onNewFrame2(QPixmap newFrm);
    void onNewTitle();

    void on_recordmoveButton_pressed();
    void onTimerTimeout();

    void on_redSlider1_valueChanged(int value);
    void on_redSlider2_valueChanged(int value);
    void on_redSlider3_valueChanged(int value);

    void on_blueSlider1_valueChanged(int value);
    void on_blueSlider2_valueChanged(int value);
    void on_blueSlider3_valueChanged(int value);

    void on_analyseButton_pressed();

    void on_inTimeRadio_pressed();

    void on_inSpaceRadio_pressed();

    void on_in2filesRadio_pressed();

    void on_analyseProcessButton_pressed();

    void on_openFile1_pressed();

    void on_openFile2_pressed();

signals:
    void mouseOnScreen();
    void mouseOffScreen();
private:
    Ui::MovementAnalyzer *ui;
    VideoProcessorThread processor;
    DepthVideoProcessorThread depthprocessor;
    ProjectionProcessorThread projectionprocessor;
    bool eventFilter(QObject *obj, QEvent *event);
    QPoint startPoint;
    bool wasAdd;
    QGraphicsPixmapItem pixmap;
    QGraphicsPixmapItem pixmap2;
    QTimer *timer;
    double time;
    double totalTime;
    double *object1ArrayX;
    double *object1ArrayY;
    double *object2ArrayX;
    double *object2ArrayY;
    int array1Length;
    double *object1ArrayX2;
    double *object1ArrayY2;
    double *object2ArrayX2;
    double *object2ArrayY2;
    QLineSeries *series1Object1;
    QLineSeries *series1Object2;
    QLineSeries *series2Object1;
    QLineSeries *series2Object2;
    QChart *chart1;
    QChart *chart2;
    QChartView *chartView1;
    QChartView *chartView2;
    int array2Length;
    QFile *file;
    QTextStream *streamOut;
    QString fileTitle;
};

#endif // MAINWINDOW_H
