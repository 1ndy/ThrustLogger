#include "dynamicchart.h"

DynamicChart::DynamicChart(QWidget* parent) : QWidget(parent)
{
    _chart = new QtCharts::QChart();
    _xAxis = new QtCharts::QValueAxis(_chart);
    _yAxis = new QtCharts::QValueAxis(_chart);
    _ql = new QtCharts::QLineSeries(_chart);

    _chart->addAxis(_xAxis, Qt::AlignBottom);
    _chart->addAxis(_yAxis, Qt::AlignLeft);

    _chart->legend()->hide();

    _chart->addSeries(_ql);
    _ql->attachAxis(_xAxis);
    _ql->attachAxis(_yAxis);
    _ql->setUseOpenGL(true);

    _xAxis->setRange(0, 10);
    _yAxis->setRange(-1.5, 30);

}

DynamicChart::~DynamicChart()
{

}


QtCharts::QChart* DynamicChart::getChart() {
    return _chart;
}

QList<QPointF> DynamicChart::convertStlToQ(std::queue<std::pair<float,float>> points) {
    QList<QPointF> new_points;
    int i = 0;
    while(!points.empty()) {
        new_points.append(QPointF(points.front().first, points.front().second));
        //std::cout << "converted point at index" << i << "(" << points.front().first << "," << points.front().second << ")" << std::endl;
        points.pop();
        i++;
    }
    return new_points;
}

void DynamicChart::appendNewData(std::queue<std::pair<float,float>> points) {
    if(points.size() > 0) {
        _ql->append(convertStlToQ(points));
    }
    /*
    while(!points.empty()) {
        float x = points.front().first;
        float y = points.front().second;
        _ql->append(x,y);
        points.pop();
    }
    */
}
