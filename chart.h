#ifndef CHART_H
#define CHART_H

#include <QGraphicsView>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

class Chart : public QChartView
{
    Q_OBJECT
public:
    explicit Chart(QLineSeries *series1, QLineSeries *series2, QString title, QString axisXLabel, QString axisYLabel, int rangeX, int rangeY,
                   bool hasMultiFiles, QLineSeries *series3 = nullptr, QLineSeries *series4 = nullptr, QString file1Name = "", QString file2Name = "",
                   QWidget *parent = nullptr);
    ~Chart();
signals:

public slots:
    void showWindowCoord(QPointF point, bool state); // metoda odbierająca dane z sygnału najechania kursorem na wykres w postaci parametrów funkcji

private:
    QChart *m_chart;
    bool    m_hasMultiFiles;
    int     m_countRotation;            // zmienna ograniczająca oddalanie widoku wykresu
    bool    m_isPressed;                // sprawdzenie czy jest wciśnięty klawisz
    QPoint  m_pos;                      // pozycja kursora po wciśnięciu lewego przycisku myszy
    int     m_rangeX;                   // maksymalny zakres osi x
    int     m_rangeY;                   // maksymalny zakres osi y
    QGraphicsTextItem *m_coord;         // okno wyświetlające położenie x i y nad którym znajduje się kursor
    QPen   *m_pen1;
    QPen   *m_pen2;
    QPen   *m_pen3;
    QPen   *m_pen4;
    QLineSeries *m_series1;
    QLineSeries *m_series2;
    QLineSeries *m_series3;
    QLineSeries *m_series4;

    void wheelEvent(QWheelEvent *event);        // metoda obsługi zdarzeń pokrętła myszki
    void mousePressEvent(QMouseEvent *event);   // metoda obsługi zdarzeń naciśnięcia klawisza myszki
    void mouseReleaseEvent(QMouseEvent *event); // metoda obsługi zdarzeń puszczenia klawisza myszki
    void mouseMoveEvent(QMouseEvent *event);    // metoda obsługi zdarzeń przesuwania myszki
};

#endif // CHART_H
