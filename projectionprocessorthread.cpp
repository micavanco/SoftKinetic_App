#include "projectionprocessorthread.h"

ProjectionProcessorThread::ProjectionProcessorThread(QObject *parent) : QThread(parent), color(0)
{

}

void ProjectionProcessorThread::run()
   {
     using namespace cv;
     VideoCapture camera(0);
     float hrange[] = {0, 180};
     const float* ranges[] = {hrange};
     int bins = 24;
     int channels[] = {0}; // the first and the only channel
     int histSize[] = { bins }; // number of bins
     int fromto[] = {0, 0};
     Mat inFrame;
     Mat hsv, hue, histogram;
     Mat backProj;
     TermCriteria criteria;
     criteria.maxCount = 5;
     criteria.epsilon = 3;
     criteria.type = TermCriteria::EPS;
     trackRect=cv::Rect();

     while(camera.isOpened() && !isInterruptionRequested())
     {
       camera >> inFrame;
       if(inFrame.empty())
       {
           emit CameraOff(QString("Wyłączona. \nSprawdź połączenie z komputerem\n i włącz ponownie."));
           return;
       }

       for(int i=0; i<histogram.rows; i++)
           {
           if(i==color || (i==color+1) || (i==color+2) || (i==color-1)) // filter
                   histogram.at<float>(i,0) = 255;
                 else
                   histogram.at<float>(i,0) = 0;
           }

       /*for(int j=0;j<backProj.rows;j++)
       {
           for(int i=0;i<backProj.rows;i++)
           {
               if(backProj.at<float>(j,i)==255)
               {

               }
           }
       }*/

       if(trackRect.size().area() > 0)
       {
         QMutexLocker locker(&rectMutex);

         cvtColor(inFrame, hsv, CV_BGR2HSV);
         hue.create(hsv.size(), hsv.depth());
         mixChannels(&hsv, 1, &hue, 1, fromto, 1);

         if(updateHistogram)
         {
           Mat roi(hue, trackRect);

           calcHist(&roi, 1, channels, Mat(), histogram, 1, histSize, ranges);

           normalize(histogram,
               histogram,
               0,
               255,
               NORM_MINMAX);





           updateHistogram = false;
         }

         calcBackProject(&hue,
           1,
           0,
           histogram,
           backProj,
           ranges);

         TermCriteria criteria;
         criteria.maxCount = 5;
         criteria.epsilon = 3;
         criteria.type = TermCriteria::EPS;
         RotatedRect rotRec = CamShift(backProj, trackRect, criteria);

         cvtColor(backProj, backProj, CV_GRAY2BGR);

         ellipse(inFrame, rotRec, Scalar(0,255,0), 1);
         rectangle(inFrame, trackRect, Scalar(0,0,255), 1);
       }
       emit CameraOn(QString("Włączona"));

       emit newFrame(
            QPixmap::fromImage(
               QImage(
                 inFrame.data,
                 inFrame.cols,
                 inFrame.rows,
                 inFrame.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

       emit newFrame2(
            QPixmap::fromImage(
               QImage(
                 backProj.data,
                 backProj.cols,
                 backProj.rows,
                 backProj.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

     }
     emit CameraOff(QString("Wyłączona"));
   }

void ProjectionProcessorThread::onNewColorValue(int c)
{
    color=c;
}

void ProjectionProcessorThread::setTrackRect(QRect rect)
{
  QMutexLocker locker(&rectMutex);
  if((rect.width()>2) && (rect.height()>2))
  {
     trackRect.x = rect.left();
     trackRect.y = rect.top();
     trackRect.width = rect.width();
     trackRect.height = rect.height();
     updateHistogram = true;
  }
}

