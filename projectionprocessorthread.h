#ifndef PROJECTIONPROCESSORTHREAD_H
#define PROJECTIONPROCESSORTHREAD_H

#include <QThread>
#include <QPixmap>
#include "opencv2/opencv.hpp"
#include "camerathread.h"
#include <QMutexLocker>
#include <DepthSense.hxx>

class ProjectionProcessorThread : public QThread
{
    Q_OBJECT
public:
    explicit ProjectionProcessorThread(QObject *parent = nullptr);
    void configureColorNode();
    void configureNode(DepthSense::Node node);

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
    DepthSense::Context    m_context;
    DepthSense::ColorNode  m_cnode;
    bool m_bDeviceFound;
    int color;
    cv::Rect trackRect;
    QMutex rectMutex;
    bool updateHistogram;

    void run() override;
    friend void onNewColorSample(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data);
};

#endif // PROJECTIONPROCESSORTHREAD_H
