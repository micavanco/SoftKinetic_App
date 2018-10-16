#include "depthvideoprocessorthread.h"

DepthVideoProcessorThread::DepthVideoProcessorThread(QObject *parent) : QThread(parent)
{

}

static cv::Mat d_mat;
static int corX=0;
static int corY=0;
static bool getDepth=false;
static float cameraDepth=0.0;
void onNewDepthSample(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data)
{
    int w, h;
    DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);
    static cv::Mat depth_mat(h, w, CV_16SC1);
    memcpy(depth_mat.data, data.depthMap, w*h*sizeof(int16_t));
    d_mat=depth_mat;
}
void DepthVideoProcessorThread::run()
{

    std::vector<DepthSense::Device> da;
    int32_t w, h;

    m_context = DepthSense::Context::create("localhost");
    da = m_context.getDevices();
    if (da.size() >= 1)
        {
        emit CameraOn(QString::fromStdString(da[0].getSerialNumber()));
            m_bDeviceFound = true;
            std::vector<DepthSense::Node> na = da[0].getNodes();
            for (int n = 0; n<(int)na.size();n++)
                configureNode(na[n]);
        }
    else
    {
        emit CameraOff(QString(tr("Kamera odłączona. \nSprawdź połączenie\nz komputerem \ni włącz ponownie.")));
        return;
    }
    emit CameraOn(QString::fromStdString(da[0].getSerialNumber()));
    m_dnode.newSampleReceivedEvent().connect(&onNewDepthSample);
    CameraThread camThread;
    camThread.set(m_context);
    if(m_bDeviceFound)
    {
        m_context.startNodes();
        camThread.start();
    }
    using namespace cv;
    while(d_mat.empty())continue;
    while(!isInterruptionRequested())
    {
    cv::Mat disp_mat = ModDepthForDisplay(d_mat);
     if(disp_mat.empty())
      {
          m_dnode.unset();
          m_bDeviceFound = false;
          emit CameraOff(QString(tr("Odłączona. \nSprawdź połączenie\nz komputerem \ni włącz ponownie.")));
          return;
      }

      emit CameraOn(QString(tr("Włączona")));
      emit inDisplay(
           QPixmap::fromImage(
              QImage(
                disp_mat.data,
                disp_mat.cols,
                disp_mat.rows,
                disp_mat.step,
                QImage::Format_RGB888)
                    .rgbSwapped()));

    }
    emit CameraOff(QString("Wyłączona"));

    m_context.stopNodes();
    m_context.quit();
    m_context.unregisterNode(m_dnode);
    m_dnode.unset();
    camThread.requestInterruption();
    camThread.wait();
    cv::destroyAllWindows();
}

cv::Mat DepthVideoProcessorThread::ModDepthForDisplay(const cv::Mat &mat)
{
        const float depth_near = 0;
        const float depth_far = 1000;

        const float alpha = 255.0 / (depth_far - depth_near);
        const float beta = -depth_near*alpha;

        cv::Mat fmat;
        mat.convertTo(fmat, CV_32F);


        for (int r = 0;r<mat.rows; ++r)
        {
            for (int c = 0; c<mat.cols; ++c)
            {
                float v = fmat.at<float>(r, c)*alpha + beta;

                if (v>255) v = 255;
                if (v<0)   v = 0;

                fmat.at<float>(r, c) = v;
            }
        }

        if(getDepth && !fmat.empty())
        {
            cameraDepth=(fmat.at<float>(corY, corX)-beta)/alpha;
            if(cameraDepth==1000)emit newValue(QString("Błąd odczytu"));
            else emit newValue(QString("%1 mm").arg(cameraDepth));
        }


        cv::Mat bmat;
        fmat.convertTo(bmat, CV_8U);

        cv::Mat cmat;
        cv::cvtColor(bmat, cmat, CV_GRAY2BGR);
        cv::applyColorMap(cmat, cmat, m_colorType);

        return cmat;
}


void DepthVideoProcessorThread::configureNode(DepthSense::Node node)
{
    if ((node.is<DepthSense::DepthNode>()) && (!m_dnode.isSet()))
    {
        m_dnode = node.as<DepthSense::DepthNode>();
        configureDepthNode();
        m_context.registerNode(node);
    }

}

void DepthVideoProcessorThread::configureDepthNode()
{
    DepthSense::DepthNode::Configuration config = m_dnode.getConfiguration();
    config.frameFormat = m_frameFormat;
    config.framerate = m_frameRate;
    config.mode = m_cameraMode;
    config.saturation = true;

    m_dnode.setEnableDepthMap(true);
    m_dnode.setEnableConfidenceMap(true);

    try
    {
        m_context.requestControl(m_dnode, 0);

        m_dnode.setConfiguration(config);
    }
    catch (DepthSense::ArgumentException& e)
    {

    }
    catch (DepthSense::UnauthorizedAccessException& e)
    {

    }
    catch (DepthSense::IOException& e)
    {

    }
    catch (DepthSense::InvalidOperationException& e)
    {

    }
    catch (DepthSense::ConfigurationException& e)
    {

    }
    catch (DepthSense::StreamingException& e)
    {

    }
    catch (DepthSense::TimeoutException&)
    {

    }
}

void DepthVideoProcessorThread::setConfiguration(int framerate, DepthSense::FrameFormat frameformat, DepthSense::DepthNode::CameraMode cameraMode, int colorType)
{
    m_frameFormat=frameformat;
    m_frameRate=framerate;
    m_cameraMode=cameraMode;
    m_colorType=colorType;
}

void DepthVideoProcessorThread::onGettingDepth()
{
    getDepth=true;
}

void DepthVideoProcessorThread::offGettingDepth()
{
    getDepth=false;
}

void DepthVideoProcessorThread::setDepthValue(int x, int y)
{
    corX=x;
    corY=y;
}

float DepthVideoProcessorThread::getDepthValue()
{
    return cameraDepth;
}
