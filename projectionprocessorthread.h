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
    void configureDepthNode();
    cv::Mat ModDepthForDisplay(const cv::Mat &mat);

signals:
    void inDisplay(QPixmap pixmap);
    void inDisplay2(QPixmap pixmap);
    void CameraOn(QString) const ;
    void CameraOff(QString) const ;
    void newFrame(QPixmap pix);
    void newFrame2(QPixmap pix);
    void newFrame3(QPixmap pix);
    void monitorValue(QString) const;
    void monitorDepthValue(QString) const;
    void monitorValue2(QString) const;
    void monitorDepthValue2(QString) const;
    void newTitle(QString) const;
    void newTitle2(QString) const;
    void checkIfRecord();
public slots:

    void setTrackRect(QRect rect);
    void setTrackRect2(QRect rect);
    void setHueRed(int r){hueRed=r;}
    void setSaturationRed(int r){saturationRed=r;}
    void setValueRed(int r){valueRed=r;}
    void setHueBlue(int b){hueBlue=b;}
    void setSaturationBlue(int b){saturationBlue=b;}
    void setValueBlue(int b){valueBlue=b;}
private:
    DepthSense::Context    m_context;
    DepthSense::ColorNode  m_cnode;
    DepthSense::DepthNode  m_dnode;
    bool m_bDeviceFound;
    float corX;
    float corY;
    float corX2;
    float corY2;
    int hueRed;
    int saturationRed;
    int valueRed;
    int hueBlue;
    int saturationBlue;
    int valueBlue;
    cv::Rect trackRect;
    cv::Rect trackRect2;
    QMutex rectMutex;
    QMutex rectMutex2;
    bool updateHistogram;
    bool updateHistogram2;

    void run() override;
    friend void onNewColorSample(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data);
    friend void onNewDepthSample2(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data);
};

#endif // PROJECTIONPROCESSORTHREAD_H
