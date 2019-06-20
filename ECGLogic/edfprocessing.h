#pragma once

#include <vector>
#include <cinttypes>


// Структура данных, в которой хранится индекс начала и конца QRS комплекса
struct QRS{
    uint32_t start;
    uint32_t end;
};

// Структура данных, в которой хранится местоположение R зубца
struct RPeak{
    uint32_t x;
    double y;
};

// Функция для дифференцирования сигнала
std::vector<double> differentiateSignal(std::vector<double> &values);

// Функция для интегрирования окном
std::vector<double> integrateSignalWindow(std::vector<double> &values, uint32_t windowSize);

// Функция нахождения среднего значения с окном
std::vector<double> averageWindow(std::vector<double> &values, uint32_t windowSize);

// Функция для нормализации уровня
std::vector<double> normalizeSignal(std::vector<double> &values);

// Функция получения максимального значения в массиве
double getMaxValue(std::vector<double> &values);

// Функция получения среднего значения в массиве
double getAvgValue(std::vector<double> &values);

// Функция, которая возвращает массив values, в котором значения меньше соответствующие
// значения в массиве cutValues замещаются нулями
std::vector<double> cutWithArray(std::vector<double> &values, std::vector<double> &cutValues);

std::vector<double> cutWithValue(std::vector<double> &values, double value);

std::vector<double> squareArray(std::vector<double> &values);

// Функция реализующая предварительную обработку сигнала
std::vector<double> preprocessSignal(std::vector<double> &values, uint32_t normLevel);

// Функция возвращает массив структур QRS комплекса (создерижит индекс начала и конца)
// Принимает на вход заранее обработанный массив
std::vector<QRS> findQRSComplexes(std::vector<double> &values, uint32_t MIN_QRS_THRESHOLD);

// Функция возвращает все R-зубца, которая она нашла
std::vector<RPeak> findRPeaks(std::vector<double> &values, std::vector<QRS> &qrsComplexes);