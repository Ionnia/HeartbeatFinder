#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <SDL2/SDL_audio.h>
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
    // Инициализация графиков для АУДИО
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

void test(){
    qDebug() << "In Thread!" << "\n";
}

// Открываем аудиофайл
void MainWindow::on_actionOpen_triggered()
{
    fileName = QFileDialog::getOpenFileName();
    isECGFile = false;
    if(!fileName.isEmpty()){
        QFile file(fileName);
        if(!wavFile.isEmpty){
            wavFile.free();
        }
        if(wavFile.openWAV(fileName)){
            chart1->removeSeries(chartSeries1);
            chart2->removeSeries(chartSeries2);
            chart2->removeSeries(integralSeries);
            chart2->removeSeries(pulses);
//            spectrumChart->removeSeries(spectrumSeries);
            chartSeries1->clear();
            chartSeries2->clear();
            integralSeries->clear();
            pulses->clear();
//            spectrumSeries->clear();
            // Считываем данные
//            bufToQLineSeries(chartSeries1, wavFile.wav_buf_16bit, wavFile.wav_len/2, 441);
//            absBufToQLineSeriesWithPorog(chartSeries2, integralSeries, wavFile.wav_buf_16bit, wavFile.wav_len/2, 441, 0);
            std::thread t1(bufToQLineSeries, chartSeries1, wavFile.wav_buf_16bit, wavFile.wav_len/2, 441, wavFile.wav_spec.freq);
            std::thread t2(absBufToQLineSeriesWithPorog, chartSeries2, integralSeries, wavFile.wav_buf_16bit, wavFile.wav_len/2, 441, 0, wavFile.wav_spec.freq);
            t1.join();
            t2.join();
            calculatePulsePoints(pulses, integralSeries, ui->platoSlider->sliderPosition());

            setChart(ui->mainAudioGraphView, chart1, chartSeries1);
            setChart(ui->modifiedAudioGraphView, chart2, chartSeries2);

            ui->porogSlider->setSliderPosition(0);
            ui->checkBox->setEnabled(true);
            ui->checkBox_2->setEnabled(true);

            ui->tabWidget->setCurrentIndex(1);
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

// АУДИО: Когда слайдер пороговой амплитуды перемещён
void MainWindow::on_porogSlider_sliderMoved(int position)
{
    ui->label->setText("Порог амплитуды: " + QString::number(position));
}

// АУДИО: Когда слайдер пороговой амплитуды отпущен
void MainWindow::on_porogSlider_sliderReleased()
{
    if(!fileName.isEmpty()){
        if(ui->checkBox->isChecked()){
            chart2->removeSeries(integralSeries);
        } else {
            chart2->removeSeries(chartSeries2);
        }
        absBufToQLineSeriesWithPorog(chartSeries2, integralSeries, wavFile.wav_buf_16bit, wavFile.wav_len/2, 441, ui->porogSlider->sliderPosition());
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

// АУДИО: Когда переключён чекбокс интегрального значения
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

// Кнопка выхода в меню
void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

// АУДИО: Когда слайдер ширины плато перемещён
void MainWindow::on_platoSlider_sliderMoved(int position)
{
    ui->label_2->setText("Ширина плато: " + QString::number(position * 0.01) + " сек.");
}

// АУДИО: Когда слайдер ширины плато отпущен
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

// АУДИО: Когда переключён чекбокс биений сердца
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

// Сохранение времени относитель начала записи
void MainWindow::on_actionSave_triggered()
{
    QString saveFileName = QFileDialog::getSaveFileName();
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly);

    double timeOfPulseInSeconds = 0;
    QString writeLine = "";
    if(isECGFile){
        for(int i = 0; i < vecRPeaks.size(); ++i){
            timeOfPulseInSeconds = vecRPeaks[i].x;
            writeLine = QString::number(timeOfPulseInSeconds/ECG_SAMPLING_FREQUENCY) + "\n";
            file.write(writeLine.toStdString().c_str());
        }
    } else {
        for(int i = 0; i < pulses->count(); ++i){
            timeOfPulseInSeconds = pulses->at(i).x();
            writeLine = QString::number(timeOfPulseInSeconds) + "\n";
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
        for(int i = 1; i < vecRPeaks.size(); ++i){
            intervalBetweenPulses = vecRPeaks[i].x - vecRPeaks[i-1].x;
            writeLine = QString::number(intervalBetweenPulses/ECG_SAMPLING_FREQUENCY) + "\n";
            file.write(writeLine.toStdString().c_str());
        }
    } else {
        for(int i = 1; i < pulses->count(); ++i){
            intervalBetweenPulses = pulses->at(i).x() - pulses->at(i-1).x();
            writeLine = QString::number(intervalBetweenPulses) + "\n";
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
        ui->tabWidget->setCurrentIndex(0);
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
        std::vector<double> preprocessedSignal = preprocessSignal(edf.data.data[numOfEcgChannel], ui->normLevelSpinBox->value());
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
        for(int i = 0; i < edf.data.data[numOfSelectedChannel].size(); ++i){
            writeLine = QString::number(edf.data.data[numOfSelectedChannel][i] * digToPhysCoefficient) + "\n";
            file.write(writeLine.toStdString().c_str());
        }
    }
    file.close();
}
