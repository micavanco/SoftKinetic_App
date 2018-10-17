#include "projectionprocessorthread.h"

ProjectionProcessorThread::ProjectionProcessorThread(QObject *parent) : QThread(parent), color(0)
{

}

static cv::Mat c_mat;

// New color sample event handler
void onNewColorSample(DepthSense::ColorNode node, DepthSense::ColorNode::NewSampleReceivedData data)
{
    int w, h;
    DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);

    cv::Mat color_mat(h, w, CV_8UC3);
    memcpy(color_mat.data, data.colorMap, w*h * 3);
    c_mat=color_mat;
}

void ProjectionProcessorThread::run()
   {
     using namespace cv;
     float hrange[] = {0, 180};
     const float* ranges[] = {hrange};
     int bins = 24;
     int channels[] = {0}; // the first and the only channel
     int histSize[] = { bins }; // number of bins
     int fromto[] = {0, 0};
     Mat hsv, hue, histogram;
     Mat backProj;
     TermCriteria criteria;
     criteria.maxCount = 5;
     criteria.epsilon = 3;
     criteria.type = TermCriteria::EPS;
     trackRect=cv::Rect();

     std::vector<DepthSense::Device> da; // wektor przechowujący obiekty będące reprezentacją pojedynczej podłączonej kamery, zawierające wszystkie niezbędne metody

     m_context = DepthSense::Context::create("localhost"); // utworzenie połączenia lokalnego, komunikującego się z kamerą za pośrednictwem protokołu IP
     da = m_context.getDevices();

     if (da.size() >= 1)
         {
             emit CameraOn(QString::fromStdString(da[0].getSerialNumber())); // emituj sygnał z numerem seryjnym podłączonej kamery
             m_bDeviceFound = true;
             std::vector<DepthSense::Node> na = da[0].getNodes(); // wywołanie metody w celu pobrania właściwości określającej źródło nadchodzących danych wysłanych przez kamerę
             for (int n = 0; n<(int)na.size();n++)
                 configureNode(na[n]);            // metoda konfigurująca właściwość (tzw. węzeł) określającą połączenie z daną kamerą
         }
     else
     {
         emit CameraOff(QString(tr("Kamera odłączona. \nSprawdź połączenie\nz komputerem \ni włącz ponownie.")));
         return;
     }
     emit CameraOn(QString::fromStdString(da[0].getSerialNumber()));
     m_cnode.newSampleReceivedEvent().connect(&onNewColorSample);    // przekazanie referencji do metody onNewColorSample do metody connect w celu powiązania funkcji

     CameraThread camThread;    /*utworzenie nowego wątku*/          // przyjmującej odebrane dane z transmisji kamery, przekazywane jako parametry funkcji
     camThread.set(m_context);   // przekazanie do metody wątku ustawiającego kontekst, czyli klasę reprezentującą sesję aplikacji (połączenie klienta przez TCP/IP)
     if(m_bDeviceFound)          // jeżeli znaleziono urządzenie
     {
         m_context.startNodes(); // rozpocznij transmisję danych z kamery
         camThread.start();      // rozpocznij nowy wątek transmisji danych z kamery
     }
/*  -------------------- Główna pętla -------------------------------------------------    */
     while(c_mat.empty())continue;   // wykonywanie pętli do momentu odebrania pierwszych danych ( jednej klatki z transmitowanego obrazu, w postaci macierzy )
     while(!isInterruptionRequested())
     {

       if(c_mat.empty())
       {
           m_cnode.unset();  // zakończ połączenie z kamerą i wyjdź z pętli
           m_bDeviceFound = false;
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

         cvtColor(c_mat, hsv, CV_BGR2HSV);
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

         ellipse(c_mat, rotRec, Scalar(0,255,0), 1);
         rectangle(c_mat, trackRect, Scalar(0,0,255), 1);
       }
       emit CameraOn(QString("Włączona"));

       emit newFrame(
            QPixmap::fromImage(
               QImage(
                 c_mat.data,
                 c_mat.cols,
                 c_mat.rows,
                 c_mat.step,
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

     m_context.stopNodes();  // zatrzymanie transmisji
     m_context.quit();   // zatrzymanie wątku
     m_context.unregisterNode(m_cnode); // wyrejestrowanie wątku transmisji z kontekstu połączenia
     m_cnode.unset();
     camThread.requestInterruption(); // wysłanie prośby o zatrzymanie wątku
     camThread.wait();
     cv::destroyAllWindows();
}

void ProjectionProcessorThread::configureColorNode()
{

    DepthSense::ColorNode::Configuration config = m_cnode.getConfiguration();
    config.frameFormat = DepthSense::FRAME_FORMAT_VGA;
    config.compression = DepthSense::COMPRESSION_TYPE_MJPEG;
    config.powerLineFrequency = DepthSense::POWER_LINE_FREQUENCY_50HZ;
    config.framerate = 25;

    m_cnode.setEnableColorMap(true);

    try
    {
        m_context.requestControl(m_cnode, 0);

        m_cnode.setConfiguration(config);
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

void ProjectionProcessorThread::configureNode(DepthSense::Node node)
{
    /*if ((node.is<DepthSense::DepthNode>()) && (!m_dnode.isSet()))
    {
        m_dnode = node.as<DepthSense::DepthNode>();
        configureDepthNode();
        m_context.registerNode(node);
    }*/

    if ((node.is<DepthSense::ColorNode>()) && (!m_cnode.isSet()))
    {
        m_cnode = node.as<DepthSense::ColorNode>();
        configureColorNode();
        m_context.registerNode(node);
    }

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

