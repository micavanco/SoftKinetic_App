#include "projectionprocessorthread.h"

ProjectionProcessorThread::ProjectionProcessorThread(QObject *parent) : QThread(parent)
{

}

static cv::Mat c_mat;
static cv::Mat d_mat;

// New color sample event handler
void onNewColorSample(DepthSense::ColorNode node, DepthSense::ColorNode::NewSampleReceivedData data)
{
    int w, h;
    DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);

    cv::Mat color_mat(h, w, CV_8UC3);
    memcpy(color_mat.data, data.colorMap, w*h * 3);
    c_mat=color_mat;
}

/*
    Metoda nasłuchująca, wywoływana przy każdorazowym odbiorze nowych danych (jedna klatka obrazu wideo
    z kamery Softkinetic DS325
*/
void onNewDepthSample2(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data)
{
    int w, h;
    DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);
    cv::Mat depth_mat(h, w, CV_16SC1);
    memcpy(depth_mat.data, data.depthMap, w*h*sizeof(int16_t));
    d_mat=depth_mat;
}

void ProjectionProcessorThread::run()
   {
     using namespace cv;
     trackRect=cv::Rect();
     trackRect2=cv::Rect();
     Mat hsv, hsv2;
     Mat backProj, backProj2;
     TermCriteria criteria;
     criteria.maxCount = 5;
     criteria.epsilon = 3;
     criteria.type = TermCriteria::EPS;

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
     //m_dnode.newSampleReceivedEvent().connect(&onNewDepthSample2);    // przekazanie referencji do metody onNewColorSample do metody connect w celu powiązania funkcji

     CameraThread camThread;    /*utworzenie nowego wątku*/          // przyjmującej odebrane dane z transmisji kamery, przekazywane jako parametry funkcji
     camThread.set(m_context);   // przekazanie do metody wątku ustawiającego kontekst, czyli klasę reprezentującą sesję aplikacji (połączenie klienta przez TCP/IP)
     if(m_bDeviceFound)          // jeżeli znaleziono urządzenie
     {
         m_context.startNodes(); // rozpocznij transmisję danych z kamery
         camThread.start();      // rozpocznij nowy wątek transmisji danych z kamery
     }
/*  -------------------- Główna pętla -------------------------------------------------    */
     while(c_mat.empty())continue;   // wykonywanie pętli do momentu odebrania pierwszych danych ( jednej klatki z transmitowanego obrazu, w postaci macierzy )
     //while(d_mat.empty())continue;   // wykonywanie pętli do momentu odebrania pierwszych danych ( jednej klatki z transmitowanego obrazu, w postaci macierzy )
     while(!isInterruptionRequested())
     {
       if(c_mat.empty())
       {
           m_cnode.unset();  // zakończ połączenie z kamerą i wyjdź z pętli
           m_bDeviceFound = false;
           emit CameraOff(QString("Wyłączona. \nSprawdź połączenie z komputerem\n i włącz ponownie."));
           return;
       }

       cv::Mat disp_mat = c_mat.clone();
       cv::Mat disp_mat2=disp_mat.clone();

       if(trackRect.size().area() > 0)
       {
           cvtColor(disp_mat, hsv, CV_BGR2HSV);
           cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), backProj);


         QMutexLocker locker(&rectMutex);

            if(updateHistogram)
            {
              rectangle(disp_mat, trackRect, Scalar(0, 0, 255));
              emit newTitle(QString("Obiekt 1"));
              emit checkIfRecord();

              updateHistogram = false;
            }
        CamShift(backProj, trackRect, criteria);
        cvtColor(backProj, disp_mat, CV_GRAY2BGR);
        rectangle(disp_mat, trackRect, Scalar(0, 0, 255));
        corX=trackRect.x+trackRect.width/2;
        corY=trackRect.y+trackRect.height/2;
        emit monitorValue(QString("x: %1, y: %2").arg(corX).arg(corY));

       }

       if(trackRect2.size().area() > 0)
       {
         cvtColor(disp_mat2, hsv2, CV_BGR2HSV);
         cv::inRange(hsv2, cv::Scalar(100, 150, 0), cv::Scalar(140, 255, 255), backProj2);

         QMutexLocker locker(&rectMutex2);
         if(updateHistogram2)
         {
           rectangle(disp_mat2, trackRect2, Scalar(255, 0, 0));
           emit newTitle2(QString("Obiekt 2"));
           emit checkIfRecord();

           updateHistogram2 = false;
         }
         CamShift(backProj2, trackRect2, criteria);
         cvtColor(backProj2, disp_mat2, CV_GRAY2BGR);
         rectangle(disp_mat2, trackRect2, Scalar(255, 0, 0));
        corX2=trackRect2.x+trackRect2.width/2;
        corY2=trackRect2.y+trackRect2.height/2;
        emit monitorValue2(QString("x: %1, y: %2").arg(corX2).arg(corY2));
       }
       emit CameraOn(QString("Włączona"));

       emit newFrame(
            QPixmap::fromImage(
               QImage(
                 disp_mat.data,
                 disp_mat.cols,
                 disp_mat.rows,
                 disp_mat.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

       emit newFrame2(
            QPixmap::fromImage(
               QImage(
                 disp_mat2.data,
                 disp_mat2.cols,
                 disp_mat2.rows,
                 disp_mat2.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

     }
     emit CameraOff(QString("Wyłączona"));

     m_context.stopNodes();  // zatrzymanie transmisji
     m_context.quit();   // zatrzymanie wątku
     m_context.unregisterNode(m_cnode); // wyrejestrowanie wątku transmisji z kontekstu połączenia
     m_context.unregisterNode(m_dnode); // wyrejestrowanie wątku transmisji z kontekstu połączenia
     m_cnode.unset();
     m_dnode.unset();
     camThread.requestInterruption(); // wysłanie prośby o zatrzymanie wątku
     camThread.wait();
     cv::destroyAllWindows();
}

cv::Mat ProjectionProcessorThread::ModDepthForDisplay(const cv::Mat &mat)
{
        const float depth_near = 0;
        const float depth_far = 1000;   // maksymalny zakres pola odczytu kamery SoftKinetic

        const float alpha = 255.0 / (depth_far - depth_near); // kanał alpha obrazu przeznaczony jest na informowanie o poziomie nasycenia kolorów składowych RGB
        const float beta = -depth_near*alpha;

        cv::Mat fmat;               // utworzenie nowej macierzy z biblioteki OpenCV
        mat.convertTo(fmat, CV_32F);// konwersja macierzy do formatu 32 bitowego typu zmiennoprzecinkowego zawierającego się w zakresie od 0 do 1


        for (int r = 0;r<mat.rows; ++r) // iteracyjny zapis wartości ze starej macierzy do nowej w formacie zawierającym się w przedziale od 0 do 1 typu float
        {
            for (int c = 0; c<mat.cols; ++c)
            {
                float v = fmat.at<float>(r, c)*alpha + beta;

                if (v>255) v = 255; // jeżeli liczba wykracza poza zakres przestrzeni barw RGB to jest zaokrągla do maksimum
                if (v<0)   v = 0;   // i analogicznie przy wartościach minimalnych

                fmat.at<float>(r, c) = v;
            }
        }

        if(!fmat.empty())   // jeżeli kursor jest w obrębie obrazu (wartość getDepth) i macierz nie jest pusta to
        {
            float cameraDepth=(fmat.at<float>(corY, corX)-beta)/alpha; // przekonwertuj wartość na danej pozycji z powrotem do jednostki milimetra
            float cameraDepth2=(fmat.at<float>(corY2, corX2)-beta)/alpha; // przekonwertuj wartość na danej pozycji z powrotem do jednostki milimetra
            if(cameraDepth==1000)emit monitorDepthValue(QString("Błąd odczytu"));    // jeżeli wartość wykracza poza zakres pracy kamery to wyslij informację o błędzie
            else emit monitorDepthValue(QString("%1 mm").arg(cameraDepth)); // emituj sygnał z wartością odległości obiektu od kamery
            if(cameraDepth2==1000)emit monitorDepthValue2(QString("Błąd odczytu"));    // jeżeli wartość wykracza poza zakres pracy kamery to wyslij informację o błędzie
            else emit monitorDepthValue2(QString("%1 mm").arg(cameraDepth2)); // emituj sygnał z wartością odległości obiektu od kamery
        }

        cv::Mat bmat;
        fmat.convertTo(bmat, CV_8U);    // konwertuj nowo utworzoną macierz do formatu 8 bitowego typu bez znaku o zakresie od 0 do 255

        cv::Mat cmat;
        cv::cvtColor(bmat, cmat, CV_GRAY2BGR); // konwertuj macierz z jednej przestrzeni kolorów skali szarości do BGR i przekopiuj do macierzy cmat
        cv::applyColorMap(cmat, cmat, 2); // zastosuj daną mapę kolorów

        return cmat;
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

/*
    Metoda wykorzystywana do konfiguracji trybu pracy kamery - w tym przypadku do konfiguracji trybu głębi
*/
void ProjectionProcessorThread::configureDepthNode()
{
    DepthSense::DepthNode::Configuration config = m_dnode.getConfiguration(); // pobierz konfiguracje z kontekstu połączenia z kamerą
    config.frameFormat = DepthSense::FRAME_FORMAT_QVGA; // ustaw format obrazu
    config.framerate = 25; // ustaw częstotliwość próbkowania
    config.mode = DepthSense::DepthNode::CameraMode::CAMERA_MODE_CLOSE_MODE; // ustaw tryb pracy
    config.saturation = true;   // ustaw nasycenie kolorów

    m_dnode.setEnableDepthMap(true);        // ustaw mapę głębi
    m_dnode.setEnableConfidenceMap(true);   // ustaw mapę gęstości

    try
    {
        m_context.requestControl(m_dnode, 0);   // wysłanie prośby o przejęcie kontroli i zapis danych

        m_dnode.setConfiguration(config);   //  zapisz nowo utworzone ustawienia do zmiennych klasy
    }
    catch (DepthSense::ArgumentException& e)    // przechwytywanie wyjątków gdy wystąpi błąd
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
    if ((node.is<DepthSense::DepthNode>()) && (!m_dnode.isSet()))   // jeżeli nowo utworzony wątek transmisji jest typu głębi i wątek nie jest ustawiony
    {
        m_dnode = node.as<DepthSense::DepthNode>(); // ustaw nowy wątek transmisji na tryb głębi
        configureDepthNode();   // konfiguruj szczegóły trybów pracy kamery
        m_context.registerNode(node);   // zarejestruj nowy wątek w kontekście obsługiwanej kamery
    }

    if ((node.is<DepthSense::ColorNode>()) && (!m_cnode.isSet()))
    {
        m_cnode = node.as<DepthSense::ColorNode>();
        configureColorNode();
        m_context.registerNode(node);
    }

}

void ProjectionProcessorThread::setTrackRect(QRect rect)
{
  QMutexLocker locker(&rectMutex);
  if((rect.width()>2) && (rect.height()>2))
  {
     if(rect.left()<0)trackRect.x=1;
     else if(rect.left()>640)trackRect.x=640;
     else trackRect.x = rect.left();

     if(rect.top()<0)trackRect.y=1;
     else if(rect.top()>480)trackRect.y=480;
     else trackRect.y = rect.top();


     if(rect.left()+rect.width()>640)trackRect.width=640-rect.left()-1;
     else trackRect.width = rect.width();

     if(rect.height()+rect.top()>480)trackRect.height=480-rect.top()-1;
     else trackRect.height = rect.height();

     updateHistogram = true;
  }
}

void ProjectionProcessorThread::setTrackRect2(QRect rect)
{
  QMutexLocker locker(&rectMutex2);
  if((rect.width()>2) && (rect.height()>2))
  {
     if(rect.left()<0)trackRect2.x=1;
     else if(rect.left()>640)trackRect2.x=640;
     else trackRect2.x = rect.left();

     if(rect.top()<0)trackRect2.y=1;
     else if(rect.top()>480)trackRect2.y=480;
     else trackRect2.y = rect.top();


     if(rect.left()+rect.width()>640)trackRect2.width=640-rect.left()-1;
     else trackRect2.width = rect.width();

     if(rect.height()+rect.top()>480)trackRect2.height=480-rect.top()-1;
     else trackRect2.height = rect.height();

     updateHistogram2 = true;
  }
}
