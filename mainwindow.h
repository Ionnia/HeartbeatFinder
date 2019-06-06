#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileDialog>
#include <QFile>

#include <QVector>
#include <QString>

#include <QDebug>

#include <QChart>
#include <QChartView>
#include <QLineSeries>

#include "wav_file.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_porogSlider_sliderMoved(int position);

    void on_porogSlider_sliderReleased();

    void on_checkBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    QString fileName;

    WavFile wavFile;

    // Вывод исходного графика звука
    QChartView *chartView1;
    QChart *chart1;
    QLineSeries *chartSeries1;

    // Вывод абсолютных значений амплитуды
    QChartView *chartView2;
    QChart *chart2;
    QLineSeries *chartSeries2;

    QLineSeries *integralSeries;
};

#endif // MAINWINDOW_H
