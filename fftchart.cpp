#include "fftchart.h"

FftChart::FftChart(double *series1, double *series2, int range1, int range2,QString title, QString file1Name, QString file2Name,
                   QString whichObject, QString whichAxis, QWidget *parent)
    : QChartView(parent)
{
    int smallerRange;
    // sprawdzenie zakresu czasu przebiegu oraz dopasowanie do fft, czyli do wartości potegi 2
    if(range1 > range2)
    {
        smallerRange = range2;
        if(range2 >= 64 && range2 < 128)
            m_range2 = m_range1 = 64;
        else if(range2 >= 128 && range2 < 256)
            m_range2 = m_range1 = 128;
        else if(range2 >= 256 && range2 < 512)
            m_range2 = m_range1 = 256;
        else if(range2 >= 512 && range2 < 1024)
            m_range2 = m_range1 = 512;
        else if(range2 >= 1024 && range2 < 2048)
            m_range2 = m_range1 = 1024;
        else if(range2 >= 2048 && range2 < 4096)
            m_range2 = m_range1 = 2048;
        else if(range2 >= 2048 && range2 < 4096)
            m_range2 = m_range1 = 2048;
        else if(range2 >= 4096 && range2 < 8192)
            m_range2 = m_range1 = 4096;
    }else
    {
        smallerRange = range1;
        if(range1 >= 64 && range1 < 128)
            m_range2 = m_range1 = 64;
        else if(range1 >= 128 && range1 < 256)
            m_range2 = m_range1 = 128;
        else if(range1 >= 256 && range1 < 512)
            m_range2 = m_range1 = 256;
        else if(range1 >= 512 && range1 < 1024)
            m_range2 = m_range1 = 512;
        else if(range1 >= 1024 && range1 < 2048)
            m_range2 = m_range1 = 1024;
        else if(range1 >= 2048 && range1 < 4096)
            m_range2 = m_range1 = 2048;
        else if(range1 >= 2048 && range1 < 4096)
            m_range2 = m_range1 = 2048;
        else if(range1 >= 4096 && range1 < 8192)
            m_range2 = m_range1 = 4096;
    }
    // utworzenie dwóch obiektów klasy reprezentującej transformatę Fourier'a
    ffft::FFTReal <double> fft_object1 (m_range1);
    ffft::FFTReal <double> fft_object2 (m_range1);

    // utworzenie nowych tablic z danymi na transformatę
    double* m_fftvalues1 = new double [m_range1];
    double* m_fftvalues2 = new double [m_range1];

    // transformata Fourier'a
    fft_object1.do_fft(m_fftvalues1, series1);
    fft_object2.do_fft(m_fftvalues2, series2);

    // przypisanie do wskaźników na tablice wskaźnik na null
    series1 = nullptr;
    series2 = nullptr;

    // utworzenie nowych obiektów serii danych
    m_series1 = new QLineSeries();
    m_series2 = new QLineSeries();

    // początkowa maksymalna wartość wzmocnienia na osi y
    float yMax = 1;
    // zmienna określająca częstotliwość
    double f = 1;
    // utworzenie serii danych do wykresu
    for(int i=0; i <= 64; i++)
    {
        f = i*smallerRange/m_range1;
        // obliczenie modułu z wartości zespolonej dla serii 1
        double a = m_fftvalues1[i];
        double b = 0.0;
        if(i>0 && i<range1/2)
            b = m_fftvalues1[m_range1/2+1];

        double c = qSqrt(a*a+b*b) / m_range1;
        c = 0.15*qLn(c);
        m_series1->append(f, c);
        if((c>yMax)&&(i>0))yMax=c;
        // obliczenie modułu z wartości zespolonej dla serii 2
        a = m_fftvalues2[i];
        b = 0.0;
        if(i>0 && i<range1/2)
            b = m_fftvalues2[m_range1/2+1];

        c = qSqrt(a*a+b*b) / m_range1;
        c = 0.15*qLn(c);
        m_series2->append(f, c);
        if((c>yMax)&&(i>0))yMax=c;
    }

    delete [] m_fftvalues1;
    delete [] m_fftvalues2;

    m_chart = new QChart(); // utworzenie nowego obiektu wykresu

    QPen pen1(QBrush(QRgb(0xf40659)), 3); // ustawienie koloru w postaci szesnastkowej i grubości linii
    m_series1->setPen(pen1);
    QPen pen2(QBrush(QRgb(0x0033cc)), 3);
    m_series2->setPen(pen2);

    // ustaw nazwy serii
    m_series1->setName("Obiekt "+whichObject+" z pliku "+file1Name);
    m_series2->setName("Obiekt "+whichObject+" z pliku "+file2Name);

    // dodaj serię do wykresu
    m_chart->addSeries(m_series1);
    m_chart->addSeries(m_series2);

    // utworznie obiektu modyfikującego czcionkę
    QFont font;
    font.setPixelSize(18);
    m_chart->setTitleFont(font);    // przypisanie obiektu czcionki do tytułu wykresu
    m_chart->setTitleBrush(QBrush(Qt::gray)); // ustawienie kolory tytułu wykresu

    m_chart->setTitle(title); // przypisanie łańcucha znaków do tytułu wykresu
    m_chart->createDefaultAxes(); // utworzenie domyślnych osi
    m_chart->setAnimationOptions(QChart::AllAnimations); // aktywacja animacji wszystkich elementów

    m_chart->axisX()->setRange(0, 64);   // ustawienie zakresu osi x
    m_chart->axisX()->setTitleText("Częstotliwość [Hz]"); // ustawienie tytułu osi x

    m_chart->axisY()->setRange(0, yMax);   // ustawienie zakresu osi y
    m_chart->axisY()->setTitleText(whichAxis); // ustawienie tytułu osi y


    this->setChart(m_chart);    // przypisanie obiektu wykresu do sceny, czyli tej klasy
    this->setCursor(Qt::OpenHandCursor); // ustawienie kursora otwartej dłoni

}

FftChart::~FftChart()
{
    delete m_series1;
    delete m_series2;
    delete m_chart;
}
