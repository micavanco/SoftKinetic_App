#ifndef DEPTHVIDEOPROCESSORTHREAD_H
#define DEPTHVIDEOPROCESSORTHREAD_H

#include <QThread>
#include <QPixmap>
#include <DepthSense.hxx>
#include "opencv2/opencv.hpp"
#include "camerathread.h"

class DepthVideoProcessorThread : public QThread
{
    Q_OBJECT
public:
    explicit DepthVideoProcessorThread(QObject *parent = nullptr);

    cv::Mat ModDepthForDisplay(const cv::Mat &mat);
    void configureNode(DepthSense::Node node);
    void configureDepthNode();
    void setConfiguration(int framerate, DepthSense::FrameFormat frameformat, DepthSense::DepthNode::CameraMode cameraMode, int colorType);
    void setDepthValue(int x, int y);
    float getDepthValue();

signals:
    void inDisplay(QPixmap pixmap);
    void CameraOn(QString) const ;
    void CameraOff(QString) const ;
    void newValue(QString);
public slots:
    void onGettingDepth();
    void offGettingDepth();
private:
    DepthSense::Context    m_context;
    DepthSense::DepthNode  m_dnode;
    bool m_bDeviceFound;
    int m_frameRate;
    DepthSense::FrameFormat m_frameFormat;
    DepthSense::DepthNode::CameraMode m_cameraMode;
    int m_colorType;


    friend void onNewDepthSample(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data);
    void run() override;

};

#endif // DEPTHVIDEOPROCESSORTHREAD_H
