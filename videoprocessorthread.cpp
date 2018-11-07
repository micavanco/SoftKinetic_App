#include "videoprocessorthread.h"

VideoProcessorThread::VideoProcessorThread(QObject *parent) : QThread(parent)
{

}

static cv::Mat c_mat;

void onNewColorSample2(DepthSense::ColorNode node, DepthSense::ColorNode::NewSampleReceivedData data)
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

void VideoProcessorThread::configureColorNode()
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

void VideoProcessorThread::configureNode(DepthSense::Node node)
{
    if ((node.is<DepthSense::ColorNode>()) && (!m_cnode.isSet()))
    {
        m_cnode = node.as<DepthSense::ColorNode>();
        configureColorNode();
        m_context.registerNode(node);
    }
}

void VideoProcessorThread::run()
   {
     using namespace cv;
    m_context = DepthSense::Context::create("localhost"); // utworzenie połączenia lokalnego, komunikującego się z kamerą za pośrednictwem protokołu IP
    std::vector<DepthSense::Device> da; // wektor przechowujący obiekty będące reprezentacją pojedynczej podłączonej kamery, zawierające wszystkie niezbędne metody
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
    m_cnode.newSampleReceivedEvent().connect(&onNewColorSample2);    // przekazanie referencji do metody onNewColorSample do metody connect w celu powiązania funkcji

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


       emit CameraOn(QString("Włączona"));
       emit inDisplay(
            QPixmap::fromImage(
               QImage(
                 c_mat.data,
                 c_mat.cols,
                 c_mat.rows,
                 c_mat.step,
                 QImage::Format_RGB888)
                     .rgbSwapped()));

     }
     m_context.stopNodes();  // zatrzymanie transmisji
     m_context.quit();   // zatrzymanie wątku
     m_context.unregisterNode(m_cnode); // wyrejestrowanie wątku transmisji z kontekstu połączenia
     m_cnode.unset();
     camThread.requestInterruption(); // wysłanie prośby o zatrzymanie wątku
     camThread.wait();
     cv::destroyAllWindows();
   }
