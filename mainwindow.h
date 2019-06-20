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
#include <QScatterSeries>

#include <QMessageBox>

#include "wavfile.h"
#include "ECGLogic/edfdecoder.h"

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

    void on_actionExit_triggered();

    void on_platoSlider_sliderMoved(int position);

    void on_checkBox_2_toggled(bool checked);

    void on_platoSlider_sliderReleased();

    void on_actionSave_triggered();

    void on_action_triggered();

    void on_action_2_triggered();

    void on_horizontalSlider_sliderMoved(int position);

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    QString fileName;
    bool isECGFile;

    // Данные для ЭКГ
    EDF edf;
    // График первого канала ЭКГ
    QChart *ecg1chart;
    QLineSeries *ecg1LineSeries;
    // График второго канала ЭКГ
    QChart *ecg2chart;
    QLineSeries *ecg2LineSeries;
    // График третьего канала ЭКГ
    QChart *ecg3chart;
    QLineSeries *ecg3LineSeries;
    // Скатерограмма R пиков
    QScatterSeries rPeaks;


    // Данные для АУДИО
    WavFile wavFile;
    // Вывод исходного графика звука
    QChartView *chartView1;
    QChart *chart1;
    QLineSeries *chartSeries1;

    // Вывод абсолютных значений амплитуды
    QChartView *chartView2;
    QChart *chart2;
    QLineSeries *chartSeries2;

    // Интегральный график
    QLineSeries *integralSeries;
    // График с точками пульса
    QScatterSeries *pulses;

    // График спектра
    QChartView *spectrumChartView;
    QChart *spectrumChart;
    QLineSeries *spectrumSeries;
};

#endif // MAINWINDOW_H
