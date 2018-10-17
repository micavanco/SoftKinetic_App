#include "camerathread.h"

CameraThread::CameraThread(QObject *parent) : QThread(parent)
{

}
/*
    Metoda wykorzystywana do uruchomienia transmisji z kamerą w wątku
*/
void CameraThread::run()
{
        m_context->run();
}
/*
    Metoda przypisująca do wskaźnika zmiennej klasy referencję do kontekstu transmisji z kamerą
*/
void CameraThread::set(DepthSense::Context&c)
{
    m_context=&c;
}
