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

    void on_colorSlider_sliderMoved(int position);
    void onRubberBandChanged(QRect rect,
    QPointF frScn, QPointF toScn);
    void onNewFrame(QPixmap newFrm);

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
};

#endif // MAINWINDOW_H
