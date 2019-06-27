#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "logic.h"
#include "ECGLogic/edfdecoder.h"
#include "ECGLogic/edfprocessing.h"

#include <thread>
#include <functional>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Инициализация графиков для ЭКГ
    ecg1chart = new QChart();
    ecg2chart = new QChart();
    ecg3chart = new QChart();
    ecg1PartLineSeries = new QLineSeries();
    ecg2PartLineSeries = new QLineSeries();
    ecg3PartLineSeries = new QLineSeries();
}

MainWindow::~MainWindow()
{
    delete ecg1chart;
    delete ecg2chart;
    delete ecg3chart;
    delete ecg1PartLineSeries;
    delete ecg2PartLineSeries;
    delete ecg3PartLineSeries;
    delete ui;
}

// Кнопка выхода в меню
void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

// Сохранение времени относитель начала записи
void MainWindow::on_actionSave_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName();
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly);

    double timeOfPulseInSeconds = 0;
    QString writeLine = "";
    if(isECGFile){
        for(int32_t i = 0; i < vecRPeaks.size(); ++i){
            timeOfPulseInSeconds = vecRPeaks[i].x;
            writeLine = QString::number(timeOfPulseInSeconds/ECG_SAMPLING_FREQUENCY) + "\n";
            file.write(writeLine.toStdString().c_str());
        }
    }
    file.close();
}

// Сохранение последовательных интервалов
void MainWindow::on_action_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName();
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly);

    double intervalBetweenPulses = 0;
    QString writeLine = "";

    if(isECGFile){
        for(int32_t i = 1; i < vecRPeaks.size(); ++i){
            intervalBetweenPulses = vecRPeaks[i].x - vecRPeaks[i-1].x;
            writeLine = QString::number(intervalBetweenPulses/ECG_SAMPLING_FREQUENCY) + "\n";
            file.write(writeLine.toStdString().c_str());
        }
    }
    file.close();
}

// Меню: Открыть ЭКГ...
void MainWindow::on_action_2_triggered()
{
    fileName = QFileDialog::getOpenFileName();
    isECGFile = true;
    if(loadEDF(fileName.toStdString(), edf) != 0){
        QString messageBoxText = "По какой-то причине не удалось открыть файл.\n";
        messageBoxText += "Программа принимает только файлы формата .edf";
        QMessageBox::warning(this, "Не удалось открыть файл", messageBoxText);
    } else {
        edf.header.printInfo();
        --edf.header.numberOfDataRecords;   // У ECGDongle последняя запись всегда обрезана
        ecg1chart->removeSeries(ecg1PartLineSeries);
        ecg2chart->removeSeries(ecg2PartLineSeries);
        ecg3chart->removeSeries(ecg3PartLineSeries);
        if(edf.header.numberOfSignals >= 1){
            part1Number = 0;
            setPartialSeries(edf.data.data[0], ecg1PartLineSeries, edf.header.numOfSamplesInDataRecord[0], part1Number);
            setChart(ui->ecgChan1GraphView, ecg1chart, ecg1PartLineSeries);
            ui->ecgChart1Label->setText("1/" + QString::number(edf.header.numberOfDataRecords));
        }
        if(edf.header.numberOfSignals >= 2){
            part2Number = 0;
            setPartialSeries(edf.data.data[1], ecg2PartLineSeries, edf.header.numOfSamplesInDataRecord[1], part2Number);
            setChart(ui->ecgChan2GraphView, ecg2chart, ecg2PartLineSeries);
            ui->ecgChart2Label->setText("1/" + QString::number(edf.header.numberOfDataRecords));
        }
        if(edf.header.numberOfSignals >= 3){
            part3Number = 0;
            setPartialSeries(edf.data.data[2], ecg3PartLineSeries, edf.header.numOfSamplesInDataRecord[2], part3Number);
            setChart(ui->ecgChan3GraphView, ecg3chart, ecg3PartLineSeries);
            ui->ecgChart3Label->setText("1/" + QString::number(edf.header.numberOfDataRecords));
        }
    }
}

// ЭКГ: Слайдер мин. времени QRS перемещён
void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    ui->labelQRStime->setText("Мин. время QRS: " + QString::number(position) + " мс");
}

// ЭКГ: Рассчитать RR-интервалы
void MainWindow::on_pushButton_clicked()
{
    if(!fileName.isEmpty() && isECGFile){
        // Номер выбранного канала ЭКГ минус 1
        numOfEcgChannel = -1;
        if(ui->ecg1RadioButton->isChecked() && edf.header.numberOfSignals >= 1){
            numOfEcgChannel = 0;
        } else if(ui->ecg2RadioButton->isChecked() && edf.header.numberOfSignals >= 2){
            numOfEcgChannel = 1;
        } else if(ui->ecg3RadioButton->isChecked() && edf.header.numberOfSignals >= 3){
            numOfEcgChannel = 2;
        } else {
            numOfEcgChannel = -1;
        }
        // Проверка на корректность выбранного канала
        if(numOfEcgChannel == -1){
            return;
        }
        // Получение предобработанного сигнала
        std::vector<double> preprocessedSignal = preprocessSignal(edf.data.data[numOfEcgChannel], 0);
        // Нахождение QRS комплексов
        std::vector<QRS> qrsComplexes = findQRSComplexes(preprocessedSignal, ui->horizontalSlider->value());
        vecRPeaks = findRPeaks(edf.data.data[numOfEcgChannel], qrsComplexes);
        rPeaks.clear();
        uint32_t partNumber = 0;
        switch(numOfEcgChannel){
            case 0: partNumber = part1Number; break;
            case 1: partNumber = part2Number; break;
            case 2: partNumber = part3Number; break;
        }
        setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[numOfEcgChannel], partNumber);
        std::cout << "vecRPeaks size(): " << vecRPeaks.size() << std::endl;
        std::cout << "rPeaks count(): " << rPeaks.count() << std::endl;
        switch(numOfEcgChannel){
        case 0:
            ecg1chart->addSeries(&rPeaks);
            ecg1chart->createDefaultAxes();
            break;
        case 1:
            ecg2chart->addSeries(&rPeaks);
            ecg2chart->createDefaultAxes();
            break;
        case 2:
            ecg3chart->addSeries(&rPeaks);
            ecg3chart->createDefaultAxes();
            break;
        }
    }
}

// Предыдущая страница первого ЭКГ графика
void MainWindow::on_prev1PushButton_clicked()
{
    if(isECGFile && edf.header.numberOfSignals >= 0){
        if(part1Number >= 1){
            --part1Number;
        } else {
            part1Number = edf.header.numberOfDataRecords-1;
        }
        ecg1chart->removeSeries(ecg1PartLineSeries);
        setPartialSeries(edf.data.data[0], ecg1PartLineSeries, edf.header.numOfSamplesInDataRecord[0], part1Number);
        if(numOfEcgChannel == 0){
            ecg1chart->removeSeries(&rPeaks);
            setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[0], part1Number);
            ecg1chart->addSeries(&rPeaks);
        }
        ecg1chart->addSeries(ecg1PartLineSeries);
        ecg1chart->createDefaultAxes();
        ui->ecgChart1Label->setText(QString::number(part1Number + 1) + "/" + QString::number(edf.header.numberOfDataRecords));
    }
}

// Следующая страница первого ЭКГ графика
void MainWindow::on_next1PushButton_clicked()
{
    // FIXME: Случай, когда файл пустой
    if(isECGFile && edf.header.numberOfSignals >= 0){
        if(part1Number <= (uint32_t)edf.header.numberOfDataRecords-2){
            ++part1Number;
        } else {
            part1Number = 0;
        }
        ecg1chart->removeSeries(ecg1PartLineSeries);
        setPartialSeries(edf.data.data[0], ecg1PartLineSeries, edf.header.numOfSamplesInDataRecord[0], part1Number);
        if(numOfEcgChannel == 0){
            ecg1chart->removeSeries(&rPeaks);
            setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[0], part1Number);
            ecg1chart->addSeries(&rPeaks);
        }
        ecg1chart->addSeries(ecg1PartLineSeries);
        ecg1chart->createDefaultAxes();
        ui->ecgChart1Label->setText(QString::number(part1Number + 1) + "/" + QString::number(edf.header.numberOfDataRecords));
    }
}

// Предыдущая страница второго ЭКГ графика
void MainWindow::on_prev2PushButton_clicked()
{
    if(isECGFile && edf.header.numberOfSignals >= 1){
        if(part2Number >= 1){
            --part2Number;
        } else {
            part2Number = edf.header.numberOfDataRecords-1;
        }
        ecg2chart->removeSeries(ecg2PartLineSeries);
        setPartialSeries(edf.data.data[1], ecg2PartLineSeries, edf.header.numOfSamplesInDataRecord[1], part2Number);
        if(numOfEcgChannel == 1){
            ecg2chart->removeSeries(&rPeaks);
            setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[1], part2Number);
            ecg2chart->addSeries(&rPeaks);
        }
        ecg2chart->addSeries(ecg2PartLineSeries);
        ecg2chart->createDefaultAxes();
        ui->ecgChart2Label->setText(QString::number(part2Number + 1) + "/" + QString::number(edf.header.numberOfDataRecords));
    }
}

// Следующая страница второго ЭКГ
void MainWindow::on_next2PushButton_clicked()
{
    // FIXME: Случай, когда файл пустой
    if(isECGFile && edf.header.numberOfSignals >= 1){
        if(part2Number <= (uint32_t)edf.header.numberOfDataRecords-2){
            ++part2Number;
        } else {
            part2Number = 0;
        }
        ecg2chart->removeSeries(ecg2PartLineSeries);
        setPartialSeries(edf.data.data[1], ecg2PartLineSeries, edf.header.numOfSamplesInDataRecord[1], part2Number);
        if(numOfEcgChannel == 1){
            ecg2chart->removeSeries(&rPeaks);
            setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[1], part2Number);
            ecg2chart->addSeries(&rPeaks);
        }
        ecg2chart->addSeries(ecg2PartLineSeries);
        ecg2chart->createDefaultAxes();
        ui->ecgChart2Label->setText(QString::number(part2Number + 1) + "/" + QString::number(edf.header.numberOfDataRecords));
    }
}

// Предыдущая страница третьего ЭКГ
void MainWindow::on_prev3PushButton_clicked()
{
    if(isECGFile && edf.header.numberOfSignals >= 2){
        if(part3Number >= 1){
            --part3Number;
        } else {
            part3Number = edf.header.numberOfDataRecords-1;
        }
        ecg3chart->removeSeries(ecg3PartLineSeries);
        setPartialSeries(edf.data.data[2], ecg3PartLineSeries, edf.header.numOfSamplesInDataRecord[2], part3Number);
        if(numOfEcgChannel == 2){
            ecg3chart->removeSeries(&rPeaks);
            setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[2], part3Number);
            ecg3chart->addSeries(&rPeaks);
        }
        ecg3chart->addSeries(ecg3PartLineSeries);
        ecg3chart->createDefaultAxes();
        ui->ecgChart3Label->setText(QString::number(part3Number + 1) + "/" + QString::number(edf.header.numberOfDataRecords));
    }
}

// Следующая страница третьего ЭКГ
void MainWindow::on_next3PushButton_clicked()
{
    // FIXME: Случай, когда файл пустой
    if(isECGFile && edf.header.numberOfSignals >= 2){
        if(part3Number <= (uint32_t)edf.header.numberOfDataRecords-2){
            ++part3Number;
        } else {
            part3Number = 0;
        }
        ecg3chart->removeSeries(ecg3PartLineSeries);
        setPartialSeries(edf.data.data[2], ecg3PartLineSeries, edf.header.numOfSamplesInDataRecord[2], part3Number);
        if(numOfEcgChannel == 2){
            ecg3chart->removeSeries(&rPeaks);
            setPartialSeries(vecRPeaks, &rPeaks, edf.header.numOfSamplesInDataRecord[2], part3Number);
            ecg3chart->addSeries(&rPeaks);
        }
        ecg3chart->addSeries(ecg3PartLineSeries);
        ecg3chart->createDefaultAxes();
        ui->ecgChart3Label->setText(QString::number(part3Number + 1) + "/" + QString::number(edf.header.numberOfDataRecords));
    }
}

// Сохранить в милливольтах выбранный канал
void MainWindow::on_action_3_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName();
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly);

    QString writeLine = "";
    if(isECGFile){
        int32_t numOfSelectedChannel = -1;
        if(ui->ecg1RadioButton->isChecked()){
            numOfSelectedChannel = 0;
        } else if(ui->ecg2RadioButton->isChecked()){
            numOfSelectedChannel = 1;
        } else {
            numOfSelectedChannel = 2;
        }
        double maxDigitalValue = edf.header.digitalMaximum[numOfSelectedChannel];
        std::cout << "MAX_DIGITAL_VALUE: " << maxDigitalValue << std::endl;
        double maxPhysicalValue = edf.header.physicalMaximum[numOfSelectedChannel];
        std::cout << "MAX_PHYSICAL_VALUE: " << maxPhysicalValue << std::endl;
        double digToPhysCoefficient = maxPhysicalValue/maxDigitalValue;
        std::cout << "DIG_TO_PHYS_COEFFICIENT: " << digToPhysCoefficient << std::endl;
        for(int32_t i = 0; i < edf.data.data[numOfSelectedChannel].size(); ++i){
            writeLine = QString::number(edf.data.data[numOfSelectedChannel][i] * digToPhysCoefficient) + "\n";
            file.write(writeLine.toStdString().c_str());
        }
    }
    file.close();
}
