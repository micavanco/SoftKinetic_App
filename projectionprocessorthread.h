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
    void process_contour(cv::Mat& frame, std::vector<cv::Point> const& contour);

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
public slots:

    void setTrackRect(QRect rect);
    void setTrackRect2(QRect rect);
    void setTres(int i);
    void setTres2(int i);
private:
    DepthSense::Context    m_context;
    DepthSense::ColorNode  m_cnode;
    DepthSense::DepthNode  m_dnode;
    typedef std::vector<cv::Point> contour_t;
    typedef std::vector<contour_t> contour_vector_t;
    typedef std::vector<cv::Vec4i> hierarchy_t;
    bool m_bDeviceFound;
    float corX;
    float corY;
    float corX2;
    float corY2;
    int treshold;
    int treshold2;
    contour_t approx_contour;
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
