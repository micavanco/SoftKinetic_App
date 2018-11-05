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
signals:

public slots:

private:
    QChart *m_chart;
    int     m_range1;
    int     m_range2;
    QLineSeries   *m_series1;
    QLineSeries   *m_series2;
};

#endif // FFTCHART_H
