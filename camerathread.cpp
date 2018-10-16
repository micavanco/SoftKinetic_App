#include "camerathread.h"

CameraThread::CameraThread(QObject *parent) : QThread(parent)
{

}

void CameraThread::run()
{
        m_context->run();
}

void CameraThread::set(DepthSense::Context&c)
{
    m_context=&c;
}
