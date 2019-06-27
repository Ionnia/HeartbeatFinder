// Модуль для декодирования файлов в формате .edf
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>


// Заголовок EDF файла
struct EDFHeader{
    std::string version;    // 8 байт
    std::string patientID;  // 80 байт
    std::string recordingID;        // 80 байт
    std::string startDate;  // 8 байт
    std::string startTime;  // 8 байт
    int32_t numberOfBytesInHeader;  // 8 байт
    // Зарезервированные 44 байта
    int32_t numberOfDataRecords;    // 8 байт
    int32_t durationOfDataRecord;   // 8 байт
    int32_t numberOfSignals;        // 4 байта
    std::vector<std::string> labels;            // numberOfSignals * 16 байт
    std::vector<std::string> transducerTypes;    // numberOfSignals * 80 байт
    std::vector<std::string> physicalDimensions; // numberOfSignals * 8 байт
    std::vector<double> physicalMinimum;   // numberOfSignals * 8 байт
    std::vector<double> physicalMaximum;   // numberOfSingals * 8 байт
    std::vector<int32_t> digitalMinimum;    // numberOfSingals * 8 байт
    std::vector<int32_t> digitalMaximum;    // numberOfSignals * 8 байт
    std::vector<std::string> prefiltering;      // numberOfSignals * 80 байт
    std::vector<int32_t> numOfSamplesInDataRecord; // numberOfSignals * 8 байт
    // Зарезервированные 32 байта

    void printInfo();
};
// Данные EDF файла
struct EDFData{
    // Внешнее -- signal - data-record - data
    std::vector<std::vector<double>> data;
    
    void printInfo();
};
// Структура стандартного EDF файла (стандарт 1992 года)
struct EDF{
    EDFHeader header;
    EDFData data;
};

// Функция std::ifstream::read() не добавляет \0 в конце
void readNBytes(std::ifstream &file, char *buf, uint32_t numOfBytes);

// Функция читает весь заголовок .edf файла, так что дальнейшее чтение будет
// происходить в области данных
void loadEDFHeader(std::ifstream &file, EDFHeader &header);

void loadEDFData(std::ifstream &file, EDFData &data, const EDFHeader &header);

// Функция открывает .edf файл
int32_t loadEDF(std::string pathToFile, EDF &edf);
