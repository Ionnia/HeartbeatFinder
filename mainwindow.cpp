#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <SDL2/SDL_audio.h>
#include "logic.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Инициализация графиков
    chartSeries1 = new QLineSeries();
    chartSeries2 = new QLineSeries();
    chart1 = new QChart();
    chart2 = new QChart();

    integralSeries = new QLineSeries();
    pulses = new QScatterSeries();
    pulses->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    pulses->setMarkerSize(10.0);
    pulses->setColor(QColor(255, 0, 0));
}

MainWindow::~MainWindow()
{
    delete chartView1;
    delete chartView2;
    delete chart1;
    delete chart2;
    delete chartSeries1;
    delete chartSeries2;
    delete integralSeries;
    delete pulses;

    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    fileName = QFileDialog::getOpenFileName();
    if(!fileName.isEmpty()){
        QFile file(fileName);
        if(wavFile.openWAV(fileName)){
            // Считываем данные
            bufToQLineSeries(chartSeries1, wavFile.wav_buf_16bit, wavFile.wav_len/2, 10);
            absBufToQLineSeriesWithPorog(chartSeries2, integralSeries, wavFile.wav_buf_16bit, wavFile.wav_len/2, 10, 0);
            calculatePulsePoints(pulses, integralSeries, ui->platoSlider->sliderPosition());

            setChart(ui->graphicsView, chart1, chartSeries1);
            setChart(ui->graphicsView_2, chart2, chartSeries2);

            ui->porogSlider->setSliderPosition(0);
            ui->checkBox->setEnabled(true);
            ui->checkBox_2->setEnabled(true);
        } else {
            fileName = "";
        }
    }
    if(fileName.isEmpty()){
        QString messageBoxText = "По какой-то причине не удалось открыть файл.\n";
        messageBoxText += "Программа принимает только звуковые файлы формата .wav";
        QMessageBox::warning(this, "Не удалось открыть файл", messageBoxText);
    }

}

void MainWindow::on_porogSlider_sliderMoved(int position)
{
    ui->label->setText("Порог амплитуды: " + QString::number(position));
}

void MainWindow::on_porogSlider_sliderReleased()
{
    if(!fileName.isEmpty()){
        if(ui->checkBox->isChecked()){
            chart2->removeSeries(integralSeries);
        } else {
            chart2->removeSeries(chartSeries2);
        }
        absBufToQLineSeriesWithPorog(chartSeries2, integralSeries, wavFile.wav_buf_16bit, wavFile.wav_len/2, 10, ui->porogSlider->sliderPosition());
        if(ui->checkBox->isChecked()){
            chart2->addSeries(integralSeries);
        } else {
            chart2->addSeries(chartSeries2);
        }
        if(ui->checkBox_2->isChecked()){
            chart2->removeSeries(pulses);
        }
        calculatePulsePoints(pulses, integralSeries, ui->platoSlider->sliderPosition());
        if(ui->checkBox_2->isChecked()){
            chart2->addSeries(pulses);
            ui->label_3->setText("Количество биений сердца: " + QString::number(pulses->count()));
        }
        chart2->createDefaultAxes();
    }
}

void MainWindow::on_checkBox_toggled(bool checked)
{
    if(checked){
        chart2->removeSeries(chartSeries2);
        chart2->addSeries(integralSeries);
    } else {
        chart2->removeSeries(integralSeries);
        chart2->addSeries(chartSeries2);
    }
    chart2->createDefaultAxes();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_platoSlider_sliderMoved(int position)
{
    ui->label_2->setText("Ширина плато: " + QString::number(position));
}


void MainWindow::on_platoSlider_sliderReleased()
{
    if(!fileName.isEmpty()){
        calculatePulsePoints(pulses, integralSeries, ui->platoSlider->sliderPosition());
        if(ui->checkBox_2->isChecked()){
            chart2->removeSeries(pulses);
            chart2->addSeries(pulses);
            ui->label_3->setText("Количество биений сердца: " + QString::number(pulses->count()));
        }
        chart2->createDefaultAxes();
    }
}

void MainWindow::on_checkBox_2_toggled(bool checked)
{
    if(checked){
        chart2->addSeries(pulses);
        ui->label_3->setText("Количество биений сердца: " + QString::number(pulses->count()));
    } else {
        chart2->removeSeries(pulses);
        ui->label_3->setText("Количество биений сердца: 0");
    }
    chart2->createDefaultAxes();
}

void MainWindow::on_actionSave_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName();
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly);

    double ticksPerSec = wavFile.wav_spec.freq;
    double timeOfPulseInSeconds = 0;
    QString writeLine = "";

    for(int i = 0; i < pulses->count(); ++i){
        timeOfPulseInSeconds = pulses->at(i).x();
        writeLine = QString::number(timeOfPulseInSeconds) + "\n";
        file.write(writeLine.toStdString().c_str());
    }
}
