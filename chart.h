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

private:
    QChart *m_chart;
    int     m_countRotation;    // zmienna ograniczająca oddalanie widoku wykresu
    bool    m_isPressed;        // sprawdzenie czy jest wciśnięty klawisz
    QPoint  m_pos;              // pozycja kursora po wciśnięciu lewego przycisku myszy
    int     m_rangeX;
    int     m_rangeY;
    int     m_minX;             // zmienna śledząca zmiany w widoku w osi X minimum
    int     m_minY;             // zmienna śledząca zmiany w widoku w osi Y minimum
    int     m_maxX;             // zmienna śledząca zmiany w widoku w osi X maximum
    int     m_maxY;             // zmienna śledząca zmiany w widoku w osi Y maximum

    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // CHART_H
