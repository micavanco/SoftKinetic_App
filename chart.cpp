#include "chart.h"

Chart::Chart(QLineSeries *series1, QLineSeries *series2, QString title, QString axisXLabel, QString axisYLabel, int rangeX, int rangeY,
             bool hasMultiFiles, QLineSeries *series3, QLineSeries *series4, QString file1Name, QString file2Name, QWidget *parent)
    : QChartView(parent), m_countRotation(0), m_isPressed(false), m_rangeX(rangeX), m_rangeY(rangeY), m_minX(0), m_minY(0), m_maxX(rangeX), m_maxY(rangeY)
{
    m_chart = new QChart();
    if(hasMultiFiles)
    {
        series1->setName("Obiekt 1 z pliku "+file1Name);
        series2->setName("Obiekt 2 z pliku "+file1Name);
        series3->setName("Obiekt 1 z pliku "+file2Name);
        series4->setName("Obiekt 2 z pliku "+file2Name);
        m_chart->addSeries(series1);
        m_chart->addSeries(series2);
        m_chart->addSeries(series3);
        m_chart->addSeries(series4);

        QPen pen1(QBrush(QRgb(0xf40659)), 2);
        series1->setPen(pen1);
        QPen pen2(QBrush(QRgb(0x0033cc)), 2);
        series2->setPen(pen2);
        QPen pen3(QBrush(QRgb(0xe6564c)), 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        series3->setPen(pen3);
        QPen pen4(QBrush(QRgb(0x2fbdf5)), 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
        series4->setPen(pen4);
    }else
    {
        series1->setName("Obiekt 1");
        series2->setName("Obiekt 2");
        m_chart->addSeries(series1);
        m_chart->addSeries(series2);


        QPen pen1(QRgb(0xf40659));
        pen1.setWidth(2);
        series1->setPen(pen1);
        QPen pen2(QRgb(0x0033cc));
        pen2.setWidth(2);
        series2->setPen(pen2);
    }

    QFont font;
    font.setPixelSize(18);
    m_chart->setTitleFont(font);
    m_chart->setTitleBrush(QBrush(Qt::gray));

    m_chart->setTitle(title);
    m_chart->createDefaultAxes();
    m_chart->setAnimationOptions(QChart::AllAnimations);

    m_chart->axisX()->setRange(0,rangeX);
    m_chart->axisX()->setTitleText(axisXLabel);

    m_chart->axisY()->setRange(0,rangeY);
    m_chart->axisY()->setTitleText(axisYLabel);


    this->setChart(m_chart);
    this->setCursor(Qt::OpenHandCursor);
}

Chart::~Chart()
{
    delete m_chart;
}

void Chart::wheelEvent(QWheelEvent *event)
{
    if(event->angleDelta().y() > 0)
    {
        chart()->zoomIn();
        m_countRotation++;
    }
    else if(m_countRotation)
    {
        chart()->zoomOut();
        m_countRotation--;
    }
    return QChartView::wheelEvent(event);
}

void Chart::mousePressEvent(QMouseEvent *event)
{
    if(!m_isPressed)
    {
        m_isPressed = true;
        m_pos = event->pos();
        this->setCursor(Qt::ClosedHandCursor);
    }
    QChartView::mousePressEvent(event);
}

void Chart::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPressed)
    {
        chart()->setAnimationOptions(QChart::SeriesAnimations);
        m_isPressed = false;
        this->setCursor(Qt::OpenHandCursor);
    }
    QChartView::mouseReleaseEvent(event);
}

void Chart::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed)
    {
        QPoint temp = event->pos();
        int deltaX = m_pos.x()-temp.x();
        int deltaY = (m_pos.y()-temp.y())/-1;
        if(m_minX+deltaX < 0) deltaX = -m_minX;
        //else if(m_maxX+deltaX > m_rangeX) deltaX = m_rangeX-m_maxX;
        if(m_minY+deltaY < 0) deltaY = -m_minY;
        //else if(m_maxY+deltaY > m_rangeY) deltaY = m_rangeY-m_maxY;
        chart()->scroll(deltaX, deltaY);
        m_pos = temp;
        m_minX += deltaX;
        m_minY += deltaY;
       // m_maxX += deltaX;
        //m_maxY += deltaY;
    }
    QChartView::mouseMoveEvent(event);
}

