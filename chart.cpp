#include "chart.h"

Chart::Chart(QLineSeries *series1, QLineSeries *series2, QString title, QString axisXLabel, QString axisYLabel, int rangeX, int rangeY, QWidget *parent)
    : QChartView(parent)
{
    m_chart = new QChart();
    series1->setName("Obiekt 1");
    series2->setName("Obiekt 2");
    m_chart->addSeries(series1);
    m_chart->addSeries(series2);
    m_chart->createDefaultAxes();


    QFont font;
    font.setPixelSize(18);
    m_chart->setTitleFont(font);
    m_chart->setTitleBrush(QBrush(Qt::gray));

    m_chart->setTitle(title);

    QPen pen1(QRgb(0xf40659));
    pen1.setWidth(2);
    series1->setPen(pen1);
    QPen pen2(QRgb(0x0033cc));
    pen2.setWidth(2);
    series2->setPen(pen2);

    m_chart->setAnimationOptions(QChart::AllAnimations);

    m_chart->axisX()->setRange(0,rangeX);
    m_chart->axisX()->setTitleText(axisXLabel);

    m_chart->axisY()->setRange(0,rangeY);
    m_chart->axisY()->setTitleText(axisYLabel);


    this->setChart(m_chart);
}

Chart::~Chart()
{
    delete m_chart;
}
