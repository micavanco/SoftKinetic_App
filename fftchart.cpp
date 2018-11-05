#include "fftchart.h"

FftChart::FftChart(double *series1, double *series2, int range1, int range2,QString title, QString file1Name, QString file2Name,
                   QString whichObject, QString whichAxis, QWidget *parent)
    : QChartView(parent), m_countRotation(0), m_isPressed(false)
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
    for(int i=0; i < m_range1/2; i++)
    {
        // obliczenie częstotliwości
        f = i*smallerRange/m_range1;
        // obliczenie modułu z wartości zespolonej dla serii 1
        double a = m_fftvalues1[i];
        double b = 0.0;
        if(i>0 && i<range1/2)
            b = m_fftvalues1[m_range1/2+i];

        double c = qSqrt(a*a+b*b) / m_range1;
        c = 0.15*qLn(c);
        if(c < 0)c*=-1;
        m_series1->append(f, c);
        if((c>yMax)&&(i>0))yMax=c;
        // obliczenie modułu z wartości zespolonej dla serii 2
        a = m_fftvalues2[i];
        b = 0.0;
        if(i>0 && i<range1/2)
            b = m_fftvalues2[m_range1/2+i];

        c = qSqrt(a*a+b*b) / m_range1;
        c = 0.15*qLn(c);
        if(c < 0)c*=-1;
        m_series2->append(f, c);
        if((c>yMax)&&(i>0))yMax=c;
    }

    delete [] m_fftvalues1;
    delete [] m_fftvalues2;

    m_chart = new QChart(); // utworzenie nowego obiektu wykresu

    m_pen1 = new QPen(QBrush(QRgb(0xf40659)), 3); // ustawienie koloru w postaci szesnastkowej i grubości linii
    m_series1->setPen(*m_pen1);
    m_pen2 = new QPen(QBrush(QRgb(0x0033cc)), 3);
    m_series2->setPen(*m_pen2);

    // ustaw nazwy serii
    m_series1->setName("Obiekt "+whichObject+" z pliku "+file1Name);
    m_series2->setName("Obiekt "+whichObject+" z pliku "+file2Name);

    // dodaj serię do wykresu
    m_chart->addSeries(m_series1);
    m_chart->addSeries(m_series2);

    // połączenie sygnałów nacjechania kursorem na wykres z metodą klasy oraz przekazywanie wartości w postaci parametrów funkcji
    connect(m_series1, &QLineSeries::hovered, this, &FftChart::showWindowCoord);
    connect(m_series2, &QLineSeries::hovered, this, &FftChart::showWindowCoord);

    // utworznie obiektu modyfikującego czcionkę
    QFont font;
    font.setPixelSize(18);
    m_chart->setTitleFont(font);    // przypisanie obiektu czcionki do tytułu wykresu
    m_chart->setTitleBrush(QBrush(Qt::gray)); // ustawienie kolory tytułu wykresu

    m_chart->setTitle(title); // przypisanie łańcucha znaków do tytułu wykresu
    m_chart->createDefaultAxes(); // utworzenie domyślnych osi
    m_chart->setAnimationOptions(QChart::AllAnimations); // aktywacja animacji wszystkich elementów

    m_chart->axisX()->setRange(0, m_range1/2);   // ustawienie zakresu osi x
    m_chart->axisX()->setTitleText("Częstotliwość [Hz]"); // ustawienie tytułu osi x

    m_chart->axisY()->setRange(0, yMax);   // ustawienie zakresu osi y
    m_chart->axisY()->setTitleText(whichAxis); // ustawienie tytułu osi y

    m_coord = new QGraphicsTextItem(m_chart); // utworzenie obiektu wyświetlającego koordynacje kursora na wykresie i przypisanie do wykresu
    m_coord->setZValue(11); // przeniesienie okna w górę hierarchii wyświetlanych elementów aby wykres nie przesłaniał


    this->setChart(m_chart);    // przypisanie obiektu wykresu do sceny, czyli tej klasy
    this->setCursor(Qt::OpenHandCursor); // ustawienie kursora otwartej dłoni

}

FftChart::~FftChart()
{
    delete m_pen1;
    delete m_pen2;
    delete m_coord; // usuwanie obiektu wyświetlającego koordynacje kursora
    delete m_series1;
    delete m_series2;
    delete m_chart;
}

void FftChart::showWindowCoord(QPointF point, bool state)
{
    if(state) // sprawdzenie stanu położenia kursora, czy znajduje się nad wykresem
    {
        if(point.x() > 64*0.75) // sprawdzenie czy kursor znajduje się w 75% długości osi x jeżeli tak to...
            m_coord->setPos(m_chart->mapToPosition(point).x()-150, m_chart->mapToPosition(point).y()-40);// zmieniamy położenie okna z koordynacjami na lewą stronę
        else    //                                                                                          położenia kursora
            m_coord->setPos(m_chart->mapToPosition(point).x()+15, m_chart->mapToPosition(point).y()-40);// jeżeli nie to na prawą stronę
        m_coord->show(); // wyświetlenie okna
        this->setCursor(Qt::CrossCursor); // ustawienie kursora krzyża
    }else // jeżeli kursor nie znajduje się nad wykresem to ...
    {
        this->setCursor(Qt::OpenHandCursor); // ustawienie kursora otwartej dłoni
        m_coord->hide(); // ukrycie okna
    }
}

void FftChart::wheelEvent(QWheelEvent *event)
{
    if(event->angleDelta().y() > 0)// jeżeli pokrętło jest kręcona od uzytkownika, to przyjmuje wartości dodatnie
    {
        m_chart->zoomIn(); // przybliżenie wykresu
        m_countRotation++; // inkrementacja zmiennej przechowującej ilość wykonanych obrotów przybliżających
        m_pen1->setWidth(m_countRotation+2);
        m_pen2->setWidth(m_countRotation+2);
        m_series1->setPen(*m_pen1);
        m_series2->setPen(*m_pen2);
    }
    else if(m_countRotation) // jeżeli zmienna przechowuje liczby dodatnie to...
    {
        m_chart->zoomOut(); // oddal wykres
        m_countRotation--;  // dekrementacja liczb przybliżeń
        m_pen1->setWidth(m_countRotation+2);
        m_pen2->setWidth(m_countRotation+2);
        m_series1->setPen(*m_pen1);
        m_series2->setPen(*m_pen2);
    }else // jeżeli warość przybliżeń wynosi zero to...
    {
        m_chart->axisX()->setRange(0, m_range1/2); // ustaw zakres osi x na domyślny
        m_chart->axisY()->setRange(0, 1); // ustaw zakres osi y na domyślny
    }
    return QChartView::wheelEvent(event);
}
// metoda obsługi zdarzeń naciśnięcia klawisza myszki
void FftChart::mousePressEvent(QMouseEvent *event)
{
    if(!m_isPressed) // jeżeli klawisz nie był wcześniej wciśnięty to...
    {
        m_isPressed = true;     // ustaw wartość informującą o wciśnięciu klawisza
        m_pos = event->pos();   // pobierz pozycję kursora
        this->setCursor(Qt::ClosedHandCursor); // ustawienie kursora zamkniętej dłoni
    }
    QChartView::mousePressEvent(event);
}
// metoda obsługi zdarzeń puszczenia klawisza myszki
void FftChart::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed) // jeżeli klawisz myszki był wciśnięty to...
    {
        m_chart->setAnimationOptions(QChart::SeriesAnimations); // ustaw animację serii wykresu
        m_isPressed = false; // ustaw wartość informującą o puszczeniu klawisza
        this->setCursor(Qt::OpenHandCursor); // ustawienie kursora otwartej dłoni
    }
    QChartView::mouseReleaseEvent(event);
}
// metoda obsługi zdarzeń przesuwania myszki
void FftChart::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) // jeżeli klawisz myszki był wciśnięty to...
    {
        QPoint temp = event->pos();     // przechowaj aktualną pozycję kursora w zmiennej tymczasowej
        int deltaX = m_pos.x()-temp.x();        // obliczenie o ile przesunął się kursor od momentu przyciśnięcia klawisza myszki
        int deltaY = (m_pos.y()-temp.y())/-1;   // to samo dla osi y
        m_chart->scroll(deltaX, deltaY);        // przesunięcie zakresu osi o wartości tego przesunięcia
        m_pos = temp;           // przypisanie aktualnej pozycji do zmiennej klasy
    }
    m_coord->setHtml("<div style='background-color: #ffffff; font-size: 15px;'>"+QString("f: %1 Hz \nA: %2")
                     .arg(m_chart->mapToValue(event->pos()).x())
                     .arg(m_chart->mapToValue(event->pos()).y())+"</div>");     // przypisanie koordynacji kursora do okna z informacją o położeniu
    QChartView::mouseMoveEvent(event);
}

