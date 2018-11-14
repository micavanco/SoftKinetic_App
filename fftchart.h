#ifndef FFTCHART_H
#define FFTCHART_H

#include <QGraphicsView>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtMath>
#include "ffft/FFTReal.h"

QT_CHARTS_USE_NAMESPACE

class FftChart : public QChartView
{
    Q_OBJECT
public:
    explicit FftChart(double *series1, double *series2, int range1, int range2, QString title, QString file1Name, QString file2Name,
                      QString whichObject, QString whichAxis, QWidget *parent = nullptr);
    ~FftChart();

    float getMaxDiff() const {return m_maxDiff;} // metoda zwracająca m_maxDiff
    QString resultsMessage() const {return m_resultsMessage;} // metoda zwracająca informację odnośnie analizy dwóch sygnałów
signals:

public slots:
    void showWindowCoord(QPointF point, bool state); // metoda odbierająca dane z sygnału najechania kursorem na wykres w postaci parametrów funkcji

private:
    QChart *m_chart;
    int     m_countRotation;            // zmienna ograniczająca oddalanie widoku wykresu
    bool    m_isPressed;                // sprawdzenie czy jest wciśnięty klawisz
    QPoint  m_pos;                      // pozycja kursora po wciśnięciu lewego przycisku myszy
    QGraphicsTextItem *m_coord;         // okno wyświetlające położenie x i y nad którym znajduje się kursor
    int     m_range1;
    int     m_range2;
    QLineSeries   *m_series1;
    QLineSeries   *m_series2;
    QPen   *m_pen1;
    QPen   *m_pen2;
    float   m_maxDiff;                  // zmienna okreslająca wartość maksymalnej róznicy pomiędzy dwoma sygnałami
    int     m_maxHz;                    // częstotliwość przy maksymalnej różnicy
    QString m_resultsMessage;           // informacja odnośnie analizy dwóch sygnałów
    int     m_countMatch;               // zmienna zawierająca ilość dopasowanych próbek
    double  m_percentage;               // na ile procent przebiegi są podobne


    void wheelEvent(QWheelEvent *event);        // metoda obsługi zdarzeń pokrętła myszki
    void mousePressEvent(QMouseEvent *event);   // metoda obsługi zdarzeń naciśnięcia klawisza myszki
    void mouseReleaseEvent(QMouseEvent *event); // metoda obsługi zdarzeń puszczenia klawisza myszki
    void mouseMoveEvent(QMouseEvent *event);    // metoda obsługi zdarzeń przesuwania myszki
};

#endif // FFTCHART_H
