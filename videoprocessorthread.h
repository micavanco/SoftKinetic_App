#ifndef VIDEOPROCESSORTHREAD_H
#define VIDEOPROCESSORTHREAD_H

#include <QThread>
#include <QPixmap>
#include "opencv2/opencv.hpp"
#include "camerathread.h"
#include <DepthSense.hxx>

class VideoProcessorThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoProcessorThread(QObject *parent = nullptr);

    void configureColorNode();
    void configureNode(DepthSense::Node node);
signals:
    void inDisplay(QPixmap pixmap);
    void CameraOn(QString) const ;
    void CameraOff(QString) const ;
public slots:
private:
    DepthSense::Context    m_context;
    DepthSense::ColorNode  m_cnode;
    bool m_bDeviceFound;
    void run() override;

    friend void onNewColorSample2(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data);
};

#endif // VIDEOPROCESSORTHREAD_H
