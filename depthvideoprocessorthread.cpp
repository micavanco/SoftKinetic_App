#include "depthvideoprocessorthread.h"

DepthVideoProcessorThread::DepthVideoProcessorThread(QObject *parent) : QThread(parent)
{

}
/*
    Utworzenie statycznych zmiennych w celu umożliwienia bezpośredniego odniesienia się w metodzie onNewDepthSample.
    > cv::Mat       - obiekt z biblioteki OpenCV służący jako n-wymiarowa macierz o jedno lub wielo kanałowym zagęszczeniu
    > corX/corY     - zmienne do zampisu kordynacji kursowa na ekranie
    > getDepth      - zmienna boolowska służąca do informowania o tym czy mysz znajduje się w obrębie obrazu transmitowanego z kamery
    > cameraDepth   - zmienna określająca odległość obiektu od kamery
*/
static cv::Mat d_mat;
static int corX=0;
static int corY=0;
static bool getDepth=false;
static float cameraDepth=0.0;
/*
    Metoda nasłuchująca, wywoływana przy każdorazowym odbiorze nowych danych (jedna klatka obrazu wideo
    z kamery Softkinetic DS325
*/
void onNewDepthSample(DepthSense::DepthNode node, DepthSense::DepthNode::NewSampleReceivedData data)
{
    int w, h;
    DepthSense::FrameFormat_toResolution(data.captureConfiguration.frameFormat, &w, &h);
    static cv::Mat depth_mat(h, w, CV_16SC1);
    memcpy(depth_mat.data, data.depthMap, w*h*sizeof(int16_t));
    d_mat=depth_mat;
}
/*
    Metoda dziedziczona z klasy nadrzędnej.
    Odpowiedzialna za utworzene nowego wątku (procesu działającego równolegle do reszty aplikacji)
    nie blokującego wykonywania pozostałej częsci aplikacji.
*/
void DepthVideoProcessorThread::run()
{

    std::vector<DepthSense::Device> da; // wektor przechowujący obiekty będące reprezentacją pojedynczej podłączonej kamery, zawierające wszystkie niezbędne metody
    int32_t w, h;

    m_context = DepthSense::Context::create("localhost"); // utworzenie połączenia lokalnego, komunikującego się z kamerą za pośrednictwem protokołu IP
    da = m_context.getDevices();                          // wywołanie metody w celu pobrania listy dostępnych urządzeń

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
    m_dnode.newSampleReceivedEvent().connect(&onNewDepthSample);    // przekazanie referencji do metody onNewDepthSample do metody connect w celu powiązania funkcji

    CameraThread camThread;    /*utworzenie nowego wątku*/          // przyjmującej odebrane dane z transmisji kamery, przekazywane jako parametry funkcji
    camThread.set(m_context);   // przekazanie do metody wątku ustawiającego kontekst, czyli klasę reprezentującą sesję aplikacji (połączenie klienta przez TCP/IP)
    if(m_bDeviceFound)          // jeżeli znaleziono urządzenie
    {
        m_context.startNodes(); // rozpocznij transmisję danych z kamery
        camThread.start();      // rozpocznij nowy wątek transmisji danych z kamery
    }
    using namespace cv;
    while(d_mat.empty())continue;   // wykonywanie pętli do momentu odebrania pierwszych danych ( jednej klatki z transmitowanego obrazu, w postaci macierzy )
    while(!isInterruptionRequested()) // wykonuj do póki nie zostanie przesłane żądanie zatrzymania wątku
    {
    cv::Mat disp_mat = ModDepthForDisplay(d_mat);   // wywołanie metody konwertującej otrzymane wartości w macierzy do macierzy zawierającej adekwatne do rzeczywistości wartości
     if(disp_mat.empty()) /* jeżeli otrzymana macierz jest pusta */  // oraz w celu zobrazowania, do wartości maksymalnego zakresu przestrzeni barw RGB
      {
          m_dnode.unset();  // zakończ połączenie z kamerą i wyjdź z pętli
          m_bDeviceFound = false;
          emit CameraOff(QString(tr("Odłączona. \nSprawdź połączenie\nz komputerem \ni włącz ponownie.")));
          return;
      }

      emit CameraOn(QString(tr("Włączona"))); // emituj sygnał informujący o włączonej kamerze
      emit inDisplay(
           QPixmap::fromImage(
              QImage(               // wysłanie sygnału z danymi jednej klatki wideo otrzymanej z kamery w postaci obiektu mapy bitowej opakowane wcześniej w obiekt QImage
                disp_mat.data,
                disp_mat.cols,
                disp_mat.rows,
                disp_mat.step,
                QImage::Format_RGB888)
                    .rgbSwapped()));

    }
    emit CameraOff(QString("Wyłączona"));

    m_context.stopNodes();  // zatrzymanie transmisji
    m_context.quit();   // zatrzymanie wątku
    m_context.unregisterNode(m_dnode); // wyrejestrowanie wątku transmisji z kontekstu połączenia
    m_dnode.unset();
    camThread.requestInterruption(); // wysłanie prośby o zatrzymanie wątku
    camThread.wait();
    cv::destroyAllWindows();
}
/*
    Metoda wykorzystywana do konwersji przechwyconych wartości z transmisji z kamerą do macierzy zawierającej adekwatne do rzeczywistości wartości
    oraz w celu zobrazowania, do wartości maksymalnego zakresu przestrzeni barw RGB
*/
cv::Mat DepthVideoProcessorThread::ModDepthForDisplay(const cv::Mat &mat)
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

        if(getDepth && !fmat.empty())   // jeżeli kursor jest w obrębie obrazu (wartość getDepth) i macierz nie jest pusta to
        {
            cameraDepth=(fmat.at<float>(corY, corX)-beta)/alpha; // przekonwertuj wartość na danej pozycji z powrotem do jednostki milimetra
            if(cameraDepth==1000)emit newValue(QString("Błąd odczytu"));    // jeżeli wartość wykracza poza zakres pracy kamery to wyslij informację o błędzie
            else emit newValue(QString("%1 mm").arg(cameraDepth));  // emituj sygnał z wartością odległości obiektu od kamery
        }


        cv::Mat bmat;
        fmat.convertTo(bmat, CV_8U);    // konwertuj nowo utworzoną macierz do formatu 8 bitowego typu bez znaku o zakresie od 0 do 255

        cv::Mat cmat;
        cv::cvtColor(bmat, cmat, CV_GRAY2BGR); // konwertuj macierz z jednej przestrzeni kolorów skali szarości do BGR i przekopiuj do macierzy cmat
        cv::applyColorMap(cmat, cmat, m_colorType); // zastosuj daną mapę kolorów

        return cmat;
}
/*
    Metoda wykorzystywana do konfiguracji utworzonego wątku transmisji
*/
void DepthVideoProcessorThread::configureNode(DepthSense::Node node)
{
    if ((node.is<DepthSense::DepthNode>()) && (!m_dnode.isSet()))   // jeżeli nowo utworzony wątek transmisji jest typu głębi i wątek nie jest ustawiony
    {
        m_dnode = node.as<DepthSense::DepthNode>(); // ustaw nowy wątek transmisji na tryb głębi
        configureDepthNode();   // konfiguruj szczegóły trybów pracy kamery
        m_context.registerNode(node);   // zarejestruj nowy wątek w kontekście obsługiwanej kamery
    }

}
/*
    Metoda wykorzystywana do konfiguracji trybu pracy kamery - w tym przypadku do konfiguracji trybu głębi
*/
void DepthVideoProcessorThread::configureDepthNode()
{
    DepthSense::DepthNode::Configuration config = m_dnode.getConfiguration(); // pobierz konfiguracje z kontekstu połączenia z kamerą
    config.frameFormat = m_frameFormat; // ustaw format obrazu
    config.framerate = m_frameRate; // ustaw częstotliwość próbkowania
    config.mode = m_cameraMode; // ustaw tryb pracy
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
/*
    Metoda wykorzystywana do zapisu wartości ustawień kamery do zmiennych klasy
*/
void DepthVideoProcessorThread::setConfiguration(int framerate, DepthSense::FrameFormat frameformat, DepthSense::DepthNode::CameraMode cameraMode, int colorType)
{
    m_frameFormat=frameformat;
    m_frameRate=framerate;
    m_cameraMode=cameraMode;
    m_colorType=colorType;
}
/*
    Slot wykorzystywany do ustawienia wartości boolowskiej określającej lokalizację kursora
*/
void DepthVideoProcessorThread::onGettingDepth()
{
    getDepth=true;
}

void DepthVideoProcessorThread::offGettingDepth()
{
    getDepth=false;
}
/*
    Metoda do przypisywania wartości koordynacji do zmiennych statycznych
*/
void DepthVideoProcessorThread::setDepthValue(int x, int y)
{
    corX=x;
    corY=y;
}
/*
    Metoda zwracająca wartość odległości obiektu od kamery w formacie zmiennoprzecinkowym
*/
float DepthVideoProcessorThread::getDepthValue()
{
    return cameraDepth;
}
