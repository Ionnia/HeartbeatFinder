#ifndef LOGIC_H
#define LOGIC_H

#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>

QT_CHARTS_USE_NAMESPACE

void bufToQLineSeries(QLineSeries *series, int16_t *buf, uint32_t bufSize, uint32_t spacing, uint32_t ticksPerSecond = 44100){
    series->clear();
    int64_t averageValue = 0;      // Среднее значение в диапазоне spacing
    for(uint32_t i = 0; i < bufSize; ++i){
        averageValue += buf[i];
        if(i % spacing == 0){
            series->append((double)i/ticksPerSecond, buf[i]);
            averageValue = 0;
        }
    }
}

void absBufToQLineSeries(QLineSeries *series, int16_t *buf, uint32_t bufSize, uint32_t spacing, uint32_t ticksPerSecond = 44100){
    series->clear();
    int64_t averageValue = 0;
    for(uint32_t i = 0; i < bufSize; ++i){
        averageValue += abs(buf[i]);
        if(i % spacing == 0){
            series->append((double)i/ticksPerSecond, averageValue);
            averageValue = 0;
        }
    }
}

void absBufToQLineSeriesWithPorog(QLineSeries *series, QLineSeries *integralSeries, int16_t *buf, uint32_t bufSize, uint32_t spacing, uint32_t porog,
                                  uint32_t ticksPerSecond = 44100){
    series->clear();
    integralSeries->clear();
    uint32_t integrationWindow = 10;
    uint32_t integrator = 0;
    int64_t averageValue = 0;
    for(uint32_t i = 0; i < bufSize; ++i){
        averageValue += abs(buf[i]);
        if(i % spacing == 0){
            integralSeries->append((double)i/ticksPerSecond, integrator);
            if((uint32_t)abs(buf[i]) < porog){
                series->append((double)i/ticksPerSecond, 0);
                integrator += 0;
            } else {
                series->append((double)i/ticksPerSecond, averageValue);
                integrator += abs(averageValue);
            }
            averageValue = 0;
        }
    }
}

void setChart(QChartView *chartView, QChart *chart, QLineSeries *series){
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();

    chartView->setChart(chart);
}

void calculatePulsePoints(QScatterSeries *scatter, QLineSeries *integralSeries, int widthOfPlato, uint32_t ticksPerSecond = 44100){
    scatter->clear();
    int intSeriesSize = integralSeries->count();
    int sameValueCount = 0;
    bool insidePulseInterval = false;
    double intervalStart = 0;
    double intervalEnd = 0;
    double pulseTime = 0;
    for(int i = 1; i < intSeriesSize; ++i){
        // Если текущее значение равно предыдущему
        if(integralSeries->at(i).y() == integralSeries->at(i-1).y()){
            ++sameValueCount;
        } else {    // В противном случае
            if(!insidePulseInterval){   // Если мы были вне интервала удара сердца, то мы только что зашли в него
                insidePulseInterval = true;
                intervalStart = integralSeries->at(i).x();
            }
            sameValueCount = 0;
        }
        // Если значения повторяются больше определённого кол-ва раз, то мы находимся между ударами сердца
        if(sameValueCount > widthOfPlato && insidePulseInterval){
             intervalEnd = integralSeries->at(i - widthOfPlato).x();
             insidePulseInterval = false;
             pulseTime = (intervalEnd + intervalStart)/2;
             scatter->append(pulseTime, 0);
        }
    }
    if(insidePulseInterval && scatter->count() > 0){
        intervalEnd = integralSeries->at(intSeriesSize-1).x();
        pulseTime = (intervalEnd + intervalStart)/2;
        scatter->append(pulseTime, 0);
    }
}

#endif // LOGIC_H
