#ifndef PROJECTIONPROCESSORTHREAD_H
#define PROJECTIONPROCESSORTHREAD_H

#include <QThread>
#include <QPixmap>
#include "opencv2/opencv.hpp"
#include <QMutexLocker>

class ProjectionProcessorThread : public QThread
{
    Q_OBJECT
public:
    explicit ProjectionProcessorThread(QObject *parent = nullptr);


signals:
    void inDisplay(QPixmap pixmap);
    void inDisplay2(QPixmap pixmap);
    void CameraOn(QString) const ;
    void CameraOff(QString) const ;
    void newFrame(QPixmap pix);
    void newFrame2(QPixmap pix);
public slots:
    void onNewColorValue(int c);

    void setTrackRect(QRect rect);
private:
    void run() override;
    int color;
    cv::Rect trackRect;
    QMutex rectMutex;
    bool updateHistogram;
};

#endif // PROJECTIONPROCESSORTHREAD_H
