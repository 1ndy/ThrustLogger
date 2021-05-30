#ifndef DYNAMICCHART_H
#define DYNAMICCHART_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <queue>
#include <iostream>

using namespace QtCharts;

class DynamicChart : QWidget
{

    Q_OBJECT

public:
    DynamicChart(QWidget* parent = nullptr);
    ~DynamicChart();

    QList<QPointF> convertStlToQ(std::queue<std::pair<float,float>> points);
    void appendNewData(std::queue<std::pair<float,float>> points);
    QtCharts::QChart* getChart();

private:
    QtCharts::QChart* _chart;
    QtCharts::QLineSeries* _ql;
    QtCharts::QValueAxis *_xAxis, *_yAxis;
    float xMin, xMax, yMin, yMax;

};

#endif // DYNAMICCHART_H
