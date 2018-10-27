#include "videoprocessorthread.h"

VideoProcessorThread::VideoProcessorThread(QObject *parent) : QThread(parent)
{

}

void VideoProcessorThread::run()
   {
     using namespace cv;
     VideoCapture *camera = new VideoCapture(1);
     Mat inFrame;
     while(camera->isOpened() && !isInterruptionRequested())
     {
       *camera >> inFrame;
       if(inFrame.empty())
       {
           emit CameraOff(QString("Wyłączona. \nSprawdź połączenie z komputerem\n i włącz ponownie."));
           return;
       }


       emit CameraOn(QString("Włączona"));
       emit inDisplay(
            QPixmap::fromImage(
               QImage(
                 inFrame.data,
                 inFrame.cols,
                 inFrame.rows,
                 inFrame.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

     }
     camera->release();
     delete camera;
     emit CameraOff(QString("Wyłączona"));
   }
