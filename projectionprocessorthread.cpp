#include "projectionprocessorthread.h"

ProjectionProcessorThread::ProjectionProcessorThread(QObject *parent) : QThread(parent), hueRed(149), saturationRed(126), valueRed(0),
  hueBlue(76), saturationBlue(47), valueBlue(106)
{

}
/*
    Utworzenie statycznych zmiennych w celu umożliwienia bezpośredniego odniesienia się w metodzie onNewDepthSample.
    > cv::Mat       - obiekt z biblioteki OpenCV służący jako n-wymiarowa macierz o jedno lub wielo kanałowym zagęszczeniu
*/
static cv::Mat c_mat;
static cv::Mat d_mat;

/*
    Metoda nasłuchująca, wywoływana przy każdorazowym odbiorze nowych danych (jedna klatka obrazu wideo
    z kamery Softkinetic DS325 w postaci obrazu w przestrzeni kolorów RGB)
*/
void onNewColorSample(DepthSense::ColorNode node, DepthSense::ColorNode::NewSampleReceivedData data)
{
    if(data.colorMap.size() != 0)
    {
        int w, h;
        DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);

        cv::Mat color_mat(h, w, CV_8UC3);

        memcpy(color_mat.data, data.colorMap, w*h * 3);
        c_mat=color_mat;
    }
}

/*
    Metoda nasłuchująca, wywoływana przy każdorazowym odbiorze nowych danych (jedna klatka obrazu wideo
    z kamery Softkinetic DS325 dla trybu pracy głębi

    Możliwość przyszłościowego rozwoju aplikacji o nowe funkcje badania głębi

void onNewDepthSample2(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data)
{
    int w, h;
    DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);
    cv::Mat depth_mat(h, w, CV_16SC1);
    memcpy(depth_mat.data, data.depthMap, w*h*sizeof(int16_t));
    d_mat=depth_mat;
}
*/

/*
    Wywołanie wątku dla transmisji obrazu z kamery
*/
void ProjectionProcessorThread::run()
   {
     using namespace cv;
     // Utworzenie zmiennych reprezentujących zaznaczane prostokąty na obrazach
     trackRect  = cv::Rect();
     trackRect2 = cv::Rect();
     // Zmienne przechowujące obraz w reprezentacji HSV
     Mat hsv, hsv2;
     Mat backProj, backProj2;
     // Zmienna określająca kryteria działania algorytmu MeanShift
     TermCriteria criteria;
     criteria.maxCount = 5;
     criteria.epsilon = 3;
     criteria.type = TermCriteria::EPS;

     // wektor przechowujący obiekty będące reprezentacją pojedynczej podłączonej kamery, zawierające wszystkie niezbędne metody
     std::vector<DepthSense::Device> da;

     // utworzenie połączenia lokalnego, komunikującego się z kamerą za pośrednictwem protokołu IP
     m_context = DepthSense::Context::create("localhost");
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
         // emituj sygnał do GUI jeżeli nie udało się uzyskać żądnych podłączonych obiektów
         emit CameraOff(QString(tr("Kamera odłączona. \nSprawdź połączenie\nz komputerem \ni włącz ponownie.")));
         return;
     }
     emit CameraOn(QString::fromStdString(da[0].getSerialNumber()));
     m_cnode.newSampleReceivedEvent().connect(&onNewColorSample);    // przekazanie referencji do metody onNewColorSample do metody connect w celu powiązania funkcji
     // --------- Możliwość przyszłościowego rozwoju aplikacji o nowe funkcje badania głębi --------
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

       // skopiowanie obrazu przechwyconego z kamery do trzech zmiennych
       cv::Mat disp_mat3 = c_mat.clone();
       cv::Mat disp_mat2 = disp_mat3.clone();
       cv::Mat disp_mat = disp_mat2.clone();

       // sprawdzenie, czy użytkownik zaznaczył prostokąt na pierwszym obrazie w GUI
       if(trackRect.size().area() > 0)
       {
           // kowersja obrazu z przestrzeni barw RGB do reprezentacji HSV
           cvtColor(disp_mat, hsv, CV_BGR2HSV);
           // odfiltrowanie pożądanego przedziału z poszczególnych wartości HSV obrazu
           cv::inRange(hsv, cv::Scalar(hueRed, saturationRed, valueRed),
                       cv::Scalar(hueRed+30, 255, 255), backProj);

         // zablokowanie dostępu do tych samych danych przez dwa rózne wątki
         QMutexLocker locker(&rectMutex);

            // sprawdzenie, czy użytkownik w tym momencie zaznacza szukany obiekt
            if(updateHistogram)
            {
              // narysuj na obrazie prostokąt zaznaczony przez użytkownika
              rectangle(disp_mat, trackRect, Scalar(0, 0, 255));
              // emituj sygnał o zaznaczeniu
              emit newTitle(QString("Obiekt 1"));
              emit checkIfRecord();

              updateHistogram = false;
            }
        // wywołanie implementacji algorytmu MeanShift do śledzenia obiektów
        meanShift(backProj, trackRect, criteria);
        // ponowna konwersja z przestrzeni HSV do RGB w celu wyświetlenia obrazu w GUI
        cvtColor(backProj, disp_mat, CV_GRAY2BGR);
        // rysuj prostokąt na obrazie
        rectangle(disp_mat, trackRect, Scalar(0, 0, 255));
        rectangle(disp_mat3, trackRect, Scalar(0, 0, 255));
        // zapisz współrzędne obiektu do zmiennych
        corX = trackRect.x+trackRect.width/2;
        corY = trackRect.y+trackRect.height/2;
        // wyślij współrzędne obiektu pierwszego do GUI
        emit monitorValuex(QString("%1").arg(corX));
        emit monitorValuey(QString("%1").arg(480-corY));
       }

       // sprawdzenie, czy użytkownik zaznaczył prostokąt na drugim obrazie w GUI
       if(trackRect2.size().area() > 0)
       {
         // kowersja obrazu z przestrzeni barw RGB do reprezentacji HSV
         cvtColor(disp_mat2, hsv2, CV_BGR2HSV);
         // odfiltrowanie pożądanego przedziału z poszczególnych wartości HSV obrazu
         cv::inRange(hsv2, cv::Scalar(hueBlue, saturationBlue, valueBlue), cv::Scalar(hueBlue+30, 255, 255), backProj2);

         // zablokowanie dostępu do tych samych danych przez dwa rózne wątki
         QMutexLocker locker(&rectMutex2);

         // sprawdzenie, czy użytkownik w tym momencie zaznacza szukany obiekt
         if(updateHistogram2)
         {
           // narysuj na obrazie prostokąt zaznaczony przez użytkownika
           rectangle(disp_mat2, trackRect2, Scalar(255, 0, 0));
           // emituj sygnał o zaznaczeniu
           emit newTitle2(QString("Obiekt 2"));
           emit checkIfRecord();

           updateHistogram2 = false;
         }
         // wywołanie implementacji algorytmu MeanShift do śledzenia obiektów
         meanShift(backProj2, trackRect2, criteria);
         // ponowna konwersja z przestrzeni HSV do RGB w celu wyświetlenia obrazu w GUI
         cvtColor(backProj2, disp_mat2, CV_GRAY2BGR);
         // rysuj prostokąt na obrazie
         rectangle(disp_mat2, trackRect2, Scalar(255, 0, 0));
         rectangle(disp_mat3, trackRect2, Scalar(255, 0, 0));
        // zapisz współrzędne obiektu do zmiennych
        corX2=trackRect2.x+trackRect2.width/2;
        corY2=trackRect2.y+trackRect2.height/2;
        // wyślij współrzędne obiektu drugiego do GUI
        emit monitorValue2x(QString("%1").arg(corX2));
        emit monitorValue2y(QString("%1").arg(480-corY2));
       }

       // wyślij informację do GUI o włączonej kamerze
       emit CameraOn(QString("Włączona"));

       // sprawdź, czy macierz zawiera dane
        if(!disp_mat.empty())
       emit newFrame(// wyślij obraz do elementu GUI
            QPixmap::fromImage(
               QImage(
                 disp_mat.data,
                 disp_mat.cols,
                 disp_mat.rows,
                 disp_mat.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));
        if(!disp_mat2.empty())
       emit newFrame2(
            QPixmap::fromImage(
               QImage(
                 disp_mat2.data,
                 disp_mat2.cols,
                 disp_mat2.rows,
                 disp_mat2.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));
        if(!disp_mat3.empty())
       emit newFrame3(
            QPixmap::fromImage(
               QImage(
                 disp_mat3.data,
                 disp_mat3.cols,
                 disp_mat3.rows,
                 disp_mat3.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

     }
     // po otrzymaniu informacji o zakończeniu wątku, emitowany jest sygnał o informacji wyłączonej kamery
     emit CameraOff(QString("Wyłączona"));

     m_context.stopNodes();  // zatrzymanie transmisji
     m_context.quit();       // zatrzymanie wątku
     m_context.unregisterNode(m_cnode); // wyrejestrowanie wątku transmisji z kontekstu połączenia
     //m_context.unregisterNode(m_dnode);  wyrejestrowanie wątku transmisji z kontekstu połączenia
     m_cnode.unset();
     //m_dnode.unset();
     camThread.requestInterruption(); // wysłanie prośby o zatrzymanie wątku
     camThread.wait();
     cv::destroyAllWindows();
}
// --------- Możliwość przyszłościowego rozwoju aplikacji o nowe funkcje badania głębi --------
/*
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
}*/
/*
    Metoda konfigurująca parametry transmisji z kamerą SoftKinetic dla przesyłu obrazu w postaci RGB
*/
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

// --------- Możliwość przyszłościowego rozwoju aplikacji o nowe funkcje badania głębi --------
/*
    Metoda wykorzystywana do konfiguracji trybu pracy kamery - w tym przypadku do konfiguracji trybu głębi

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
*/

/*
    Metoda konfigurująca parametry transmisji z kamerą SoftKinetic
*/
void ProjectionProcessorThread::configureNode(DepthSense::Node node)
{
    /*if ((node.is<DepthSense::DepthNode>()) && (!m_dnode.isSet()))   // jeżeli nowo utworzony wątek transmisji jest typu głębi i wątek nie jest ustawiony
    {
        m_dnode = node.as<DepthSense::DepthNode>(); // ustaw nowy wątek transmisji na tryb głębi
        configureDepthNode();   // konfiguruj szczegóły trybów pracy kamery
        m_context.registerNode(node);   // zarejestruj nowy wątek w kontekście obsługiwanej kamery
    }*/

    if ((node.is<DepthSense::ColorNode>()) && (!m_cnode.isSet()))
    {
        m_cnode = node.as<DepthSense::ColorNode>();
        configureColorNode();
        m_context.registerNode(node);
    }

}
/*
  Metoda przypisująca wymiary zaznaczonego przez użytkownika prostokąta na obrazie z kamery dla obiektu pierwszego
*/
void ProjectionProcessorThread::setTrackRect(QRect rect)
{
  // zablokowanie dostępu do tych samych danych przez dwa rózne wątki
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
/*
  Metoda przypisująca wymiary zaznaczonego przez użytkownika prostokąta na obrazie z kamery dla obiektu drugiego
*/
void ProjectionProcessorThread::setTrackRect2(QRect rect)
{
  // zablokowanie dostępu do tych samych danych przez dwa rózne wątki
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
